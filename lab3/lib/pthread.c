#include "pthread.h"
#include "lib.h"
/*
 * pthread lib here
 * 用户态多线程写在这
 */

ThreadTable tcb[MAX_TCB_NUM];
int current;

#define RESERVED_SPACE 10

int add_one(int i){ //set this function to avoid switching too fast

    return (i+1)%MAX_TCB_NUM;
}

void pthread_initial(void){
    int i;
    for (i = 0; i < MAX_TCB_NUM; i++) {
        tcb[i].state = STATE_DEAD;
        tcb[i].joinid = -1;

        tcb[i].first=1;

        //clear the stack space for the return value
        for(int j=1;j<=RESERVED_SPACE;j++)
            tcb[i].stack[MAX_STACK_SIZE-j]=0;

    }
    tcb[0].state = STATE_RUNNING;
    tcb[0].pthid = 0;
    current = 0; // main thread
    return;
}

int pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine)(void *), void *arg){
    
    int i=0;
    for(;i<MAX_TCB_NUM;i++)
    {
        if(tcb[i].state==STATE_DEAD)
            break;
    }
    if(i==MAX_TCB_NUM)
        return -1;

    *thread=i;
    
    //set create thread
    tcb[i].state=STATE_RUNNABLE;
    tcb[i].pthid=i;
    tcb[i].stackTop=(uint32_t)(&tcb[i].cont);
    tcb[i].pthArg=(uint32_t)arg; //the address of the arg of the function
    tcb[i].stack[MAX_STACK_SIZE-RESERVED_SPACE-1]=tcb[i].pthArg; //set the argument of the function and reserve space
    tcb[i].cont.esp=tcb[i].cont.ebp=(uint32_t)(&tcb[i].stack[MAX_STACK_SIZE-RESERVED_SPACE-2]); //set the stack frame
    tcb[i].cont.eip=(uint32_t)(*start_routine); //set the entry of the function

    tcb[current].first=0;

    //clear the stack space for the return value
    for(int j=1;j<=RESERVED_SPACE;j++)
        tcb[i].stack[MAX_STACK_SIZE-j]=0;

    return 0;
}

void pthread_exit(void *retval){
    
    for(int i=1;i<=RESERVED_SPACE;i++)
    {
        if(tcb[current].stack[MAX_STACK_SIZE-i]!=0)
        {
            *(void **)tcb[current].stack[MAX_STACK_SIZE-i]=retval; //pass this value to the second argument in pthread_join
            break;
        }
    }

    tcb[current].state=STATE_DEAD;

    for(int i=0;i<MAX_TCB_NUM;i++)
    {
        if(tcb[i].joinid==current)
        {
            tcb[i].joinid=-1;
            tcb[i].state=STATE_RUNNABLE;
        }
    }
    
    int i=add_one(current);
    for(;i!=current;i=add_one(i))
    {
        if(tcb[i].state==STATE_RUNNABLE)
            break;
    }

    tcb[i].state=STATE_RUNNING;

    current=i;

    //enter thread i
    if(tcb[current].first==0)
    {
        asm volatile("movl %0, %%eax":"=m"(tcb[current].cont.eax));
        asm volatile("movl %0, %%ecx":"=m"(tcb[current].cont.ecx));
        asm volatile("movl %0, %%edx":"=m"(tcb[current].cont.edx));
        asm volatile("movl %0, %%ebx":"=m"(tcb[current].cont.ebx));
        asm volatile("movl %0, %%esi":"=m"(tcb[current].cont.esi));
        asm volatile("movl %0, %%edi":"=m"(tcb[current].cont.edi));
    }
    else
    {
        tcb[current].first=0;
    }

    asm volatile("movl %0, %%ebp":"=m"(tcb[current].cont.ebp));
    asm volatile("movl %0, %%esp":"=m"(tcb[current].cont.esp));
    asm volatile("jmp *%0":"=m"(tcb[current].cont.eip));

    return;
}

