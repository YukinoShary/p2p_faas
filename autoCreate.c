#include<stdio.h>
#include<stdlib.h>
#include<time.h>

#define bufferSize 4096

int main(int argc, char *argv[])
{
    if(argc != 2)
    {
        perror("invalid argument");
        return -1;
    }
    time_t t;
    srand((unsigned) time(&t));
    for(int i = 1;i < argc; i++)
    {
        int size;
        time_t start, end;
        char buffer[bufferSize];
        size = 0;
        FILE *fp;
        fp = fopen(argv[i], "wb");
        start = clock();
        printf("size:%d\n", 100*(1 << 20));
        while(size < 100*(2 << 20))
        {
            for(int j = 0; j <= (bufferSize - 1); j++)
            {
                buffer[j] = (rand() % 126);
            }
            fwrite(buffer, 1, bufferSize, fp);
            size += bufferSize;
            printf("%d/%d\n", size, 1024*1024*100);
        }
        fclose(fp);
        end = clock();
        printf("file create complete\n");
        printf("Time spend: %f\n", (float)(end - start)/CLOCKS_PER_SEC);
    }
}