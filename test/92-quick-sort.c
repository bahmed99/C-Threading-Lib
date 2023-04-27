#include <stdio.h>
#include <stdlib.h>
#include "thread.h"
#include <sys/time.h>
#include <assert.h>


int *arr; 


int checkSort(int *arr, int size) {
    int i;
    for (i = 0; i < size - 1; i++) {
        if (arr[i] > arr[i + 1]) {
            return 0;
        }

    }
    return 1 ;
}
       

int partition(int *arr, int low, int high) {
    int pivot = arr[high];
    int i = low - 1;
    int j;
    for (j = low; j < high; j++) {
        if (arr[j] <= pivot) {
            i++;
            int temp = arr[i];
            arr[i] = arr[j];
            arr[j] = temp;
        }
    }
    int temp = arr[i+1];
    arr[i+1] = arr[high];
    arr[high] = temp;
    return i + 1;
}

void *quick_sort(void *arg) {
    int low = *((int *) arg);
    int high = *((int *) arg + 1);
    if (low < high) {
        int pi = partition(arr, low, high);
        thread_t thread_left, thread_right;
        int left_args[2] = {low, pi - 1};
        int right_args[2] = {pi + 1, high};
        thread_create(&thread_left,  quick_sort, left_args);
        thread_create(&thread_right,  quick_sort, right_args);
        thread_join(thread_left, NULL);
        thread_join(thread_right, NULL);
    }
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
    for (int i = 0; i < size; i++) {
        arr[i] = rand() % 1000;
    }

    gettimeofday(&tv1, NULL);
    int args[2] = {0, size - 1};
    thread_t thread;
    err= thread_create(&thread,  quick_sort, args);
    assert(!err);

    err=thread_join(thread, NULL);
    assert(!err);
    gettimeofday(&tv2, NULL);

    us = (tv2.tv_sec-tv1.tv_sec)*1000000+(tv2.tv_usec-tv1.tv_usec);

    printf("Le tri rapide est effectué avec %s en %ld us\n",checkSort(arr,size)?"success":"echec" , us);    

    free(arr);
    return 0;
}