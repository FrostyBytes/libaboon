/*
  libaboon/sysdeps.c
  copyright (c) 2017 by andrei borac
*/

__asm__
(
  "lbi_get_stack_pointer_asm:" LB_LF
  "movq %rsp, %rax" LB_LF
  "retq" LB_LF
);

/*
  a "correct" reading (for ABI-mandated 16-byte alignment) is 8%16
*/
extern uintptr_t lb_read_stack_pointer(void) __asm__("lbi_get_stack_pointer_asm");

static void lb_read_stack_pointer_check(void)
{
  if (!((lb_read_stack_pointer() & (16 - 1)) == 8)) {
    LB_ABORT;
  }
}

static LB_INLINE uintptr_t lb_bsr(uintptr_t valu)
{
  uintptr_t retv;
  
  __asm__
  (
    "bsrq %[valu], %[retv]"
    : [retv] "=r" (retv)
    : [valu]  "r" (valu)
  );
  
  return retv;
}

__asm__
(
  // rdi = value
  "lbi_bswap_asm:" LB_LF
  "movq %rdi, %rax" LB_LF
  "bswapq %rax" LB_LF
  "retq" LB_LF
);

extern uintptr_t lb_bswap(uintptr_t val) __asm__("lbi_bswap_asm");

static uint64_t lb_bswap_64(uint64_t val)
{
  return ((uint64_t)(lb_bswap((LB_U(val) << (64-64)))));
}

static uint32_t lb_bswap_32(uint32_t val)
{
  return ((uint32_t)(lb_bswap((LB_U(val) << (64-32)))));
}

static uint16_t lb_bswap_16(uint16_t val)
{
  return ((uint16_t)(lb_bswap((LB_U(val) << (64-16)))));
}

#define LB_BSWAP_AUTO(x)                                                \
  {                                                                     \
    /****/ if (sizeof((x)) == 2) {                                      \
      (x) = ((__typeof__(x))(lb_bswap_16(((uint16_t)(LB_U(x))))));      \
    } else if (sizeof((x)) == 4) {                                      \
      (x) = ((__typeof__(x))(lb_bswap_32(((uint32_t)(LB_U(x))))));      \
    } else if (sizeof((x)) == 8) {                                      \
      (x) = ((__typeof__(x))(lb_bswap_64(((uint64_t)(LB_U(x))))));      \
    } else {                                                            \
      LB_ILLEGAL;                                                       \
    }                                                                   \
  }

__asm__
(
  // rdi = dst
  // rsi = src
  // rdx = len
  "lbi_bswap_32_a_asm:" LB_LF
  "movl 0(%rsi), %eax" LB_LF
  "bswapl %eax" LB_LF
  "movl %eax, 0(%rdi)" LB_LF
  "addq $4, %rdi" LB_LF
  "addq $4, %rsi" LB_LF
  "dec %rdx" LB_LF
  "jnz lbi_bswap_32_a_asm" LB_LF
  "retq" LB_LF
);

extern void lbi_bswap_32_a(uint32_t* dst, uint32_t* src, uintptr_t len) __asm__("lbi_bswap_32_a_asm");

static void lb_bswap_32_a(uint32_t* dst, uint32_t* src, uintptr_t len)
{
  if (len > 0) {
    return lbi_bswap_32_a(dst, src, len);
  }
}

typedef struct { uintptr_t flags; uintptr_t unread; } lbi_string_result_t;

/*
  rig-up for executing a string instruction.
  
  rdi = dst, rsi = src, rdx = len ;; rax = flags, rdx = unread
  
  WARNING: do not ever run with len=0.
*/
#define LB_ASM_STR(label, strop)                                        \
__asm__                                                                 \
(                                                                       \
  #label "_asm" ":" LB_LF                                               \
  "movq %rdx, %rcx" LB_LF                                               \
  "movq %rsi, %rax" LB_LF                                               \
  strop LB_LF                                                           \
  "pushfq" LB_LF                                                        \
  "popq %rax" LB_LF                                                     \
  "mov  %rcx, %rdx" LB_LF                                               \
  "retq" LB_LF                                                          \
);                                                                      \
                                                                        \
extern lbi_string_result_t label(uintptr_t dst, uintptr_t src, uintptr_t len) __asm__(#label "_asm");

LB_ASM_STR(lbi_memcpy, "cld" LB_LF "rep movsb");

static void lb_memcpy(void* dst, void const* src, uintptr_t len)
{
  if (len > 0) {
    lbi_memcpy(LB_U(dst), LB_U(src), len);
  }
}

LB_ASM_STR(lbi_memset, "cld" LB_LF "rep stosb");

static void lb_memset(void* dst, uintptr_t val, uintptr_t len)
{
  if (len > 0) {
    lbi_memset(LB_U(dst), val, len);
  }
}

static void lb_bzero(void* dst, uintptr_t len)
{
  lb_memset(dst, 0, len);
}

#define LB_BZERO(x) (lb_bzero((&(x)), sizeof((x))))

#define LBI_ISSET_ZF (sr.flags & (1 << 6))

LB_ASM_STR(lbi_memcmp, "cld" LB_LF "repz cmpsb");

static bool lb_memcmp(void const* src1, void const* src2, uintptr_t len)
{
  if (len > 0) {
    lbi_string_result_t sr = lbi_memcmp(LB_U(src1), LB_U(src2), len);
    
    return ((sr.unread == 0) && (LBI_ISSET_ZF));
  } else {
    return true;
  }
}

LB_ASM_STR(lbi_memchr, "cld" LB_LF "repnz scasb");

static void* lb_memchr(void const* buf, uintptr_t val, uintptr_t len)
{
  if (len > 0) {
    lbi_string_result_t sr = lbi_memchr(LB_U(buf), val, len);
    
    if ((sr.unread > 0) || (LBI_ISSET_ZF)) {
      return ((void*)(LB_U(buf) + (len - sr.unread - 1)));
    } else {
      return NULL;
    }
  } else {
    /*
      can't locate any character in a zero-length buffer.
    */
    return NULL;
  }
}

static void* lb_memmem(void const* buf, uintptr_t buf_len, void const* ndl, uintptr_t ndl_len)
{
  void const* lim = (buf + buf_len);
  
  if (ndl_len == 0) {
    return ((void*)(buf));
  } else {
    void* fst;
    
    while (((buf < lim) && ((fst = lb_memchr(buf, (*((uint8_t*)(ndl))), LB_PTRDIF(lim, buf))) != NULL))) {
      uintptr_t rem = LB_PTRDIF(lim, fst);
      
      if (rem < ndl_len) {
        return NULL;
      }
      
      if (lb_memcmp(fst, ndl, ndl_len)) {
        return ((void*)(fst));
      }
      
      buf = (fst + 1);
    }
    
    return NULL;
  }
}
