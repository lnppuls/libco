#include "co.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <setjmp.h>
#include <sys/types.h>

#define STACK_SIZE 4 * 1024

enum CO_STATUS {
    NEW,
    RUNNING,
    WAITING,
    DEAD
};

struct CO {
    char co_name[10];
    void (*co_func)(void *);
    void *co_arg;
    enum CO_STATUS status;
    struct CO *waiter;
    struct CO *next;
    jmp_buf co_buf;
    __attribute__((aligned(16))) uint8_t co_stack[STACK_SIZE];
};

static Co *current_co;
static int co_num = 0;
static Co *main_co;

Co * co_start(char *name, void (*func)(void *), void *arg) {
    Co *co = (Co *)malloc(sizeof(Co));
    co_num++;
    memset(co->co_name, 0, 32);
    strncpy(co->co_name, name, strlen(name));
    co->co_func = func;
    co->co_arg = arg;
    co->status = NEW;
    
    if (current_co == NULL) {
        Co *main = (Co *)malloc(sizeof(Co));
        current_co = main;
        strcpy(main->co_name, "main");
        current_co->co_arg = NULL;
        current_co->next = current_co;
        current_co->waiter = NULL;
        current_co->status = RUNNING;
        co_num++;
        main_co = current_co;
    }
    Co *end = current_co;
    while (end->next != current_co) {
        end = end->next;
    }
    end->next = co;
    co->next = current_co;
    co_yield();
    return co;
}

void co_yield() {
    int val = setjmp(current_co->co_buf);
    if (val == 0) {

        if(co_num > 1) {
            int rand_num = rand();
            // 随机调度器
            rand_num = (rand_num % co_num) + 1;
            while(rand_num) {
                rand_num--;
                current_co = current_co->next;
            }
        }
        if(current_co->status == NEW) {
            current_co->status = RUNNING;
            asm volatile (
                "movq %0, %%rsp; movq %2, %%rdi; call *%1;"
                :
                : "r"((uintptr_t)&current_co->co_stack[STACK_SIZE]), "d"(current_co->co_func), "a"((uintptr_t)current_co->co_arg)
                : "memory"
            );
            current_co->status = DEAD;
            co_num--;
            Co *get_next = current_co;
            while (get_next->next != current_co) {
                get_next = get_next->next;
            }
            // 将死协程剔除出调度列表
            get_next->next = current_co->next;
            current_co = get_next;

            /*
                当子协程结束时不能退出，因为栈是新创建的没有调用者
                也就没有返回地址，这时应该直接切换到住协程处理等待事件
            */
            //切换到主协程运行
            current_co = main_co;
            longjmp(main_co->co_buf, 1);
        } else {
            longjmp(current_co->co_buf, 1);
        }
    } else {
        return;
    }
}

void co_wait(Co *co) {
    co->waiter = current_co;
    current_co->status = WAITING;
    while (co->status != DEAD) {
        co_yield();
    }
    free(co);
}