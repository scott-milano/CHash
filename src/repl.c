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
 * @brief This software module provides network sharing of the Hash structures.
 * Sharing is done via Multcast packets.  Sets and Deletes are shared on the
 * network.  When new clients join they ask for the current set of values.
 *
 * The network feature is started 
 * @addtogroup HASH
 * @{
 */
 
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <stddef.h>

#include "hash.h"
#include "entry.h"
#include "mcast.h"
#include "repl.h"

/** Return current time as long in miliseconds since epoch.
 * @return System time in miliseconds since epoch
 */
static inline long mtime() {
   struct timeval tp;
   gettimeofday( &tp, NULL );
   return tp.tv_sec*1000 + tp.tv_usec/1000;
}

int processOp(list_store_t *store,uint8_t op,id_t node,void* data,int bytes);
static int socketReady(int sock, long tmOutms);

/** Type for IDs */
typedef uint32_t id_t;

struct repl_info {
    int sock;
    uint16_t port;
    id_t id;
    id_t self;
    uint32_t state;
#define STATE_START 0       /**< Startup processing */
#define STATE_RUN 1         /**< Runtime state */
#define STATE_START_SYNC 2  /**< Initialize and start a SYNC Transmission */
#define STATE_SYNC 3        /**< Transmit SYNC packets */
    uint32_t maxCount;      /**< Maximum number of entries in OP_STAT responses */
    id_t maxNode;           /**< Node with most entries */
    pthread_cond_t startCond;
    pthread_mutex_t netLock;
};

typedef struct __attribute__ ((packed)) {
    uint16_t size;
    id_t hashid;
    id_t nodeid;
    uint8_t op;
    uint8_t data[];
} packet_t;

/* Mcast shared operations */
#define OP_NOP 0xef     /**< Null Op, wakes select/recv to check exit flag */
#define OP_SET 1        /**< Set a value */
#define OP_DEL 2        /**< Delete a value */
#define OP_SYNC 3       /**< Request that node provides all entries */
#define OP_STAT_REQ 4   /**< Request Status information */
#define OP_STAT 5       /**< Status info reply with count of entries */

#ifdef HDEBUG
static char *opLu[] = {
    [OP_NOP] = "NOP",
    [OP_SET] = "SET",
    [OP_DEL] = "DEL",
    [OP_SYNC] = "SYNC",
    [OP_STAT_REQ] = "STAT_REQ",
    [OP_STAT] = "STAT",
};
#endif

/** Macro to send message header
 * @param store List master structure
 * @param op opcode for message
 * @param size number of bytes in complete message
 * @return number of bytes sent or error from sendto.  This includes the
 * header size.
 */
static inline int send_msg(list_store_t *store, uint8_t op,void *buf,int size)
{
    int ret=-1;
    int msize=offsetof(packet_t,data)+size;
    packet_t *pkt=NULL;

    /* Alocate send buffer and populate */
    if ((pkt=malloc(msize))) {
        pkt->size=msize;
        pkt->hashid=store->id;
        pkt->nodeid=store->net->self;
        pkt->op=op;
        if (buf) {
            memcpy(&pkt->data[0],buf,size);
        }
        ret=mcast_send(store->net->sock,store->port,pkt,msize,0);
        if (ret<msize) {
            fprintf(stderr,"send_MSg size issue: expected: %d, actual: %d\n",msize,ret);
        }
        free(pkt);
    }
    return ret;
}

/** This function serves as the main thread for recieving hash updates */
static void *store_replication(void *vstore)
{
    /* Global data structures */
    list_store_t *store=(list_store_t *)vstore;
    repl_info_t *net=store->net;    /**< Pointer to data structure */

    uint8_t *buf;                   /**< Buffer for received data */
    int bytes;                      /**< Number of bytes read */
    /** Header Size, includes Hash ID, Self ID and OP */
    int hdrSize=offsetof(packet_t,data);
    int size=5*(hdrSize+store->key.sz(NULL)+store->value.sz(NULL));


    /* Alocate buffer for received data packets */
    buf=calloc(1,size);
    if (buf==NULL) {
        fprintf(stderr,"memory allocation failure: %d bytes\n",5*size);
        return NULL;
    }


    dbg("Replication Thread Started");

    /* Generate own id from pthread_self() */
    net->self=pthread_self();

    /* Set initial state */
    net->state=STATE_START;

    /* See if existing nodes have more data */
    send_msg(store,OP_STAT_REQ,NULL,0);

    dbg("Replicator Starting: id: %x, self: %x, sock: %d port: %u",store->id,
            net->self,net->sock,store->port);

    /* Notify parent thread we are running */
    pthread_mutex_lock(&net->netLock);
    pthread_cond_broadcast(&net->startCond);
    pthread_mutex_unlock(&net->netLock);

    /* Runtime Processing */
    {
        /* Assign offsets in data buffer */
        int delay=200;      /**< Time to sleep waiting for next packet */
        long startTime=mtime()+delay;   /**< Time to wait before entering run
                                          *  state */
        int index=0;          /**< Current index for sync response */
        _entry_t *eptr=NULL;  /**< Pointer for entries in sync response */

        /* Main loop */
        while (store->port) {
            /* Wait for/process incoming data */
            if (socketReady(net->sock,delay)) {
                int hdrSize=offsetof(packet_t,data);

                /* Read all available packets until empty */
                while ((bytes=mcast_recv(net->sock,buf,size,MSG_DONTWAIT))>0) {
                    packet_t *ptr=(packet_t *)buf;
                    assert(ptr->size==bytes);
                    /* discard packets from self or other hashes */
                    if ((ptr->hashid!=store->id)||(ptr->nodeid==net->self)) continue;

                    /* Process message */
                    dbg("process(%s): n: %x b: %d",opLu[ptr->op],ptr->nodeid,bytes);
                    /* Process message */
                    processOp(store,ptr->op,ptr->nodeid,ptr->data,ptr->size-hdrSize);
                }
            }

            /* Run State machine */
            switch (net->state) {
                case STATE_RUN:
                    break;
                case STATE_START_SYNC:
                    dbg("Sync requested");
                    eptr=store->list;
                    index=0;
                    net->state=STATE_SYNC;
                    /* Fall through */
                case STATE_SYNC:
                    if (index<store->index) {
                        dbg("Sync %d of %lu",index,store->index);
                        repl_update(store,eptr++);
                        index++;
                    } else {
                        net->state=STATE_RUN;
                        delay=500;
                    }
                    break;
                case STATE_START:
                    if (mtime()>startTime) {
                        net->state=STATE_RUN;
                        delay=500;
                        /* See if existing nodes have more data */
                        if (net->maxCount>store->index) {
                            dbg("Requesting update from id: %x count: %d\n",
                                    net->maxNode,net->maxCount);
                            send_msg(store,OP_SYNC,&net->maxNode,sizeof(net->maxNode));
                        }
                    }
                default:
                    break;
            } /* End of switch */

          } /* End of main loop */
    }

    /* Free allocated buffers */
    if (buf) free(buf);
    dbg("Thread Closed");
    return NULL;
}

/**
 * Processes one operation message.
 * @param store pointer to global data structure
 * @param decoded operation
 * @param data pointer to data buffer
 * @param bytes number of bytes in buffer
 * @return number of bytes processed
 * @note: The size checks may not be needed if message header size is checked
 * by the calling function.
 */
int processOp(list_store_t *store,uint8_t op,id_t node,void* data,int bytes)
{
    repl_info_t *net=store->net;
    void *key=data;
    void *value;
    int keySize;

    /* Ensure hash type match */
    switch (op) {
        case OP_SET:
            if (bytes>0) {
                _entry_t *eptr=NULL;
                keySize=store->key.sz(key);
                value=key+keySize;
                if (bytes>=0) {
                    pthread_mutex_lock(&store->lock);
                    eptr=_hash_search(store,key,value);
                    pthread_mutex_unlock(&store->lock);
                    if (eptr) dbgentry(eptr);
                    else dbg("Insert Failure");
                    bytes-=(keySize+store->value.sz(value));
                } else {
                    dbg("Value Size Error, OP_Set, bytes: %d, key: %d val: %lu",
                            bytes,keySize,store->value.sz(value));
                }
            } else {
                dbg("Key Size Error, OP_Set, bytes: %d",bytes);
            }
            break;
        case OP_DEL:
            if (bytes>0) {
                keySize=store->key.sz(key);
                if (bytes>=keySize) {
                    int index;
                    pthread_mutex_lock(&store->lock);
                    index=_find_index(store,key);
                    dbgindex(index);
                    if (index>=0) {
                        _delete_entry(store,index);
                    }
                    pthread_mutex_unlock(&store->lock);
                    bytes-=keySize;
                } else {
                    dbg("Key Size Error, OP_Set, bytes: %d",bytes);
                }
            } else {
                dbg("Key Missing, OP_Del, bytes: %d",bytes);
            }
            break;
        case OP_STAT_REQ: {
            /* Request for list index size for a sync operation */
            if (store->index) {
                /* Only send if there are items availble to send */
                send_msg(store,OP_STAT,&store->index,sizeof(store->index));
            }
        } break;
        case OP_STAT: {
            size_t *sz=(size_t *)(data);
            if (bytes>=sizeof(*sz)) {
                if (*sz>net->maxCount) {
                    net->maxCount=*sz;
                    net->maxNode=node;
                }
            }
            bytes-=sizeof(store->index);
            } break;
        case OP_SYNC:
            dbg("sync: %x ?= %d",*((id_t*) data),net->self);
            if (*((id_t*) data)==net->self) {
                net->state=STATE_START_SYNC;
                dbg("State -> STATE_START_SYNC");
            }
            bytes-=sizeof(net->self);
            break;
        case OP_NOP:
            dbg("nop: %d, %d",bytes,net->sock);
            break;
        default:
            dbg("unknown: %u",op);
            break;
    }
    return bytes;
}

/** Transmit an update packet */
bool repl_update(list_store_t *store,_entry_t *eptr)
{
    repl_info_t *net=store->net;
    int bytes=0;

    /* Ensure buffer is allocated */
    if ((net)&&(net->sock)) {
        int keysize=store->key.sz(eptr->key);
        int valsize=store->value.sz(eptr->val);
        int msize=offsetof(packet_t,data)+keysize+valsize;
        packet_t *pkt=NULL;

        dbgentry(eptr);
        /* Alocate send buffer and populate */
        if ((pkt=malloc(msize))) {
            pkt->size=msize;
            pkt->hashid=store->id;
            pkt->nodeid=store->net->self;
            pkt->op=OP_SET;
            memcpy(&pkt->data[0],eptr->key,keysize);
            memcpy(&pkt->data[keysize],eptr->val,valsize);
            bytes=mcast_send(store->net->sock,store->port,pkt,msize,0);
            free(pkt);
        }
        if (bytes>=(keysize+valsize)) return true;
        fprintf(stderr,"Set size issue: Bytes: %d, Size: %d\n",bytes,keysize+valsize);
    }
    return false;
}

bool repl_remove(list_store_t *store,void *keyref)
{
    repl_info_t *net=store->net;
    int bytes=0;

    /* Ensure buffer is allocated */
    if ((net)&&(net->sock)) {
        int keysize=store->key.sz(keyref);
        bytes=send_msg(store,OP_DEL,keyref,keysize);
        if (bytes>=keysize) return true;
        fprintf(stderr,"Del size issue: Bytes: %d, Size: %d\n",bytes,keysize);
    }
    return false;
}

/** Allocate resources and start the receive thread */
bool repl_start(list_store_t *store)
{
    bool ret=true;

    if ((store->port)&&(!store->net)) {
        repl_info_t *net=calloc(1,sizeof(*(store->net)));
        if (net) {
            pthread_mutex_init(&net->netLock,NULL);
            pthread_cond_init(&net->startCond,NULL);

            /* Initialize Multicast */
            net->sock=mcast_init(store->port);
            if (net->sock<=0) {
                if (net) free(net);
                return false;
            } else {
                /* Save net structure where thread will access */
                store->net=net;

                /* Lock condition */
                pthread_mutex_lock(&net->netLock);
                pthread_create(&store->nethandle, NULL, store_replication,(void*)store);

                /* Wait for start */
                pthread_cond_wait(&net->startCond, &net->netLock);
                pthread_mutex_unlock(&net->netLock);
            }
        } else {
            fprintf(stderr,"Memory Allocation Error: %d\n",(int) sizeof(*(store->net)));
            ret=false;
        }
    } else {
        ret=false;
    }
    return ret;
}


/** Close the socket to wake the thread so it sees the exit condition. Then 
 * join to gather thread resources */
void repl_close(list_store_t *store)
{
    repl_info_t *net=store->net;

    /* Ensure network is up */
    if ((net)&&(net->sock)) {
        /* Set port to 0 so thread will exit next loop */
        store->port=0;
        close(store->net->sock);
        store->net->sock=0;
        pthread_join(store->nethandle,NULL);

        /* Clean up resources */
        if (store->net) free(store->net);
        store->net=NULL;
    }
}

/** Indicates when data is available on the listen or accepted socket using
 * select.  This avoids blocking, forever in the accept call, and allows the
 * thread to exit
 * @param sFd Listen socket file descriptor
 * @param cFd Active client socket file descriptor
 * @return File descriptor with data or 0 on a timeout
 */
static int socketReady(int sock, long tmOutms)
{
    fd_set read_fds;
    int status;
    int nfds=0;
    struct timeval wait;

    /* Set Timeout */
    memset(&wait,0x00,sizeof(wait));
    wait.tv_sec = tmOutms/1000;
    wait.tv_usec = tmOutms%1000;

    /* Setup select inputs */
    FD_ZERO(&read_fds);

    /* Add descriptors */
    if (sock>0) {
        nfds = sock;
        FD_SET(sock, &read_fds);
    }

    /* Pause on select */
    status = select(nfds + 1, &read_fds, (fd_set *)0, (fd_set *)0, &wait);

    /* Check status */
    if (status==0) {
        /* Timeout */
        return 0;
    } else if (status<0) {
        /* Error, sleep to stop runaway error, also occurs on close */
        usleep(10000);
        return 0;
    } else {
        /* Check if server connection is ready */
        if (FD_ISSET(sock, &read_fds)) {
            return sock;
        }
    }
    return 0;
}

/**@}*/
