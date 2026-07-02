#define _GNU_SOURCE

#include "../include/axionetd.h"
#include "../include/http.h"
#include "../include/router.h"
#include "../include/defaultRoutes.h"
#include "../include/threading.h"
#include "../include/memory-pool.h"

#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <signal.h>

#include <stdio.h>

#include <pthread.h>

Axionet* globalServer = NULL;
int globalEpollFd = -1;
bool enableLogging = true;
bool useThreading;

MemoryPool *connPool;
MemoryPool *bufferPool;
MemoryPool *queryPool;
MemoryPool *responsePool;

AxioJobQueue jobQueue;

const char* statusCodeToText(int status) {
    switch (status / 100) {
        case 1: return "\033[37mInformational";
        case 2: return "\033[32mSuccess";
        case 3: return "\033[33mRedirection";
        case 4: return "\033[31mClient Error";
        case 5: return "\033[35mServer Error";
        default: return "\033[90mUnknown";
    }
}

bool isMethodAllowed(const AxioRoute *route, const char *method) {
    if (!method || !route) return false;

    // Empty array = allow all
    if (!route->methods[0] || route->amountOfMethods == 0) return true;

    for (size_t i = 0; i < route->amountOfMethods; i++) {
        if (route->methods[i] && strcmp(route->methods[i], method) == 0) return true; 
    }

    return false;
};

void closeConnection(int epollFd, AxioConnection *conn) {
    epoll_ctl(epollFd, EPOLL_CTL_DEL, conn->fd, NULL);
    close(conn->fd);

    if (conn->response) poolFree(responsePool, conn->response);

    if (conn->readBuffer) {
        poolFree(bufferPool, conn->readBuffer);
    }

    poolFree(connPool, conn);
}

void pushJob(AxioJob job) {
    pthread_mutex_lock(&jobQueue.mutex);

    while (jobQueue.size == AXIO_JOB_QUEUE_SIZE) {
        pthread_cond_wait(&jobQueue.cond, &jobQueue.mutex);
    }

    jobQueue.jobs[jobQueue.back] = job;
    jobQueue.back = (jobQueue.back + 1) % AXIO_JOB_QUEUE_SIZE;
    jobQueue.size++;

    pthread_cond_signal(&jobQueue.cond);
    pthread_mutex_unlock(&jobQueue.mutex);
}

AxioJob popJob() {
    pthread_mutex_lock(&jobQueue.mutex);

    while (jobQueue.size == 0) {
        pthread_cond_wait(&jobQueue.cond, &jobQueue.mutex);
    }

    AxioJob job = jobQueue.jobs[jobQueue.front];
    jobQueue.front = (jobQueue.front + 1) % AXIO_JOB_QUEUE_SIZE;
    jobQueue.size--;

    pthread_cond_signal(&jobQueue.cond);
    pthread_mutex_unlock(&jobQueue.mutex);

    return job;
}

void* workerThread(void *arg) {
    (void)arg;

    while (1) {
        AxioJob job = popJob();

        AxioResponse response = {0};
        bool handled = false;

        for (size_t j = 0; j < globalServer->routeAmount; j++) {
            if (strcmp(job.request.path, globalServer->routes[j]->path) == 0) {

                if (!isMethodAllowed(globalServer->routes[j], job.request.method)) {
                    route405(&response, responsePool);
                    handled = true;
                    break;
                }

                globalServer->routes[j]->handler(&job.request, &response, responsePool);
                handled = true;
                break;
            }
        }

        if (!handled || !response.response) {
            route404(&response, responsePool);
        }

        if (enableLogging) {
            printf("\033[1;35m%s\033[37m - \033[36m%s %d %s\033[0m\n",
                job.request.method,
                job.request.path,
                response.status,
                statusCodeToText(response.status)
            );
        }

        job.conn->response = response.response;
        job.conn->total = response.len;
        job.conn->sent = 0;
        job.conn->state = AXIO_WRITING;

        if (enableLogging) {
            printf("\033[1;35m%s\033[37m - \033[36m%s %d %s\033[0m\n",
                job.request.method,
                job.request.path,
                response.status,
                statusCodeToText(response.status)
            );
        }

        struct epoll_event ev = {0};
        ev.events = EPOLLOUT | EPOLLET;
        ev.data.ptr = job.conn;

        epoll_ctl(globalEpollFd, EPOLL_CTL_MOD, job.conn->fd, &ev);

        yyjson_doc_free(job.request.json);
    }

    return NULL;
}

