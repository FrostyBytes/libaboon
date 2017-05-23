/*
  libaboon/start.c
  copyright (c) 2017 by andrei borac
*/

__asm__
(
 ".globl _start"              LB_LF
 "_start:"                    LB_LF
 "mov %rsp, %rdi"             LB_LF
 "callq lbi_start"            LB_LF
 "libi_start_return_loop:"    LB_LF
 "jmp libi_start_return_loop" LB_LF
);

#define LB_MAIN_SPEC                                                    \
  static void lb_main(uintptr_t argc LB_UNUSED, char const* const* argv LB_UNUSED, char const* const* envp LB_UNUSED)

LB_MAIN_SPEC;

void lbi_start(void*);

void lbi_start(void* rsp LB_UNUSED)
{
  lb_read_stack_pointer_check();
  
  if ((
       (sizeof(int8_t)    != 1) ||
       (sizeof(int16_t)   != 2) ||
       (sizeof(int32_t)   != 4) ||
       (sizeof(int64_t)   != 8) ||
       (sizeof(intptr_t)  != 8) ||
       (sizeof(uint8_t)   != 1) ||
       (sizeof(uint16_t)  != 2) ||
       (sizeof(uint32_t)  != 4) ||
       (sizeof(uint64_t)  != 8) ||
       (sizeof(uintptr_t) != 8)
       )) {
    LB_ILLEGAL;
  }
  
  uintptr_t* sp = ((uintptr_t*)(rsp));
  
  uintptr_t argc = *(sp++);
  char const* const* argv = ((char const *const *)(sp));
  sp += argc + 1;
  char const* const* envp = ((char const *const *)(sp));
  
  lb_main(argc, argv, envp);
  
  LB_ILLEGAL;
}