int pthread_join(pthread_t thread, void **retval){

    asm volatile("movl %%edi, %0":"=m"(tcb[current].cont.edi));
    asm volatile("movl %%esi, %0":"=m"(tcb[current].cont.esi));
    asm volatile("movl %%ebx, %0":"=m"(tcb[current].cont.ebx));
    asm volatile("movl %%edx, %0":"=m"(tcb[current].cont.edx));
    asm volatile("movl %%ecx, %0":"=m"(tcb[current].cont.ecx));
    tcb[current].cont.eax=0;

    asm volatile("movl (%%ebp), %%eax":"=a"(tcb[current].cont.ebp)); //get the current ebp
    asm volatile("leal 0x8(%%ebp), %%eax":"=a"(tcb[current].cont.esp)); //get the current esp
    asm volatile("movl 0x4(%%ebp), %%eax":"=a"(tcb[current].cont.eip));

    //set the waiting thread's stack right place to store the return value
    for(int i=1;i<=RESERVED_SPACE;i++)
    {
        if(tcb[thread].stack[MAX_STACK_SIZE-i]==0)
        {
            tcb[thread].stack[MAX_STACK_SIZE-i]=(uint32_t)retval;
            break;
        }
    }

    tcb[current].joinid=thread;
    tcb[current].state=STATE_BLOCKED; //block the current thread

    int i=add_one(current);
    for(;i!=current;i=add_one(i))
    {
        if(i!=0&&tcb[i].state==STATE_RUNNABLE)
            break;
    }

    tcb[i].state=STATE_RUNNING;

    current=i;

    //enter thread i
    if(tcb[current].first==0)
    {
        asm volatile("movl %0, %%eax":"=m"(tcb[current].cont.eax));
        asm volatile("movl %0, %%ecx":"=m"(tcb[current].cont.ecx));
        asm volatile("movl %0, %%edx":"=m"(tcb[current].cont.edx));
        asm volatile("movl %0, %%ebx":"=m"(tcb[current].cont.ebx));
        asm volatile("movl %0, %%esi":"=m"(tcb[current].cont.esi));
        asm volatile("movl %0, %%edi":"=m"(tcb[current].cont.edi));
    }
    else
    {
        tcb[current].first=0;
    }

    asm volatile("movl %0, %%ebp":"=m"(tcb[current].cont.ebp));
    asm volatile("movl %0, %%esp":"=m"(tcb[current].cont.esp));
    asm volatile("jmp *%0":"=m"(tcb[current].cont.eip));
    
    return 0;
}

int pthread_yield(void){

    asm volatile("movl %%edi, %0":"=m"(tcb[current].cont.edi));
    asm volatile("movl %%esi, %0":"=m"(tcb[current].cont.esi));
    asm volatile("movl %%ebx, %0":"=m"(tcb[current].cont.ebx));
    asm volatile("movl %%edx, %0":"=m"(tcb[current].cont.edx));
    asm volatile("movl %%ecx, %0":"=m"(tcb[current].cont.ecx));
    tcb[current].cont.eax=0;

    asm volatile("movl (%%ebp), %%eax":"=a"(tcb[current].cont.ebp)); //get the old ebp
    asm volatile("leal 0x8(%%ebp), %%eax":"=a"(tcb[current].cont.esp)); //get the old esp
    asm volatile("movl 0x4(%%ebp), %%eax":"=a"(tcb[current].cont.eip));

    tcb[current].state=STATE_RUNNABLE;

    int i=add_one(current);
    for(;i!=current;i=add_one(i))
    {
        if(i!=0&&tcb[i].state==STATE_RUNNABLE)
            break;
    }

    tcb[i].state=STATE_RUNNING;

    current=i;

    //enter thread i
    if(tcb[current].first==0)
    {
        asm volatile("movl %0, %%eax":"=m"(tcb[current].cont.eax));
        asm volatile("movl %0, %%ecx":"=m"(tcb[current].cont.ecx));
        asm volatile("movl %0, %%edx":"=m"(tcb[current].cont.edx));
        asm volatile("movl %0, %%ebx":"=m"(tcb[current].cont.ebx));
        asm volatile("movl %0, %%esi":"=m"(tcb[current].cont.esi));
        asm volatile("movl %0, %%edi":"=m"(tcb[current].cont.edi));
    }
    else
    {
        tcb[current].first=0;
    }

    asm volatile("movl %0, %%ebp":"=m"(tcb[current].cont.ebp));
    asm volatile("movl %0, %%esp":"=m"(tcb[current].cont.esp));
    asm volatile("jmp *%0":"=m"(tcb[current].cont.eip));

    return 0;
}
