#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<unistd.h>
#include<pthread.h>
#include<errno.h>
#include<arpa/inet.h>



struct child_value
{
    int sock_value;
};


struct child_link
{
   char link_ip[16];
   int link_port;
   int to_fd;
}linking;

/*void getline1(char* ch)
{
    char **i=(char **)malloc(sizeof(char*)*10);
    
    int h,k;
    for(h=0;h<128;h++)
    {
       i[h]=(char*)malloc(128*sizeof(char));
    } 
    memset(i,0,sizeof(char)*128*10);
    for(h=0,k=0;k<10;k++,h++)
    {
        for(;ch[h]!='\n';h++)
        {
            i[k][h]=ch[h];
        }
        if(ch[h]=='\n')
            i[k][h]=ch[h];
        puts(i[k]);
    }
    
    
}*/

/*struct sockaddr_in* get_value(char *ch)
{
     
}*/

void child_read(void * data)
{
    void * rbuf=malloc(1024);
    for(;;)
    {
        while((read(linking.to_fd,rbuf,sizeof(char)*1024))!=0)
        {
            send(((struct child_value*)data)->sock_value,rbuf,sizeof(rbuf),0);
        }
    }
    pthread_exit(0);
}

void child_write(void *data)
{
    void *buf=malloc(1024);
    int a=0;
    while((a=read(((struct child_value*)data)->sock_value,buf,sizeof(char)*1024))!=0)
    {
        send(linking.to_fd,buf,sizeof(buf),0);
    }
    if(a==0)
    {
        char *ch="HTTP/1.1 200 Connection Established\r\n\r\n";
        send(((struct child_value*)data)->sock_value,ch,sizeof(ch),0);
    }
    for(;;)
    {
        while((a=read(((struct child_value*)data)->sock_value,buf,sizeof(char)*1024))!=0)
        {
            send(linking.to_fd,buf,sizeof(buf),0);
        }

    }
    pthread_exit(0);
}
void child_handle(void * data)
{
   pthread_t rid,wid;
   rid=wid=0;
   
   /*while((read(((struct child_value *)data)->sock_value,buf,sizeof(char)*1024))!=0)
   {
       getline1(buf);   
   }*/
   if((pthread_create(&rid,NULL,(void*)child_write,(void*)&data))==-1)
   {
       printf("child_handle create read_pthread error!\n");
       exit(-1);
   }
   if((pthread_create(&wid,NULL,(void*)child_read,NULL))==-1)
   {
       printf("child_handle create write_pthread error!\n");
       exit(-1);
   }
   pthread_join(rid,NULL);
   pthread_join(wid,NULL);
   pthread_exit(0);
       
}


int main()
{
    extern int errno;
    int listenfd=0;
    struct sockaddr_in serveraddr;
    bzero(&serveraddr,sizeof(serveraddr));
    int client_fd=0;
    
    int reuse=1;

    char wait_toip[16];
    int wait_port;
    scanf("Please input the IP:%s",linking.link_ip);
    scanf("Please input the port:%d",&linking.link_port);

    struct sockaddr_in to_server;
    bzero(&to_server,sizeof(to_server));
   
    to_server.sin_family=AF_INET;
    to_server.sin_port=htons(linking.link_port);
    to_server.sin_addr.s_addr=inet_addr(linking.link_ip);
    
    int to_serverfd=socket(AF_INET,SOCK_STREAM,0);
    if(to_serverfd==-1)
    {
        printf("to_serverfd error!\n");
        exit(-1);
    }
    if(connect(to_serverfd,(struct sockaddr*)&to_server,sizeof(to_server))<0)
    {
        printf("connect error!\n");
        exit(-1);
    }
    linking.to_fd=to_serverfd;

    
    if((listenfd=socket(AF_INET,SOCK_STREAM,0))==-1)
    {
        printf("Socket error!\n");
        exit(-1);
    }
   
    if(setsockopt(listenfd,SOL_SOCKET,SO_REUSEADDR,&reuse,sizeof(reuse))<0)
    {
        perror("setsockopt error!\n");
        return -1;
    }
    serveraddr.sin_family=AF_INET;
    serveraddr.sin_port=htons(80);
    serveraddr.sin_addr.s_addr=inet_addr("127.0.0.1");
    
    if((bind(listenfd,(struct sockaddr*)&serveraddr,sizeof(serveraddr)))==-1)
    {
       printf("Bind error!  %d\n",errno);
       exit(-1);
    }
   
    listen(listenfd,1024);
    for(;;)
    {
        if((client_fd=accept(listenfd,NULL,NULL))==-1)
        {
            printf("Accept error!INVALLID_SOCKET!\n");
            continue;
        }
        int a=0;
        pthread_t ctid;
        
        struct child_value child;
        child.sock_value=client_fd;
    
        a=pthread_create(&ctid,NULL,(void *)child_handle ,(void*)&child);
    }
    return 0;
}














