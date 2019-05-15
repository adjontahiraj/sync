#include "threads.h"

//using namespace std;
int thread_id_generator = 0;
//tcb threads[MAX_THREADS];

//tcb queue_scheduler

tcb *running_thread = nullptr;
unsigned long int running_sem_id = 0;
vector<tcb*> ready_queue;
//vector<semaphore*> sem_vec;

//signal and timer structs
struct sigaction sig;
//struct itimerval timer;
//sigset_t al;


//function to start the timer
void lock() {
    //we pass in the itimerval struct
    sigprocmask(SIG_BLOCK, &sig.sa_mask, NULL);   
}

//function to stop the timer during thread context switch
void unlock() {
    sigprocmask(SIG_UNBLOCK, &sig.sa_mask, NULL);
}

int sem_init(sem_t *sem, int pshared, unsigned int value) {
    //checking for max sem value
    if(value > 65536) {
        exit(1);
    }
    semaphore* s = new semaphore;
    s->val = value;
    s->id = running_sem_id++;
    //giving the semaphore a special id number and
    sem ->__align = (long int) s;
    //s->sem = sem;
    //lock();
    //sem_vec.push_back(s);
    //unlock();
    return 0;
}

int sem_destroy(sem_t *sem) {
    //int i = sem->__align
    //getting the semaphore by using the id
    semaphore *s = (semaphore*)sem->__align;
    if(!s){
        return -1;
    }
    delete s;
    //erasing using id + top of vector
    //sem_vec.erase(sem_vec.begin()+i);
    return 0;
}

int sem_post(sem_t *sem) {
    //int i = sem->__align;
    semaphore *s = (semaphore*)sem->__align;
    if(s->val ==0) {
        lock();
        tcb* tmp = running_thread;
        unlock();
        if(tmp->sem_block = s->id) {
            lock();
            tmp->st = READY;
            tmp->sem_block = -1;
            unlock();
        }else{
            for(int x =0; x<ready_queue.size(); x++) {
                lock();
                if(ready_queue[x]->sem_block == s->id) {
                    ready_queue[x]->st = READY;
                    ready_queue[x]->sem_block = -1;
                    unlock();
                    break;
                }
            }
        }
    }
    s->val++;
    return 0; //STUB
}

//used to change between thread contexts when timer hits 50ms
void context_handler(int signal) {

    //flag can be changed by other threads
    //volatile int flag = 0;

    //changing thread state to ready and setting saving jump_register
    lock();
    running_thread->st = READY;

    if(!setjmp(running_thread->thread_reg)) {
        //threads[running_thread->thread_id] = *running_thread;
        // if(flag == 1) {
        //    flag = 0;
        //    return;
        // }
        //cout<<"Post flag"<<endl;
        //putting the thread to back to queue - round robin
        ready_queue.push_back(running_thread);


        if(ready_queue[0]->st == TERMINATED || ready_queue[0]->st == SUSPENDED || ready_queue[0]->sus_thread != -1) {
            ready_queue.push_back(ready_queue[0]);
            ready_queue.erase(ready_queue.begin());
        }
        running_thread = ready_queue[0];
        //cout<<"post tcb"<<endl;
        running_thread->st = RUNNING;
        //removing the current running thread from the queue

        ready_queue.erase(ready_queue.begin());

        if(thread_id_generator == 0) {
             unlock();
        }

        longjmp(running_thread->thread_reg,1);
        unlock();
    }
}
int sem_wait(sem_t *sem) {
    cout<<"WAIT"<<endl;
    //int i = sem->__align;
    lock();
    semaphore *s = (semaphore*)sem->__align;
    cout<<"WAIT2"<<endl;
    if(s->val == 0) {
        cout<<"WAIT3"<<endl;
        cout<<"WAIT4"<<endl;
        running_thread->st = SUSPENDED;
        running_thread->sem_block = s->id;
        cout<<"WAIT5"<<endl;
        unlock();
        cout<<"WAIT6"<<endl;
        kill(getpid(),SIGALRM);
        cout<<"WAIT7"<<endl;
        s->val-=1;
    }
    return 0; 
}
void pthread_init() {
    
    //set up main tcb block
    tcb * this_thread = new tcb();
    this_thread -> thread_id = thread_id_generator++;
    this_thread -> st = RUNNING;
    //setting the running thread to this thread
    lock();
    running_thread = this_thread;
    //clearing the signal mem and setting the context handler as the sig.sa_handler
    //memset(&sig, 0, sizeof(sig));
    sig.sa_handler =&context_handler;
    sig.sa_flags= SA_NODEFER;
    sigemptyset(&sig.sa_mask);
    //setting the sig.sa_handler as the handler for SIGVTALRM
    sigaction(SIGALRM, &sig, NULL);
    sigaddset(&sig.sa_mask, SIGALRM);
    // sigemptyset(&al);
	// sigaddset(&al, SIGALRM);
    //setting alarm to trigger every 50msec
    // timer.it_value.tv_sec = 0;
    // timer.it_value.tv_usec = ALARM_TIME;
    // timer.it_interval.tv_sec = 0;
    // timer.it_interval.tv_usec = ALARM_TIME;
    // setitimer(ITIMER_VIRTUAL, &timer, 0);
    //start_timer();
	//struct sigaction signal;
//    sig.sa_handler=context_handler;
//    sig.sa_flags= SA_NODEFER;
//
//    sigemptyset(&sig.sa_mask);
//
//    sigaction(SIGALRM,&sig,NULL);
//
    ualarm(50000,50000);
    unlock();
}


