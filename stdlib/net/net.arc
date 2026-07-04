import "__c_tools"
import "__lib_tools"

var __stdlib_path__ = stdlib_path()
var __net__ = dl_open(__stdlib_path__ + "/clib/libarcnet.so", 1)

var net_req_init = dl_sym(__net__, "arcNet_request_init", 0, false)
var net_req_deinit = dl_sym(__net__, "arcNet_request_deinit", 0, false)
var net_req_get = dl_sym(__net__, "arcNet_request_get", 1, false)

class Net
  fn init() then
    net_req_init()
  end

  fn deinit() then 
    net_req_deinit()
  end 

  fn get(url) then 
    net_req_get(url)
  end 
end
