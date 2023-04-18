#include <stdio.h>
#include <stdlib.h>
#include "thread.h"
#include <sys/time.h>
#include <assert.h>


int *arr; 
int *tmp; 


int checkSort(int *arr, int size) {
    int i;
    for (i = 0; i < size - 1; i++) {
        if (arr[i] > arr[i + 1]) {
            return 0;
        }

    }
    return 1 ;
}
       


void merge(int l, int m, int r) {
    int i, j, k;
    for (i = l, j = m + 1, k = l; i <= m && j <= r; k++) {
        if (arr[i] <= arr[j]) {
            tmp[k] = arr[i];
            i++;
        } else {
            tmp[k] = arr[j];
            j++;
        }
    }
    while (i <= m) {
        tmp[k] = arr[i];
        i++;
        k++;
    }
    while (j <= r) {
        tmp[k] = arr[j];
        j++;
        k++;
    }
    for (i = l; i <= r; i++) {
        arr[i] = tmp[i];
    }
}

void *merge_sort(void *arg) {
    int l = *((int *) arg);
    int r = *((int *) arg + 1);
    if (l >= r) {
        return NULL;
    }
    int m = (l + r) / 2;
    thread_t thread_left, thread_right;
    int left_args[2] = {l, m};
    int right_args[2] = {m + 1, r};
    thread_create(&thread_left,  merge_sort, left_args);
    thread_create(&thread_right,  merge_sort, right_args);
    thread_join(thread_left, NULL);
    thread_join(thread_right, NULL);
    merge(l, m, r);
    return NULL;
}

int main(int argc, char *argv[]) {

    int err;

    if (argc < 2) {
        printf("argument manquant: longueur du tableau\n");
        return -1;
    }

    int size= atoi(argv[1]);

    struct timeval tv1, tv2;
    unsigned long us;

    arr = malloc(sizeof(int) * size);
    tmp = malloc(sizeof(int) * size);
    for (int i = 0; i < size; i++) {
        arr[i] = rand() % 1000;
    }

    gettimeofday(&tv1, NULL);
    int args[2] = {0, size - 1};
    thread_t thread;
    err= thread_create(&thread,  merge_sort, args);
    assert(!err);

    err=thread_join(thread, NULL);
    assert(!err);
    gettimeofday(&tv2, NULL);

    us = (tv2.tv_sec-tv1.tv_sec)*1000000+(tv2.tv_usec-tv1.tv_usec);

    printf("Le tri fusion est effectué avec %s en %ld us\n",checkSort(arr,size)?"success":"echec" , us);    

    free(arr);
    free(tmp);
    return 0;
}
