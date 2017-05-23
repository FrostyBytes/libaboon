/*
  libaboon/printer.c
  copyright (c) 2017 by andrei borac
*/

#ifndef LB_STDIO_STDERR
#define LB_STDIO_STDERR 2
#endif

#define LBI_PRINT_S__TA(t, v) t v = ((t)((*(++arg))));

static uintptr_t lbi_print_s(uintptr_t a LB_UNUSED, uintptr_t b LB_UNUSED, uintptr_t c LB_UNUSED, uintptr_t d LB_UNUSED, uintptr_t e LB_UNUSED, char* buf, char const* fmt, ...)
{
  char* buf_saved = buf;
  
  void const** arg = ((void const**)(&fmt));
  
  while (*fmt) {
    switch (*fmt) {
    case ' ':
      {
        break;
      }
      
    case 's':
      {
        LBI_PRINT_S__TA(char const*, str);
        
        if (str != NULL) {
          uintptr_t len = lb_strlen(str);
          
          if (buf_saved) {
            lb_memcpy(buf, str, len);
          }
          
          buf += len;
        } else {
          if (buf_saved) {
            lb_memcpy(buf, "(null)", 6);
          }
          
          buf += 6;
        }
        
        break;
      }
      
    case 'm':
      {
        LBI_PRINT_S__TA(uint8_t const*, mem);
        LBI_PRINT_S__TA(uintptr_t, len);
        
        lb_breakpoint(len);
        
        if (buf_saved) {
          static char const nibble_to_char[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };
          
          for (uintptr_t i = 0; i < len; i++) {
            (*(buf++)) = nibble_to_char[(mem[i] >> 4) & 0x0F];
            (*(buf++)) = nibble_to_char[(mem[i]     ) & 0x0F];
          }
        } else {
          buf += (len << 1);
        }
        
        break;
      }
      
    case 'u':
      {
        LBI_PRINT_S__TA(uintptr_t, val);
        
        if (buf_saved) {
          buf += lb_utoa_64(buf, val);
        } else {
          buf += lb_utoa_64_sz(val);
        }
        
        break;
      }
      
    default:
      {
        LB_ILLEGAL;
      }
    }
    
    fmt++;
  }
  
  if (buf_saved) {
    *buf = '\0';
  }
  
  return LB_PTRDIF(buf, buf_saved);
}

#define lb_print_s(buf, fmt, ...)                                       \
  ({                                                                    \
    uintptr_t lb_print_s__1;                                            \
    uintptr_t lb_print_s__2;                                            \
    uintptr_t lb_print_s__3;                                            \
    uintptr_t lb_print_s__4;                                            \
    uintptr_t lb_print_s__5;                                            \
    __asm__("" : "=r" (lb_print_s__1), "=r" (lb_print_s__2), "=r" (lb_print_s__3), "=r" (lb_print_s__4), "=r" (lb_print_s__5)); \
    lbi_print_s(lb_print_s__1, lb_print_s__2, lb_print_s__3, lb_print_s__4, lb_print_s__5, (buf), (fmt), ##__VA_ARGS__); \
  })

static void lbi_print_fd(uintptr_t fd, char const* buf, uintptr_t len)
{
  while (len > 0) {
    intptr_t amt = ((intptr_t)(lb_syscall_3(lbt_SYS_write, fd, LB_U(buf), len)));
    
    if (amt <= 0) {
#ifdef LB_PRINTER_IGNORE_ERRORS
      break;
#else
      LB_ABORT;
#endif
    }
    
    uintptr_t uamt = LB_U(amt);
    
    buf += uamt;
    len -= uamt;
  }
}

#define lb_print_fd(fd, fmt, ...)                                       \
  ({                                                                    \
    char const* LB_PRINT_FD__FMT = (fmt);                               \
    uintptr_t   LB_PRINT_FD__LEN = lb_print_s(NULL, LB_PRINT_FD__FMT, ##__VA_ARGS__); \
    char        LB_PRINT_FD__BUF[LB_PRINT_FD__LEN+1];                   \
    lb_print_s(LB_PRINT_FD__BUF, LB_PRINT_FD__FMT, ##__VA_ARGS__);      \
    lbi_print_fd((fd), LB_PRINT_FD__BUF, LB_PRINT_FD__LEN);             \
    ((void)(0));                                                        \
  })

#define lb_print(fmt, ...)                                              \
  { lb_print_fd(LB_STDIO_STDERR, (fmt), ##__VA_ARGS__); }

#if 0

/* the old way to implement assure */

LBI_ASSURE_FAILED_SPEC
{
  lb_print("sssssss", "assure_failed: (", file, ")[", line, "]: expected (", expr, ")\n");
  LB_ABORT;
}

#endif

#ifndef LB_TRACE_COND
#define LB_TRACE_COND false
#endif

#ifndef LB_BREAK_COND
#define LB_BREAK_COND false
#endif

#define LB_TR                                                           \
  {                                                                     \
    if (LB_BREAK_COND) {                                                \
      lb_breakpoint(-1U);                                               \
    }                                                                   \
                                                                        \
    if (LB_TRACE_COND) {                                                \
      lb_print("sssss", "R (", __FILE__, ")[", LBI_S__LINE__, "]\n");   \
    }                                                                   \
  }

#define LB_PV(v)                                                        \
  ({                                                                    \
    __typeof__(v) LB_PV__A = (v);                                       \
                                                                        \
    if (LB_BREAK_COND) {                                                \
      lb_breakpoint(LB_U(LB_PV__A));                                    \
    }                                                                   \
                                                                        \
    if (LB_TRACE_COND) {                                                \
      lb_print("sssusssus", "(", __FILE__, ")[", __LINE__, "]: ", #v, "=", LB_U(LB_PV__A), "\n"); \
    }                                                                   \
                                                                        \
    LB_PV__A;                                                           \
  })

#define LB_AV(v, a)                                                     \
  ({                                                                    \
    __typeof__(v) LB_AV__A = (v);                                       \
                                                                        \
    if (LB_BREAK_COND) {                                                \
      lb_breakpoint(LB_U(LB_AV__A));                                    \
    }                                                                   \
                                                                        \
    if (LB_TRACE_COND) {                                                \
      lb_print("sssusssusss", "(", __FILE__, ")[", __LINE__, "]: ", #v, "=", LB_U(LB_AV__A), " <--- ", a, "\n"); \
    }                                                                   \
                                                                        \
    LB_AV__A;                                                           \
  })

#define LB_DG(fmt, ...)                                                 \
  {                                                                     \
    if (LB_BREAK_COND) {                                                \
      lb_breakpoint(-1U);                                               \
    }                                                                   \
                                                                        \
    if (LB_TRACE_COND) {                                                \
      lb_print((fmt), ##__VA_ARGS__);                                   \
    }                                                                   \
  }

#define LB_AN(str, val)                                                 \
  ({ ((void)(str)); (val); })
