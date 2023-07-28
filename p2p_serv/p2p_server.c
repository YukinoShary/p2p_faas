#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <lua5.4/lua.h>
#include <lua5.4/lualib.h>
#include <lua5.4/lauxlib.h>
#include "mongoose.h"
#define PORT 8080
#define MAXNODES_PER_PIPELINE 80
#define RECEIVE_CONFIRM 1
#define BUFFERSIZE 4096

typedef struct request
{
    struct MHD_PostProcessor *pp;
    const char *post_url;
    char *post_data;
}request;

typedef struct data_node
{
    const char *data;
    size_t data_lengh;
    struct data_node *prev, *next;
} data_node;

typedef struct data_queue
{
    int length;
    struct data_node *head, *rear;
} data_queue;

static struct data_queue* queue;
static struct mg_mgr mgr;
static int running_sig;

static void mongoose_start(lua_State* L);
static void mongoose_stop();
static void fn(struct mg_connection *c, int ev, void *ev_data, void *fn_data);
static void multipart_processing(struct mg_connection *c, char* data);
//static int allowed_ip(char *ip, char *nodes_tb[]);
static void set_data(const char* result);
static int get_data(lua_State* L);
int luaopen_p2p_server(lua_State *L);

/*initialize a server program and start it*/
static void mongoose_start(lua_State* L)
{
    int n, i;
    queue = malloc(sizeof(data_queue));
    queue->length = 0;
    queue->head = NULL;
    queue->rear = NULL;
    printf("start to intialize a server\n");
    running_sig = 1;
    char *ip_table[MAXNODES_PER_PIPELINE];
    n = luaL_checknumber(L, 1);
    if(!lua_istable(L, 2))
        printf("input_fft is not a valid complex table\n");
    for(i = 0; i < n; i++)
    {
        lua_rawgeti(L, 2, i + 1);
        ip_table[i] = lua_tostring(L, -1);
        lua_pop(L, 1);
    }
    lua_settop(L, 0);
    
    mg_mgr_init(&mgr);                                       // Init manager4
    printf("try to start server\n");
    mg_http_listen(&mgr, "http://0.0.0.0:8082", fn, NULL);  // Setup listener, the ip_table should be sent as the las parameter in final version.
    printf("server started\n");
    while(running_sig)
    {
        mg_mgr_poll(&mgr, 1000);                         // Event loop
    }  
    mg_mgr_free(&mgr); 
}

/*stop the server program and release the resource*/
static void mongoose_stop(lua_State* L)
{
    mg_mgr_free(&mgr); 
    printf("server stop\n");
}

/*request callback function*/
static void fn(struct mg_connection *c, int ev, void *ev_data, void *fn_data)
{
    int ack;
    struct mg_http_message *hm;
    struct mg_str *content_type *content_length;
    char *type_str;
    size_t data_size;
    const char *recv_data;

    /*
    if(!allowed_ip((c->rem)., fn_data)
    {
        printf("from the umknown ip.\n");
        mg_http_reply(c, 403, "", "%s", "Invalidated node\n");
    }
    */
    if(ev == MG_EV_HTTP_MSG)
    {
        hm = (struct mg_http_message *) ev_data;
        content_type = mg_http_get_header(hm, "Content-Type");
        content_length = mg_http_get_header(hm, "Content-Length");
        sscanf(content_length->ptr, "%zu", &data_size);
        recv_data = malloc(atoi(content_length->ptr));
        memcpy(recv_data, hm->body.ptr, data_size);
        emcpy(type_str, content_type->ptr, content_type->len);
        printf("content_type:%s", content_type->ptr);
        printf("ret:%d", ret);
        if(strcmp(content_type->ptr, "application/octet-stream") == 0)
        {
            printf("application/octet-stream");
            ack = 1;
            set_data(recv_data);
            mg_http_reply(c, 200, "", "{%m:%d}\n", MG_ESC("status"), ack); 
        }
        else if(strcmp(content_type->ptr, "multipart/form-data") == 0)
        {
            printf("multipart/form-data\r\n");
            multipart_processing(c, recv_data);
            ack = 2;
            mg_http_reply(c, 200, "", "{%m:%d}\n", MG_ESC("status"), ack); 
        }
        else if(strcmp(content_type->ptr, "text/plain") == 0)
        {
            printf("text/plain\n");
            printf("body:%s\n", hm->body.ptr);
            set_data(recv_data);
            ack = 3;
            mg_http_reply(c, 200, "", "{%m:%d}\n", MG_ESC("status"), ack); 
        }
        running_sig = 0;
        printf("finished\n");
    }
}


static void multipart_processing(struct mg_connection *c, char* data)
{
    
}


/*ip white list
  the ip address of mg_addr is in network byte order (big endian)*/
/*
static int allowed_ip(uint8_t ip[], char *nodes_tb[])
{
    int i;
    for(i = 0; i < MAXNODES_PER_PIPELINE; i++)
    {
        if(!strcmp(ip, nodes_tb[i]))
            return 1;    
        return 1;
    }
    return 0;
}
*/

/*push received data to the lua stack*/
static void set_data(const char *result)
{
    struct data_node *node;
    node = malloc(sizeof(data_node));
    node->data = result;
    printf("set data:%s\n", node->data);
    node->prev = queue->rear;
    node->next = NULL;
    queue->rear = node;
    queue->length ++;
    if(queue->length == 1)
    {
        queue->head = node;
        printf("set data as head\n");
    }
}

static int get_data(lua_State* L)
{
    printf("get data\n");
    while(1)
    {
        if(queue->length > 0)
        {
            data_node *node;
            printf("data exist\n");
            node = queue->head;
            printf("data:%s\n", node->data);
            lua_pushstring(L, node->data);
            queue->head = node->next;
            queue->length --;
            if(queue->length != 0)
                queue->head->prev = NULL;
            free(node);
            break;
        }
    }
    return 1;
}

static const struct luaL_Reg reg_funcs[] = 
{
    {"serv_start", mongoose_start},
    {"serv_stop", mongoose_stop},
    {"get_data", get_data},
    {NULL, NULL}
};

int luaopen_p2p_server(lua_State *L) 
{
    luaL_newlib(L, reg_funcs);
    return 1;
}