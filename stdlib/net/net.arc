import "__c_tools"
import "__lib_tools"

var __stdlib_path__ = stdlib_path()
var __net__ = dl_open(__stdlib_path__ + "/clib/libarcnet.so", 1)

var net_req_init = dl_sym(__net__, "arcNet_request_init", 0, false)
var net_req_deinit = dl_sym(__net__, "arcNet_request_deinit", 0, false)
var net_req_get = dl_sym(__net__, "arcNet_request_get", 1, false)

var net_HTTPServer = dl_sym(__net__, "arcNet_HttpServer", 5, false)
var net_start_server = dl_sym(__net__, "arcNet_start_server", 1, false)
var net_add_route = dl_sym(__net__, "arcNet_add_route", 4, false)
var net_free_server = dl_sym(__net__, "arcNet_free_server", 1, false)

class Net
  fn init() then
    return net_req_init()
  end

  fn deinit() then 
    return net_req_deinit()
  end 

  fn get(url) then 
    return net_req_get(url)
  end 
end

class HTTPServer 
  var host = ""
  var port = -1
  var backlog = -1
  var __workers__ = -1 # not supported
  var logging = true 
  
  var is_initialized = false

  var __server_ptr = null 

  fn init(self, host, port, backlog, logging) then
    if self.is_initialized then 
      return true
    end 

    self.host = host 
    self.port = port 
    self.backlog = backlog
    self.logging = logging 
    
    self.__server_ptr = net_HTTPServer(host, port, backlog, logging) # raises error if axionetd's initServer returns NULL

    self.is_initialized = true
  end

  fn start(self) then 
    if not self.is_initialized then 
      RuntimeError("Server is not initialized")
    end 

    net_start_server(self.__server_ptr) # only stoppable through CTRL + C
  end

  fn destroy(self) then 
    if not self.is_initialized then 
      RuntimeError("Server is not initialized")
    end


  end 
end 
