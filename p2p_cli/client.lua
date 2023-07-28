print("start client")
local target_ip = "http://192.168.128.22:8082"
package.cpath = "/home/shary/program/peer2peer/client.so"
local func_lib =  require("p2p_client")
local file = io.open("binary_data", "r")
if nil == file then
    print("open file fail")
end
io.input(file)
local send_data = io.read("all")
io.close(file)
print(func_lib.cli_send(2, target_ip, send_data))