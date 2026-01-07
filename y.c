// y.c
// Implémentation minimale de write() via syscall Linux ARM EABI
int write(int fd, const void *buf, unsigned len) {
  register int r0 asm("r0") = fd;
  register const void *r1 asm("r1") = buf;
  register unsigned r2 asm("r2") = len;
  register int r7 asm("r7") = 4; // __NR_write (ARM EABI Linux)

  asm volatile ("svc 0"
                : "+r"(r0)
                : "r"(r1), "r"(r2), "r"(r7)
                : "memory");
  return r0;
}

void hello(void);

int main(void) {
  write(1, "Start main\n", 11);
  hello();
  write(1, "End main\n", 9);

  while (1) {}   // boucle volontaire
  return 0;
}
