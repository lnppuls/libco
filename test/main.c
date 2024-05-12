#include <stdio.h>
#include "../co.h"

void entry(void *arg) {
  int num = 100000;
  while (num--) {
    printf("%s : %d\n", (const char *)arg, num);
    fflush(stdout);
    co_yield();
  }
}
int main() {
  Co *co1 = co_start("co1", entry, "a");
  Co *co2 = co_start("co2", entry, "b");
  Co *co3 = co_start("co3", entry, "c");
  Co *co4 = co_start("co4", entry, "d");
  co_wait(co1);
  co_wait(co2);
  co_wait(co3);
  co_wait(co4);
}