Axionet* initServer(const char *host, const int port, const int backlog, int workers, const bool logging) {
    useThreading = (workers > 0);
    enableLogging = logging;

    int serverFd = socket(AF_INET, SOCK_STREAM, 0); // Initialize socket

    if (serverFd == -1) { // Error while initializing socket
        return NULL;
    }

    struct addrinfo hints, *res = NULL;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    char portStr[16];
    snprintf(portStr, sizeof(portStr), "%d", port);

    const char *bindHost = host;

    if (!bindHost || strlen(bindHost) == 0) {
        bindHost = NULL;
    }

    if (getaddrinfo(bindHost, portStr, &hints, &res) != 0 || !res) {
        close(serverFd);
        return NULL;
    }

    int opt = 1;
    setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    if (bind(serverFd, res->ai_addr, res->ai_addrlen) == -1) { // Bind address structure to socket and handle error
        freeaddrinfo(res);
        close(serverFd); // Close socket
        return NULL;
    }

    freeaddrinfo(res);

    Axionet* server = malloc(sizeof(Axionet)); // Allocate memory for server

    if (!server) { // Handle memory allocation failure
        close(serverFd); // Close socket
        return NULL;
    }

    // Fill in server structure
    server->fd = serverFd;
    server->port = port;
    server->host = strdup(bindHost ? bindHost : "0.0.0.0");
    server->backlog = backlog;
    server->isRunning = false;
    server->workers = workers;

    server->routeAmount = 0;
    server->routeCapacity = 4;

    server->routes = malloc(sizeof(AxioRoute*) * server->routeCapacity);

    if (!server->routes) {
        close(serverFd);
        free(server);
        return NULL;
    }

    return server;
}

void stopServer(int sig) {
    (void)sig;

    if (globalServer) {
        globalServer->isRunning = false;
    }
}

