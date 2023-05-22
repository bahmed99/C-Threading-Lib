#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include "thread.h"

#define REC_LENGTH (4096*32)

void* rec_memory_consumer(int dummy) {
    if (dummy == 0) {
        return (void*) 0xdeadbeef;
    }
    printf("%d: %p\n", REC_LENGTH-dummy, &dummy);
    rec_memory_consumer(dummy - 1);
}

static void * thfunc1(void *dummy __attribute__((unused)))
{
  thread_yield();
  
  return rec_memory_consumer(REC_LENGTH);
}

static void * thfunc2(void *dummy __attribute__((unused)))
{
  for(int i=0; i<10; i++) {
    thread_yield();
  }
  return (void*) 0xbeefdead;
}

int main()
{
  printf("PAGESIZE: %ld\n", sysconf(_SC_PAGE_SIZE));


  thread_t th1, th2;
  int err;
  void *res = NULL;

  err = thread_create(&th1, thfunc1, NULL);
  assert(!err);
  err = thread_create(&th2, thfunc2, NULL);
  assert(!err);

  err = thread_join(th1, &res);
  assert(err == -1);
  assert(res == NULL);
  printf("join1 returned an error (which is the normal behaviour for stack overflow)\n");
  
  err = thread_join(th2, &res);
  assert(!err);
  assert(res == (void*) 0xbeefdead);

  printf("join2 OK\n");
  return 0;
}
