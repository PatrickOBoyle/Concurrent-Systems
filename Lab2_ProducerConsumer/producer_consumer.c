#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
 
#define NUM_CONSUMERS 4
#define NUM_PRODUCERS 2
#define MAX_BUFFER_ITEMS 20
#define TOTAL_TO_PRODUCE 100
 
pthread_mutex_t lock= PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t consumer_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t produce_cond = PTHREAD_COND_INITIALIZER;
struct buffer_item ** buffer;
int currentItems;
int total_produced;
 
//defines item so that we can pass information with production items
struct buffer_item{
    int prod;
    int seq ;
};
 
//creates item with relevant info
struct buffer_item * new_buffer_item(int producer, int sequence_num){
    struct buffer_item *new;
    new = malloc(sizeof(struct buffer_item));
    new -> prod = producer;
    new ->seq = sequence_num;
    return new;
}
 
void *Producer(void *threadid){
    int numProduced = 0;
    int id = (int)threadid;
    while(total_produced < TOTAL_TO_PRODUCE){
        struct  buffer_item * item = new_buffer_item(id, numProduced);
       
        for(int i = 0; i < 4000000; i++){
            // production delay
        }
 
        pthread_mutex_lock(&lock);
        if(currentItems < MAX_BUFFER_ITEMS){
            //add to buffer
            buffer[currentItems] =  item;
            //increment local production variable
            numProduced++;
            fprintf(stderr, "item produced\n" );
            //update global variables - total and current
            currentItems++;
            total_produced++;
 
            //unlock and broadcast
            pthread_mutex_unlock(&lock);
            pthread_cond_broadcast(&consumer_cond);
        } else {
            //fprintf(stderr, "Buffer full, waiting\n" );
            pthread_mutex_unlock(&lock);
            pthread_cond_wait(&produce_cond, &lock);
            //fprintf(stderr, "Buffer non-full, resuming\n" );
        }
 
    }
}
 
void *Consumer(void *threadid){
    int num_consumed = 0;
    int id = (int)threadid;
    while(total_produced < TOTAL_TO_PRODUCE || currentItems > 0){
        pthread_mutex_lock(&lock);
        if(currentItems > 0){
 
            //buffer acts as a stack, get item to consume
            struct buffer_item * to_consume = buffer[currentItems-1];
            //update local and global variables
            num_consumed++;
            currentItems--;
            fprintf(stderr, "Consuming item %i, produced by producer %i. This consumer(%i) has consumed %i items\n", to_consume -> seq, to_consume -> prod, id, num_consumed );
            //unlock and broadcast
            pthread_mutex_unlock(&lock);
            pthread_cond_broadcast(&produce_cond);
 
            for(int i = 0; i < 10000; i++){
                //delay simulate consumption
            }
        } else {
            //fprintf(stderr, "Buffer empty, waiting\n");
            pthread_mutex_unlock(&lock);
            pthread_cond_wait(&consumer_cond, &lock);
            //fprintf(stderr, "Buffer non-empty, resuming\n" );
        }
 
        // deadlock without the unlock
        pthread_mutex_unlock(&lock);
        // only one consumer getting items without broadcast here
        pthread_cond_broadcast(&consumer_cond);
    }
}
 
int main(int argc){
    buffer = malloc(sizeof(struct buffer_item **) * MAX_BUFFER_ITEMS);
    currentItems = 0;
    total_produced = 0;
    pthread_t producers[NUM_PRODUCERS];
    pthread_t consumers[NUM_CONSUMERS];
    pthread_mutex_init(&lock, NULL);
    int rc;
    for (int t=0;t<NUM_PRODUCERS;t++) {
        printf("Creating producer %d\n",t);
        rc = pthread_create(&producers[t],NULL,Producer,(void *)t);
        if(rc){
            printf("ERROR return code from pthread_create(): %d\n",rc);
            exit(-1);
        }
    }
    for (int t=0;t<NUM_CONSUMERS;t++) {
        printf("Creating consumer %d\n",t);
        rc = pthread_create(&consumers[t],NULL,Consumer,(void *)t);
        if(rc){
            printf("ERROR return code from pthread_create(): %d\n",rc);
            exit(-1);
        }
    }
    for(int t=0;t<NUM_PRODUCERS;t++) {
        pthread_join( producers[t], NULL);
    }
    for(int t=0;t<NUM_CONSUMERS;t++) {
        pthread_join( consumers[t], NULL);
    }
    return 0;
}