void startServer(Axionet* server) {
    globalServer = server;

    connPool = poolCreate(sizeof(AxioConnection), AXIO_MEMORY_POOL_SIZE);
    bufferPool = poolCreate(8192, AXIO_MEMORY_POOL_SIZE);
    queryPool = poolCreate(sizeof(AxioQueryParam) * AXIO_MAX_QUERY_PARAMS, AXIO_MEMORY_POOL_SIZE);
    responsePool = poolCreate(16384, AXIO_MEMORY_POOL_SIZE);

    // init queue
    if (useThreading) {
        pthread_mutex_init(&jobQueue.mutex, NULL);
        pthread_cond_init(&jobQueue.cond, NULL);
        jobQueue.front = jobQueue.back = jobQueue.size = 0;
    }

    // workers
    pthread_t *workers = NULL;

    if (useThreading) {
        workers = malloc(sizeof(pthread_t) * server->workers);

        for (int i = 0; i < server->workers; i++) {
            pthread_create(&workers[i], NULL, workerThread, NULL);
            pthread_detach(workers[i]);
        }
    }

    // CTRL + C handling
    struct sigaction sa = {0};
    sa.sa_handler = stopServer;
    sigaction(SIGINT, &sa, NULL);

    globalEpollFd = epoll_create1(0);

    fcntl(server->fd, F_SETFL, O_NONBLOCK);
    listen(server->fd, server->backlog);

    server->isRunning = true;

    struct epoll_event ev = {0};
    ev.events = EPOLLIN | EPOLLET;
    ev.data.fd = server->fd;

    epoll_ctl(globalEpollFd, EPOLL_CTL_ADD, server->fd, &ev);

    struct epoll_event events[64];

    printf("\033[1;35mAxionetd: \033[33mServer running on \033[36mhttp://%s:%d\033[0m\n",
           server->host, server->port);

    while (server->isRunning) {
        int n = epoll_wait(globalEpollFd, events, 64, -1);
        if (n <= 0) continue;

        for (int i = 0; i < n; i++) {
            if (events[i].data.fd == server->fd) {
                while (1) {
                    int clientFd = accept4(server->fd, NULL, NULL, SOCK_NONBLOCK | SOCK_CLOEXEC);

                    if (clientFd < 0) break;

                    AxioConnection *conn = poolAlloc(connPool);

                    if (!conn) {
                        close(clientFd);
                        continue;
                    }

                    memset(conn, 0, sizeof(*conn));

                    conn->fd = clientFd;
                    conn->state = AXIO_READING;

                    struct epoll_event cev = {0};
                    cev.events = EPOLLIN | EPOLLET;
                    cev.data.ptr = conn;

                    epoll_ctl(globalEpollFd, EPOLL_CTL_ADD, clientFd, &cev);
                }
                continue;
            }

            AxioConnection *conn = events[i].data.ptr;

            if (events[i].events & (EPOLLERR | EPOLLHUP)) {
                closeConnection(globalEpollFd, conn);
                continue;
            }

            if (conn->state == AXIO_READING && (events[i].events & EPOLLIN)) {
                size_t size = 8192, total = 0;
                char *buf = poolAlloc(bufferPool);

                if (!buf) {
                    closeConnection(globalEpollFd, conn);
                    continue;
                }

                while (1) {
                    ssize_t r = read(conn->fd, buf + total, size - total - 1);

                    if (r <= 0) break;

                    total += r;

                    if (strstr(buf, "\r\n\r\n")) break;
                }

                buf[total] = '\0';
                conn->readBuffer = buf;

                AxioRequest req = {0};

                if (!parseRequest(&req, buf, queryPool)) {
                    AxioResponse resp = {0};
                    route400(&resp, responsePool);

                    conn->response = resp.response;
                    conn->total = resp.len;
                    conn->sent = 0;
                    conn->state = AXIO_WRITING;

                    if (enableLogging) {
                        printf("\033[1;31mERROR:\033[33m Malformed request. \033[0m\n");
                    }
                } else {
                    bool handled = false;

                    for (size_t j = 0; j < server->routeAmount; j++) {
                        if (strcmp(req.path, server->routes[j]->path) == 0) {
                            if (!isMethodAllowed(server->routes[j], req.method)) {
                                AxioResponse resp = {0};
                                route405(&resp, responsePool);

                                conn->response = resp.response;
                                conn->total = resp.len;
                                conn->state = AXIO_WRITING;
                                handled = true;
                                break;
                            }

                            if (useThreading && server->routes[j]->threaded) {
                                // thread pool
                                AxioJob job = {0};
                                job.conn = conn;
                                job.request = req;

                                pushJob(job);
                            } else {
                                // fallback inline
                                AxioResponse resp = {0};

                                server->routes[j]->handler(&req, &resp, responsePool);

                                if (!resp.response) {
                                    route404(&resp, responsePool);
                                }

                                if (enableLogging) {
                                    printf("\033[1;35m%s\033[37m - \033[36m%s %d %s\033[0m\n",
                                        req.method,
                                        req.path,
                                        resp.status,
                                        statusCodeToText(resp.status)
                                    );
                                }

                                conn->response = resp.response;
                                conn->total = resp.len;
                                conn->sent = 0;
                                conn->state = AXIO_WRITING;

                                struct epoll_event ev = {0};
                                ev.events = EPOLLOUT | EPOLLET;
                                ev.data.ptr = conn;

                                epoll_ctl(globalEpollFd, EPOLL_CTL_MOD, conn->fd, &ev);

                                yyjson_doc_free(req.json);
                            }

                            handled = true;
                            break;
                        }
                    }

                    if (!handled) {
                        AxioResponse resp = {0};
                        route404(&resp, responsePool);

                        conn->response = resp.response;
                        conn->total = resp.len;
                        conn->state = AXIO_WRITING;
                    }
                }
            }

            if (conn->state == AXIO_WRITING) {
                while (conn->sent < conn->total) {
                    ssize_t w = write(conn->fd, conn->response + conn->sent, conn->total - conn->sent);

                    if (w > 0) conn->sent += w;
                    else break;
                }

                if (conn->sent >= conn->total) {
                    closeConnection(globalEpollFd, conn);
                }
            }
        }
    }

    free(server->routes);
    free(server->host);
    free(workers);

    poolDestroy(bufferPool);
    poolDestroy(connPool);
    poolDestroy(queryPool);
    poolDestroy(responsePool);

    close(server->fd);
    close(globalEpollFd);
}