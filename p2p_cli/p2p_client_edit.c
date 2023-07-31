#include <stdlib.h>
#include <stdio.h>
#include <curl/curl.h>
#include <lua5.4/lua.h>
#include <lua5.4/lualib.h>
#include <lua5.4/lauxlib.h>
#include <sys/time.h>

static void curl_init(lua_State *lua_S);
int luaopen_p2p_client(lua_State *L);
int timeval_subtract (struct timeval *result, struct timeval *x, struct timeval *y);


static void curl_init(lua_State *L)
{
    int n, ret;
    FILE *fd;
    printf("start to initialize a client\n");
    char *send_data, *target_ip;
    struct stat file_info;
    CURL *curl;
    curl_off_t speed_upload, total_time;
    struct curl_slist *hs = NULL;
    struct timeval *start_t, *end_t, *time_result;
    n = luaL_checknumber(L, 1);
    curl = curl_easy_init();
    //time start
    start_t = malloc(sizeof(struct timeval));
    end_t = malloc(sizeof(struct timeval));
    time_result = malloc(sizeof(struct timeval));
    gettimeofday(start_t, NULL);
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
            curl_slist_free_all(hs);
            gettimeofday(end_t, NULL);
            curl_easy_cleanup(curl);
        }
        break;
        case 3:
        /* chunked transfer */
        target_ip = lua_tostring(L, 2);
        send_data = lua_tostring(L, 3); 
        fd = fopen(send_data, "rb");
        if(fstat(fileno(fd), &file_info) != 0)
            return 1;
        if(curl)
        {
            CURLcode ret;
            curl_easy_setopt(curl, CURLOPT_URL, target_ip);
            curl_easy_setopt(curl, CURLOPT_POST, 1L);
            curl_easy_setopt(curl, CURLOPT_READDATA, fd);
            curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE,
                     (curl_off_t)file_info.st_size);
            hs = curl_slist_append(hs, "Transfer-Encoding: chunked");
            hs = curl_slist_append(hs, "Content-Type: application/octet-stream");
            curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, hs);
            printf("set option\n");
            ret = curl_easy_perform(curl);
            printf("post send\n");
            if(ret != CURLE_OK)
            {
                curl_easy_strerror(ret);
            }
            else
            {
                curl_easy_getinfo(curl, CURLINFO_SPEED_UPLOAD_T, &speed_upload);
                curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME_T, &total_time);

                fprintf(stderr, "Speed: %lu bytes/sec during %lu.%06lu seconds\n",
                (unsigned long)speed_upload,
                (unsigned long)(total_time / 1000000),
                (unsigned long)(total_time % 1000000));
            }
            curl_slist_free_all(hs);
            gettimeofday(end_t, NULL);
            curl_easy_cleanup(curl);
        }
        break;
    }  

    if(timeval_subtract(time_result, end_t, start_t) == 1)
    {
        printf("ERROR during computing esplased time.\n");
    }
    else
    {
        printf("used time: %ld.%06ld\n", time_result->tv_sec, time_result->tv_usec);
    }
    free(start_t);
    free(end_t);
    free(time_result);
    fclose(fd);
}

static 

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

/* Subtract the ‘struct timeval’ values X and Y,
   storing the result in RESULT.
   Return 1 if the difference is negative, otherwise 0. */

int timeval_subtract (struct timeval *result, struct timeval *x, struct timeval *y)
{
  /* Perform the carry for the later subtraction by updating y. */
  if (x->tv_usec < y->tv_usec) {
    int nsec = (y->tv_usec - x->tv_usec) / 1000000 + 1;
    y->tv_usec -= 1000000 * nsec;
    y->tv_sec += nsec;
  }
  if (x->tv_usec - y->tv_usec > 1000000) {
    int nsec = (x->tv_usec - y->tv_usec) / 1000000;
    y->tv_usec += 1000000 * nsec;
    y->tv_sec -= nsec;
  }

  /* Compute the time remaining to wait.
     tv_usec is certainly positive. */
  result->tv_sec = x->tv_sec - y->tv_sec;
  result->tv_usec = x->tv_usec - y->tv_usec;

  /* Return 1 if result is negative. */
  return x->tv_sec < y->tv_sec;
}