static long int i64_ptr_mangle(long int p) {
    long int ret;
    asm(" mov %1, %%rax;\n"
        " xor %%fs:0x30, %%rax;"
        " rol $0x11, %%rax;"
        " mov %%rax, %0;"
    : "=r"(ret)
    : "r"(p)
    : "%rax"
    );
    return ret;
}

//wrapper function to run the thread funct
int wrapper() {
	void *(*start_routine)(void*)=running_thread->start_routine;
    void *arg=running_thread->args;
    //running_thread->funct_return = start_routine(arg);
    //running_thread->st=TERMINATED;
    pthread_exit(start_routine(arg));

}

int pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine)(void*), void *arg) {
    if(thread_id_generator == 0) {

        pthread_init();
    }
    lock();

    tcb *t = new tcb();

    t->thread_id = thread_id_generator++;
    t->st = READY;
    t ->start_routine = start_routine;

    t ->args = arg;
    //t ->funct_return = NULL;

    //cout<<"MAIN TCB init"<<endl;

    if(setjmp(t->thread_reg)) {
        cerr<<"ERROR: could not set jump"<<endl;
        return -1;
    }

    t ->thread_reg[0].__jmpbuf[6] = i64_ptr_mangle((unsigned long int)t->pg_ptr);
    t ->thread_reg[0].__jmpbuf[7] = i64_ptr_mangle((unsigned long int)wrapper);

    ready_queue.push_back(t);
    unlock();

    *thread = t->thread_id;

    return 0;
}

int check_ready_q(int thread) {
    int found_thread = -1;
    lock();
    for(int i = 0; i < ready_queue.size(); i++) {
        if(ready_queue[i]->thread_id == thread) {
            found_thread = i;
            break;
        }
    }
    unlock();
    return found_thread;
}


tcb* find_thread(pthread_t thread) {
    lock();
    tcb* t = running_thread;
    
    for(int i = 0; i < ready_queue.size(); i++) {
        if(ready_queue[i]->thread_id == thread) {
            t = ready_queue[i];
            unlock();
            return t;
        }
    }
    unlock();
    return t;
}
int pthread_join(pthread_t thread, void **value_ptr){
    //deadlock on self
    // if(pthread_self() == thread) {
    //     return -1;
    // }
    
    tcb* blocking_thread;
    int i = check_ready_q(thread);
    if (i == -1) { //not found
        return ESRCH;
    }
    //lock();
    blocking_thread = find_thread(thread);
    //unlock();
    if(blocking_thread->st == TERMINATED) {
        *value_ptr = blocking_thread->funct_return;
        return 0;
    }
    if(blocking_thread->blocking != nullptr) {
        return EINVAL;
    }
    blocking_thread->blocking = running_thread;
    lock();
    running_thread->st = SUSPENDED; 
    
    //making handler move on to next
    context_handler(1);

    if(value_ptr != NULL) {
        *value_ptr = blocking_thread->funct_return;
    }
    unlock();
    return 0;
}

void pthread_exit(void *value_ptr) {
    lock();
    //cout<<"Called Exit: " << endl;
    if(running_thread->thread_id != 0 ) {
        //delete running_thread -> stack_ptr;
        //tcb * del = running_thread;
        //delete[] running_thread->stack_ptr;
        running_thread->st = TERMINATED;
        running_thread->funct_return = value_ptr;
        if(running_thread->blocking != nullptr) {
            running_thread->blocking->st = READY;
        }
        running_thread = ready_queue[0];
        running_thread->st = RUNNING;
        ready_queue.erase(ready_queue.begin());
        //delete del;
		//start_timer();
        longjmp(running_thread->thread_reg,1);
    }
    //start_timer();
    unlock();
    exit(0);
}

pthread_t pthread_self(void) {
    //return running_thread -> thread_id;
    lock();
    if(running_thread== nullptr) {
        unlock();
        exit(0);
    }
    pthread_t tmp = running_thread->thread_id;
    unlock();
    return tmp;
    
}

