// a.c
#include <unistd.h>
// z.c
int write(int fd, const void *buf, unsigned len);

void hello(void) {
  write(1, "Hello from Z\n", 13);
}

