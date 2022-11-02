#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#define LEN 7
int v1[LEN] = {2, 3, 8, 1, 2, 0, 4};
int v2[LEN] = {1, 6, 3, 8, 10, 2, -4};
//int v1[LEN] = {1, 2, 3, 4, 5, 10, 20};
//int v2[LEN] = {2, 2, 2, 2, 2, 10, 20};

#define MAX_BUF 3
int LEN_BUF = 0;
int PUT_IDX_BUF = 0;
int GET_IDX_BUF = 0;
int buffer[MAX_BUF];

int scalar_product = 0;

pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void* producer(void* arg){
    printf("[PRODUCER THREAD STARTED]\n");

    for (int i=0; i < LEN; i++) {
        printf("[ITERATION %d: PRODUCER THREAD ACQUIRES THE LOCK]\n", i);
        pthread_mutex_lock(&mutex);
        while (LEN_BUF == MAX_BUF) {
            printf("[ITERATION %d: PRODUCER THREAD IS WAITING]\n", i);
            pthread_cond_wait(&cond, &mutex);
        }

        int current_product = v1[i] * v2[i];
        buffer[PUT_IDX_BUF] = current_product;
        PUT_IDX_BUF = (PUT_IDX_BUF + 1) % MAX_BUF;
        LEN_BUF++;
        printf("[ITERATION %d: PRODUCER THREAD PRODUCED %d]\n", i, current_product);

        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&mutex);
    }

    printf("[PRODUCER THREAD COMPLETED TASK]\n");
    return NULL;
}

void* consumer(void* arg){
    printf("[CONSUMER THREAD STARTED]\n");

    for (int i=0; i < LEN; i++) {
        printf("[ITERATION %d: CONSUMER THREAD ACQUIRES THE LOCK]\n", i);
        pthread_mutex_lock(&mutex);
        while (LEN_BUF == 0) {
            printf("[ITERATION %d: CONSUMER THREAD IS WAITING]\n", i);
            pthread_cond_wait(&cond, &mutex);
        }

        int current_product = buffer[GET_IDX_BUF];
        scalar_product += current_product;
        GET_IDX_BUF = (GET_IDX_BUF + 1) % MAX_BUF;
        LEN_BUF--;
        printf("[ITERATION %d: CONSUMER THREAD CONSUMED %d]\n", i, current_product);

        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&mutex);
    }

    printf("[PRODUCER THREAD COMPLETED TASK]\n");
    return NULL;
}


int main(){
    pthread_t p, c;

    if(pthread_create(&p, NULL, producer, NULL)!=0){
        perror("[ERROR: cannot create thread]");
        exit(1);
    }

    if(pthread_create(&c, NULL, consumer, NULL)!=0){
        perror("[ERROR: cannot create thread]");
        exit(1);
    }

    pthread_join(p, NULL);
    pthread_join(c, NULL);

    pthread_cond_destroy(&cond);
    pthread_mutex_destroy(&mutex);

    printf("[SCALAR PRODUCT: %d]\n", scalar_product);
    return 0;
}