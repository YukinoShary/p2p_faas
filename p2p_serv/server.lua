local ip_num = 1
local table = {"172.27.79.255"}    
package.cpath = "p2p_server.so" 
local func_lib = require("p2p_server")
func_lib.serv_start(ip_num, table)
print(func_lib.get_data(1))
