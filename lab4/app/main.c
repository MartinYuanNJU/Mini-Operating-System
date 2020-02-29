#include "lib.h"
#include "types.h"

/*int data = 0;

int uEntry(void) {
    
    int i = 4;
    int ret = 0;
    sem_t sem;
    printf("Father Process: Semaphore Initializing.\n");
    ret = sem_init(&sem, 2);
    if (ret == -1) {
        printf("Father Process: Semaphore Initializing Failed.\n");
        exit();
    }

    ret = fork();
    if (ret == 0) {
        while( i != 0) {
            i --;
            printf("Child Process: Semaphore Waiting.\n");
            sem_wait(&sem);
            printf("Child Process: In Critical Area.\n");
        }
        printf("Child Process: Semaphore Destroying.\n");
        sem_destroy(&sem);
        exit();
    }
    else if (ret != -1) {
        while( i != 0) {
            i --;
            printf("Father Process: Sleeping.\n");
            sleep(128);
            printf("Father Process: Semaphore Posting.\n");
            sem_post(&sem);
        }
        printf("Father Process: Semaphore Destroying.\n");
        sem_destroy(&sem);
        exit();
    }
    
    return 0;
}*/

void produce(int pid, sem_t *product, sem_t *mutex)
{
    int i=8;
    while(i!=0)
    {
        i--; //produce product
        printf("pid: %d, producer: %d, operation: try lock\n",pid,pid-1);
        sem_wait(mutex);
        printf("pid: %d, producer: %d, operation: locked\n",pid,pid-1);
        printf("pid: %d, producer: %d, operation: produce product %d\n",pid,pid-1,8-i);
        sem_post(product);
        sem_post(mutex);
        printf("pid: %d, producer: %d, operation: unlock\n",pid,pid-1);
        printf("pid: %d, producer: %d, operation: sleep\n",pid,pid-1);
        sleep(128);
    }
    printf("pid: %d, producer: %d, producing done\n",pid,pid-1);
    return;
}

void consume(int pid, sem_t *product, sem_t *mutex)
{
    int i=4;
    while(i!=0)
    {
        printf("pid: %d, consumer: %d, operation: try consume product %d\n",pid,pid-3,5-i);
        sem_wait(product);
        printf("pid: %d, consumer: %d, operation: try lock\n",pid,pid-3);
        sem_wait(mutex);
        printf("pid: %d, consumer: %d, operation: locked\n",pid,pid-3);
        printf("pid: %d, consumer: %d, operation: consume product %d\n",pid,pid-3,5-i);
        sem_post(mutex);
        printf("pid: %d, consumer: %d, operation: unlock\n",pid,pid-3);
        i--; //consume product
        printf("pid: %d, consumer: %d, operation: sleep\n",pid,pid-3);
        sleep(128);
    }
    printf("pid: %d, consumer: %d, consuming done\n",pid,pid-3);
    return;
}

int uEntry(void)
{
    //define two semaphores, initialize them
    sem_t product;
    sem_t mutex;
    sem_init(&product, 0);
    sem_init(&mutex, 1);

    //fork six child process
    int ret1=fork();
    if(ret1>0) {
        int ret2=fork();
        if(ret2>0) {
            int ret3=fork();
            if(ret3>0) {
                int ret4=fork();
                if(ret4>0) {
                    int ret5=fork();
					if(ret5>0) {
						fork();
					}
                }
            }
        }
    }

    //get pid to figure out the identity and call relative function
    int pid=getpid();
    if(pid>=2&&pid<=3) {
        produce(pid, &product, &mutex);
	}
    else if(pid>=4&&pid<=7) {
        consume(pid, &product, &mutex);
	}

	exit();
    return 0;
}
