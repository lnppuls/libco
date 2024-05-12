
typedef struct CO Co;

// 创建线程返回线程信息的结构体
Co * co_start(char *name, void (*func)(void *), void *arg);

// 出让线程
void co_yield();

// 等待co线程执行完
void co_wait(Co *co);

