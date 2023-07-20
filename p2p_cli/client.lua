print("start client")
local send_data = "hello world"
local target_ip = "http://192.168.128.22:8080"
package.cpath = "/home/shary/program/peer2peer/client.so"
local func_lib =  require("p2p_client")
print(func_lib.cli_send(2, target_ip, send_data))