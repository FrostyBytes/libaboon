/*
  libaboon/syscall.c
  copyright (c) 2017 by andrei borac
*/

/*
  user: %rdi, %rsi, %rdx, %rcx, %r8 and %r9
  kern: %rdi, %rsi, %rdx, %r10, %r8 and %r9
*/

__asm__
(
  "lbi_syscall_6_asm:" LB_LF
  "movq 0(%r9), %r10" LB_LF
  "movq 8(%r9), %r9" LB_LF
  "movq %rcx, %rax" LB_LF
  "syscall" LB_LF
  "retq" LB_LF
  
  "lbi_syscall_5_asm:" LB_LF
  "movq %r9, %r10" LB_LF
  "lbi_syscall_3_asm:" LB_LF
  "movq %rcx, %rax" LB_LF
  "syscall" LB_LF
  "retq" LB_LF
  
  "lbi_syscall_2_asm:" LB_LF
  "movq %rdx, %rax" LB_LF
  "syscall" LB_LF
  "retq" LB_LF
  
  "lbi_syscall_1_asm:" LB_LF
  "movq %rsi, %rax" LB_LF
  "syscall" LB_LF
  "retq" LB_LF
  
  "lbi_syscall_0_asm:" LB_LF
  "movq %rdi, %rax" LB_LF
  "syscall" LB_LF
  "retq" LB_LF
);

extern intptr_t lbi_syscall_6(uintptr_t a, uintptr_t b, uintptr_t c, uintptr_t n, uintptr_t e, uintptr_t* x) __asm__("lbi_syscall_6_asm");
extern intptr_t lbi_syscall_5(uintptr_t a, uintptr_t b, uintptr_t c, uintptr_t n, uintptr_t e, uintptr_t d)  __asm__("lbi_syscall_5_asm");
extern intptr_t lbi_syscall_3(uintptr_t a, uintptr_t b, uintptr_t c, uintptr_t n)                            __asm__("lbi_syscall_3_asm");

extern intptr_t lbi_syscall_2(uintptr_t a, uintptr_t b, uintptr_t n)                                         __asm__("lbi_syscall_2_asm");
extern intptr_t lbi_syscall_1(uintptr_t a, uintptr_t n)                                                      __asm__("lbi_syscall_1_asm");
extern intptr_t lbi_syscall_0(uintptr_t n)                                                                   __asm__("lbi_syscall_0_asm");

static intptr_t lb_syscall_6(uintptr_t n, uintptr_t a, uintptr_t b, uintptr_t c, uintptr_t d, uintptr_t e, uintptr_t f)
{
  uintptr_t x[] = { d, f };
  
  return lbi_syscall_6(a, b, c, n, e, x);
}

#define lb_syscall_5(n, a, b, c, d, e) lbi_syscall_5((a), (b), (c), (n), (e), (d))
#define lb_syscall_3(n, a, b, c)       lbi_syscall_3((a), (b), (c), (n))

#define lb_syscall_2(n, a, b)          lbi_syscall_2((a), (b), (n))
#define lb_syscall_1(n, a)             lbi_syscall_1((a), (n))
#define lb_syscall_0(n)                lbi_syscall_0((n))

#define lb_syscall_4(n, a, b, c, d) lb_syscall_5((n), (a), (b), (c), (d), (0))
