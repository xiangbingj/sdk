#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

#define MAX_BUF_SIZE (1024)

char buf_send[MAX_BUF_SIZE];
char buf_recv[MAX_BUF_SIZE];
struct timeval get_time[2][2];
double total_rate[2] = {0.0};

int main(int argc, char const *argv[])
{
    int flag = 0;
    if(argc != 2)
    {
        printf("Usage:\n\t%s <ip address>\n", argv[0]);
        return -1;
    }

     int sockfd = socket(AF_INET,SOCK_STREAM,0);
     if (0 > sockfd)
     {
          perror("socket");
          return -1;
     }
     struct sockaddr_in addr = {};
     addr.sin_family = AF_INET;
     addr.sin_port = htons(4321);
     addr.sin_addr.s_addr = inet_addr(argv[1]);

    if (connect(sockfd, (struct sockaddr*)&addr, sizeof(addr)) < 0)
    {
        printf("connect error: %s(errno: %d)\n", strerror(errno), errno);
        return 0;
    }
    socklen_t addr_len = sizeof(struct sockaddr_in);
    uint32_t i = 0;
    uint32_t packet_num = 0;
    for (i = 0; i < MAX_BUF_SIZE-1; i++) {
        buf_send[i] = 'a' + (i % 26);
    }
    buf_send[MAX_BUF_SIZE-1] = '\0';
     while(1)
     {
         gettimeofday(&get_time[0][0], NULL);
          send(sockfd,buf_send,sizeof(buf_send),0);

          recv(sockfd,buf_recv,sizeof(buf_recv),0);
          gettimeofday(&get_time[0][1], NULL);
          for (i = 0; i < MAX_BUF_SIZE-1; i++)
          {
            if(buf_recv[i] != ('a' + (i % 26)))
            {
                flag = 1;
                break;
            }
          }
          if(flag)
          {
              printf("receive fail i = %d %c %c \n", i, buf_recv[i], ('a' + (i % 26)));
              flag = 0;
          }
          else
          {
              total_rate[0] += 1000*1000*8.0*MAX_BUF_SIZE/((get_time[0][1].tv_sec-get_time[0][0].tv_sec)*1000*1000 + (get_time[0][1].tv_usec-get_time[0][0].tv_usec));
              //total_rate[1] += 1000*1000*8.0*MAX_BUF_SIZE/((get_time[1][1].tv_sec-get_time[1][0].tv_sec)*1000*1000 + (get_time[1][1].tv_usec-get_time[1][0].tv_usec));
              packet_num++;
              if((packet_num % 50) == 0)
              {
                printf("[%f Mbps]\n", total_rate[0]/1024.0/1024.0/50.0);
                total_rate[0] = 0;
                total_rate[1] = 0;
                packet_num = 0;
              }
          }

          memset(buf_recv, 0, MAX_BUF_SIZE);
     }
     close(sockfd);
     return 0;
}
