#include <stdio.h>
#include <stdlib.h>
#include "thread.h"
#include <sys/time.h>
#include <assert.h>

int *arr;

int checkSort(int *arr, int size)
{
    int i;
    for (i = 0; i < size - 1; i++)
    {
        if (arr[i] > arr[i + 1])
        {
            return 0;
        }
    }
    return 1;
}
void *insertion_sort(void *arg)
{
    int l = *((int *)arg);
    int r = *((int *)arg + 1);
    for (int i = l + 1; i <= r; i++)
    {
        int val = arr[i];
        int j = i - 1;
        while (j >= l && arr[j] > val)
        {
            arr[j + 1] = arr[j];
            j--;
        }
        arr[j + 1] = val;
    }
    return NULL;
}

void merge(int l, int m, int r)
{
    int n_left = m - l + 1;
    int n_right = r - m;
    int left[n_left], right[n_right];
    for (int i = 0; i < n_left; i++)
    {
        left[i] = arr[l + i];
    }
    for (int j = 0; j < n_right; j++)
    {
        right[j] = arr[m + j + 1];
    }
    int i = 0, j = 0, k = l;
    while (i < n_left && j < n_right)
    {
        if (left[i] <= right[j])
        {
            arr[k] = left[i];
            i++;
        }
        else
        {
            arr[k] = right[j];
            j++;
        }
        k++;
    }
    while (i < n_left)
    {
        arr[k] = left[i];
        i++;
        k++;
    }
    while (j < n_right)
    {
        arr[k] = right[j];
        j++;
        k++;
    }
}


int main(int argc, char *argv[])
{

    int err;

    if (argc < 2)
    {
        printf("argument manquant: longueur du tableau\n");
        return -1;
    }

    int size = atoi(argv[1]);

    struct timeval tv1, tv2;
    unsigned long us;

    arr = malloc(sizeof(int) * size);
    for (int i = 0; i < size; i++)
    {
        arr[i] = rand() % 1000;
    }

    gettimeofday(&tv1, NULL);
    thread_t thread_left, thread_right;
    int left_args[2] = {0, size / 2 - 1};
    int right_args[2] = {size / 2, size - 1};
    err = thread_create(&thread_left, insertion_sort, left_args);
    assert(!err);
    err = thread_create(&thread_right, insertion_sort, right_args);
    assert(!err);
    err = thread_join(thread_left, NULL);
    assert(!err);

    err = thread_join(thread_right, NULL);

    assert(!err);

    merge(0, size / 2 - 1, size - 1);

    gettimeofday(&tv2, NULL);

    us = (tv2.tv_sec - tv1.tv_sec) * 1000000 + (tv2.tv_usec - tv1.tv_usec);

    printf("Le tri par insertion est effectué avec %s en %ld us\n", checkSort(arr, size) ? "success" : "echec", us);

    free(arr);
    return 0;
}