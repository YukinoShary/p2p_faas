print("start client")
local target_ip = "http://192.168.128.22:8082"
package.cpath = "client.so"
local func_lib = require("p2p_client_edit")
local file = io.open("sendFile", "r")
if nil == file then
    print("open file fail")
end
io.input(file)
local send_data = io.read("all")
print(func_lib.cli_send(2, target_ip, send_data))
io.close(file)