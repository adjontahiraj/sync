#include <iostream>
#include <vector>
#include <unistd.h>
#include <ucontext.h>
#include <cstdlib>
#include <sys/time.h>
#include <signal.h>
#include <cstring>
#include <string.h>
#include <algorithm>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <iostream>
#include <sys/ucontext.h>
#include <pthread.h>
#include <setjmp.h>
#include <malloc.h>
#include <semaphore.h>



#ifndef THREADS_H
#define THREADS_H

#define MAX_THREADS 129    //128 + main thread
#define STACK_SIZE  32767  //2^15 -1
#define ALARM_TIME  50000  // in micro-sec => 50msec
#define SEM_VALUE_MAX 65536
using namespace std;
enum STATE {
    READY,      // in queue waiting for turn
    RUNNING,    // top of queue
    SUSPENDED,  // when waiting on other thread
    TERMINATED  // exited
};

struct tcb {
    tcb(){ //initializing an empty tcb block
        thread_id = 0;
        st = READY;
        stack_ptr = new unsigned long [STACK_SIZE];
        pg_ptr = stack_ptr + STACK_SIZE / 8 - 3;
        args = nullptr;
        funct_return = nullptr;
        start_routine = nullptr;
        blocking = nullptr;
        sus_thread = -1; //initialize to -1, not suspended by anyone
    }
    pthread_t thread_id;    //thread id
    STATE st;                //state of thread
    unsigned long *stack_ptr;//stack pointer
    unsigned long *pg_ptr;   //program pointer
    void*args;              //---Inputs----
    void*funct_return;
    void*(*start_routine)(void*);
    int sem_block = -1;
    tcb* blocking;
    jmp_buf thread_reg;     //the register buffer
    pthread_t sus_thread; //used for pthread_join
};

//using the struct to hold the sem which has the id number in align
struct semaphore {
    //sem_t* sem;
    int id;
    unsigned long val;
};

void lock();

void unlock();

int sem_init(sem_t *sem, int pshared, unsigned int value);

int sem_destroy(sem_t *sem);

int sem_wait(sem_t *sem);

int sem_post(sem_t *sem);

int pthread_join(pthread_t thread, void **value_ptr);

int signal_init();

void context_handler();

void pthread_init();

static long int i64_ptr_mangle(long int p);

int wrapper();

int pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine)(void*), void *arg);

void pthread_exit(void *value_ptr);

pthread_t pthread_self(void);

#endif
