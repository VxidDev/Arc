import "__c_tools"
import "../stdlib/json/json.arc"

var netlib = dl_open("./stdlib/clib/net/build/libarcnet.so", 1)

var net_req_init = dl_sym(netlib, "arcNet_request_init", 0, false)
var net_req_deinit = dl_sym(netlib, "arcNet_request_deinit", 0, false)
var net_req_get = dl_sym(netlib, "arcNet_request_get", 1, false)

net_req_init()

var res = net_req_get("https://arc-net-test.free.beeceptor.com/test")
print("raw:", res)

print("json:", to_json(res))

net_req_deinit()

dl_close(netlib)
