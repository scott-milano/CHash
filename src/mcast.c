/**
 * @file
 * @author Scott Milano
 * @copyright Copyright 2019 Scott Milano
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */
 
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>


/**< Lock for list list access */
pthread_mutex_t lock=PTHREAD_MUTEX_INITIALIZER;
/** Static address for all sends/recv calls */
struct sockaddr_in mcAddr;
/** Static socket */
int mcSock=0;
int mcId=0;

typedef struct {
    uint32_t id;
    uint16_t size;
    uint8_t buf[];
} packet_t;

#define BASE_ADDRESS "239.0.0.1"
int mcast_init(uint16_t port)
{
    struct ip_mreq mreq;
    int opt=1;
    int sock;
    pthread_mutex_lock(&lock);

    /* Get a uniq id to throw away own packets */
    mcId=pthread_self();
    mcId<<=16;
    mcId+=getpid();
 
    /* set up socket */
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("socket");
        pthread_mutex_unlock(&lock);
        return sock;
    }

    bzero((char *)&mcAddr, sizeof(mcAddr));
    mcAddr.sin_family = AF_INET;
    mcAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    mcAddr.sin_port = htons(port);
 
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))<0) {
        perror("setsockopt reuse");
        close(sock);
        pthread_mutex_unlock(&lock);
        return -1;
    }         

    if (bind(sock, (struct sockaddr *) &mcAddr, sizeof(mcAddr)) < 0) {
        perror("bind");
        close(sock);
        pthread_mutex_unlock(&lock);
        return -1;
    }    
    mreq.imr_multiaddr.s_addr = inet_addr(BASE_ADDRESS);         
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);         
    if (setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP,&mreq, sizeof(mreq)) < 0) {
        perror("setsockopt mreq");
        close(sock);
        pthread_mutex_unlock(&lock);
        return -1;
    }         
    mcAddr.sin_addr.s_addr = inet_addr(BASE_ADDRESS);
    pthread_mutex_unlock(&lock);
    return sock;
}

int mcast_recv(int sock, uint8_t *buf, int size)
{
    socklen_t addrlen;
    int bytes;
    struct sockaddr_in addr;
    addrlen = sizeof(addr);

    bytes = recvfrom(sock, buf, size, 0,(struct sockaddr *) &addr, &addrlen);
    if (bytes < 0) {
         perror("recvfrom");
    }

    return bytes;
}

int mcast_send(int sock,uint16_t port, uint8_t *buf, int size)
{
    socklen_t addrlen;
    int bytes;
    addrlen = sizeof(mcAddr);

    pthread_mutex_lock(&lock);
    /* Set port */
    mcAddr.sin_port = htons(port);
    /* send */
    bytes = sendto(sock,buf,size, 0,(struct sockaddr *) &mcAddr, addrlen);
    if (bytes < 0) {
        perror("sendto");
    }

    pthread_mutex_unlock(&lock);
    return bytes;
}

