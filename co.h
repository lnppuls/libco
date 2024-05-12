typedef struct CO Co;

// 创建协程
Co * co_start(char *name, void (*func)(void *), void *arg);

// 出让线程
void co_yield();

// 等待协程执行完
void co_wait(Co *co);

