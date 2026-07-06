import "__c_tools"
import "@stdlib/json/json.arc"

var netlib = dl_open("./stdlib/clib/net/build/libarcnet.so", 1)

var net_req_init = dl_sym(netlib, "arcNet_request_init", 0, false)
var net_req_deinit = dl_sym(netlib, "arcNet_request_deinit", 0, false)
var net_req_get = dl_sym(netlib, "arcNet_request_get", 1, false)
#var net_HttpServer = dl_sym(netlib, "arcNet_HttpServer", 5, false)
#var net_start_server = dl_sym(netlib, "arcNet_start_server", 1, false)
#var net_add_route = dl_sym(netlib, "arcNet_add_route", 4, false)

net_req_init()

var res = net_req_get("https://arc-net-test.free.beeceptor.com/test")
print("raw:", res)

print("json:", to_json(res))

#var server = net_HttpServer("localhost", 8000, 4096, 0, false)

#fn test(path) then 
#  return "<h1>You're currently on: " + path + "<h1>"
#end

#net_add_route(server, "/", ["GET"], test)
#net_start_server(server)

net_req_deinit()
dl_close(netlib)
