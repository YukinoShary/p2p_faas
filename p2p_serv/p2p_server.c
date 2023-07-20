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
    char *data;
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

static void mongoose_start(lua_State* L);
static void mongoose_stop();
static void fn(struct mg_connection *c, int ev, void *ev_data, void *fn_data);
static void multipart_processing(struct mg_connection *c, char* data);
static int allowed_ip(char *ip, char *nodes_tb[]);
static void set_data(const char* result);
static int get_data(lua_State* L);
static queue_node* queue_dequeue();
int luaopen_p2p_server(lua_State *L);

static queue_node* queue_dequeue()
{
    queue_node* de = queue->head;
    if(queue->length == 0)
    {
        perror("queue is empty!\n");
        return NULL;
    }
    else
    {
        queue->head = de->next;
        queue->head->prev = NULL;
    }
    queue->length --;
    return de;
}

/*initialize a server program and start it*/
static void mongoose_start(lua_State* L)
{
    int n, i;
    queue = malloc(sizeof(data_queue));
    queue->length = 0;
    queue->head = NULL;
    queue->rear = NULL;
    printf("start to intialize a server\n");
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
    
    mg_mgr_init(&mgr);                                        // Init manager4
    mg_http_listen(&mgr, "http://0.0.0.0:8080", fn, &mgr);  // Setup listener
    printf("server started\n");
    while(1)
    {
        mg_mgr_poll(&mgr, 1000);                         // Event loop
    }
}

/*stop the server program and release the resource*/
static void mongoose_stop(lua_State* L)
{
    MHD_stop_daemon(daemon_);
    printf("server stop\n");
}

/*request callback function*/
static void fn(struct mg_connection *c, int ev, void *ev_data, void *fn_data)
{
    int ack;
    struct mg_http_message *hm;
    char *content_type;
    size_t content_length; 
    printf("get request\n");
    if(!allowed_ip(url, (char **)cls))
    {
        printf("from the umknown ip.\n");
        mg_http_reply(c, 403, "", "%s", "Invalidated node\n");
    }
    
    *hm = (struct mg_http_message *) ev_data;
    if(strcmp(hm->method->p, "POST");
    {
        content_type = mg_http_get_header(hm, "Content-Type");
        if(ev == MG_EV_HTTP_MSG)
        {
            if(strcmp(content_type, "application/octet-stream") == 0)
            {
                ack = 1;
                set_data(hm->body.ptr);
                mg_http_reply(c, 200, NULL, ack);
            }
            else if(strcmp(content_type, "multipart/form-data") == 0)
            {
                multipart_processing(c, hm->body.ptr);
                ack = 2;
                mg_http_reply(c, 200, NULL, ack);
            }
            else if(strcmp(content_type, "text/plain") == 0)
            {
                set_data(hm->body.ptr);
                content_length = hm->body.len;
                ack = 3;
                mg_http_reply(c, 200, NULL, ack);
            }
        }
        /*  send the original HTTP chunked data to lua stack
        else if(ev == MG_EV_HTTP_CHUNK)
        {
            struct data_node node;
            node = malloc(sizeof(data_node));
            if(connection_count > 8)
            {
                perror("too many connection\n");
            }
            set_data(hm->chunk.ptr);
        }
        */
        if((!content_type))
        {
            perror("failed to get the content_type\n");
            return;
        }
    }
}

static void multipart_processing(struct mg_connection *c, char* data)
{
    
}

/*ip white list*/
static int allowed_ip(char *ip, char *nodes_tb[])
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

/*push received data to the lua stack*/
static void set_data(const char *result)
{
    struct data_node *node;
    node = malloc(sizeof(data_node));
    node->data = result;
    node->prev = queue->rear;
    node->next = NULL;
    queue->length ++;
    if(queue->length == 1)
    {
        queue->head = node;
        queue->rear = node;
    }
    queue->rear = node;
}

static int get_data(lua_State* L)
{
    printf("get data\n");
    while(1)
    {
        if(queue->length > 0)
        {
            printf("data exist\n");
            lua_pushstring(L, queue->head->data);
            queue->head = queue->head->next;
            free(queue->head->prev);
            queue->head->prev = NULL;
            queue->length --;
            break;
        }
    }
    return 1;
}

static const struct luaL_Reg reg_funcs[] = 
{
    {"serv_start", microhttpd_start},
    {"serv_stop", microhttpd_stop},
    {"get_data", get_data},
    {NULL, NULL}
};

int luaopen_p2p_server(lua_State *L) 
{
    luaL_newlib(L, reg_funcs);
    return 1;
}