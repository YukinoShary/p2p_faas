#include <stdlib.h>
#include <stdio.h>
#include <curl/curl.h>
#include <lua5.4/lua.h>
#include <lua5.4/lualib.h>
#include <lua5.4/lauxlib.h>

static void curl_init(lua_State *lua_S);
int luaopen_p2p_client(lua_State *L);

static void curl_init(lua_State *L)
{
    int n;
    printf("start to initialize a client\n");
    char *send_data, *target_ip;
    CURL *curl;
    struct curl_slist *hs = NULL;
    n = luaL_checknumber(L, 1);
    curl = curl_easy_init();
    /*Determine the mode by the number of args*/
    switch(n)
    {
        case 2:
        /*one time post mode*/
        target_ip = lua_tostring(L, 2);
        send_data = lua_tostring(L, 3); 
        if(curl)
        {
            CURLcode ret;
            curl_easy_setopt(curl, CURLOPT_URL, target_ip);
            curl_easy_setopt(curl, CURLOPT_POST, 1L);
            hs =  curl_slist_append(hs, "Content-Type: text/plain");
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, hs);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, send_data);
            printf("set option\n");
            ret = curl_easy_perform(curl);
            printf("post send\n");
            if(ret != CURLE_OK)
            {
                curl_easy_strerror(ret);
            }
            curl_easy_cleanup(curl);
        }
        break;
        case 3:
        /* chunked transfer */
        target_ip = lua_tostring(L, 2);
        send_data = lua_tostring(L, 3); 
        if(curl)
        {
            CURLcode ret;
            curl_easy_setopt(curl, CURLOPT_URL, target_ip);
            curl_easy_setopt(curl, CURLOPT_POST, 1L);
            hs = curl_slist_append(hs, "Transfer-Encoding: chunked");
            hs = curl_slist_append(hs, "Content-Type: application/octet-stream");
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, hs);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, send_data);
            printf("set option\n");
            ret = curl_easy_perform(curl);
            printf("post send\n");
            if(ret != CURLE_OK)
            {
                curl_easy_strerror(ret);
            }
            curl_slist_free_all(hs);
            curl_easy_cleanup(curl);
        }
        break;
    }  
}

static int file_open()
{

}

static const struct luaL_Reg reg_funcs[] = 
{
    {"cli_send", curl_init},
    {NULL, NULL}
};

int luaopen_p2p_client(lua_State *L) 
{
    luaL_newlib(L, reg_funcs); 
    return 1;
}