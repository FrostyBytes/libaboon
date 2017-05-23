/*
  copyright (c) 2017 by andrei borac
*/

#include "./tuxenv.c"

#ifdef __has_include
#if __has_include("./cond.c")
#include "./cond.c"
#endif
#endif

#include "./libaboon/everything.c"

#define TR LB_TR

#define MAIN_PARM uintptr_t argc LB_UNUSED, char const* const* argv LB_UNUSED, char const* const* envp LB_UNUSED
#define MAIN_PASS argc, argv, envp

#define MAIN_EN_001
#define MAIN_EN_002
#define MAIN_EN_003
#define MAIN_EN_004
#define MAIN_EN_005
#define MAIN_EN_006
#define MAIN_EN_007
#define MAIN_EN_008
#define MAIN_EN_009
#define MAIN_EN_010
#define MAIN_EN_011
#define MAIN_EN_012
#define MAIN_EN_013
#define MAIN_EN_014
//#define MAIN_EN_015
//#define MAIN_EN_016
//#define MAIN_EN_017
//#define MAIN_EN_018
//#define MAIN_EN_019

#ifdef MAIN_EN_001

static void main_001(MAIN_PARM)
{
  lb_syscall_3(1, 2, LB_U("hi\n"), 3);
  
  lb_print("su su su su su s", /**/ "hello ", 0, /**/ " ", 77, /**/ " ", 777, /**/ " ", 7777, /**/ " ", 2397429742, /**/ "\n");
  
  lb_print("sus", "argc=", argc, "\n");
  
  for (uintptr_t i = 0; i < argc; i++) {
    lb_print("susss", "argv[", i, "]='", argv[i], "'\n");
  }
  
  {
    char const* const* walk = envp;
    
    while (*walk) {
      lb_print("sss", "*envp='", *walk, "'\n");
      walk++;
    }
  }
}

#endif

#ifdef MAIN_EN_002

static void main_002(MAIN_PARM)
{
  lb_syscall_0(0x1000);
  lb_syscall_1(0x1000, 0x1001);
  lb_syscall_2(0x1000, 0x1001, 0x1002);
  lb_syscall_3(0x1000, 0x1001, 0x1002, 0x1003);
  lb_syscall_4(0x1000, 0x1001, 0x1002, 0x1003, 0x1004);
  lb_syscall_5(0x1000, 0x1001, 0x1002, 0x1003, 0x1004, 0x1005);
  lb_syscall_6(0x1000, 0x1001, 0x1002, 0x1003, 0x1004, 0x1005, 0x1006);
  
  /*
    expected strace output:
    
    syscall_4096(...) = -1 (errno 38)
    syscall_4096(0x1001, ...) = -1 (errno 38)
    syscall_4096(0x1001, 0x1002, ...) = -1 (errno 38)
    syscall_4096(0x1001, 0x1002, 0x1003, ...) = -1 (errno 38)
    syscall_4096(0x1001, 0x1002, 0x1003, 0x1004, ...) = -1 (errno 38)
    syscall_4096(0x1001, 0x1002, 0x1003, 0x1004, 0x1005, ...) = -1 (errno 38)
    syscall_4096(0x1001, 0x1002, 0x1003, 0x1004, 0x1005, 0x1006) = -1 (errno 38)
  */
}

#endif

#ifdef MAIN_EN_003

static void main_003_thread(lb_context_state_t* cs, void* parm)
{
  TR;
  LB_ASSURE((LB_U(parm) == 1));
  TR;
  lb_context_yield(cs, ((void*)(2)));
  TR;
}

LB_CONTEXT_PROC(main_003_thread)

static void main_003(MAIN_PARM)
{
  uintptr_t brk0 = LBT_OK(lb_syscall_1(lbt_SYS_brk, 0));
  void*     stack_base = ((void*)(brk0));
  uintptr_t stack_size = 65536;
  lb_syscall_1(lbt_SYS_brk, (brk0 + stack_size));
  
  lb_context_t cx;
  lb_context_initialize(&cx, stack_base, stack_size, main_003_thread_proc);
  TR;
  void* retv = lb_context_enter(&cx, ((void*)(1)));
  TR;
  LB_ASSURE((LB_U(retv) == 2));
}

#endif

#ifdef MAIN_EN_004

struct struct_m2_ll;
typedef struct struct_m2_ll m2_ll;

struct struct_m2_gs;
typedef struct struct_m2_gs m2_gs;

struct struct_m2_cs;
typedef struct struct_m2_cs m2_cs;

typedef unsigned char byte;

struct struct_m2_ll
{
  m2_ll* next;
  
  byte*     buf;
  uintptr_t len;
};

struct struct_m2_gs // global state
{
  lb_alloc_t* ac;
  m2_ll* chain;
  m2_cs* senders;
};

struct struct_m2_cs // connection state
{
  m2_gs* gs;
  uintptr_t fd;
  lb_thread_t* th;
  m2_cs* peer;
  m2_cs* next_sender; // set only if sender
};

#define CHAIN_FWD ({ chain = &((*chain)->next); })

#undef  AC
#define AC (cs->gs->ac)

LB_SWITCH_PUSH_CLEANUP_VARIANT_1(m2_cs_p, m2_cs*);

static void m2_handler_recv_cleanup(lb_thread_t* th, m2_cs* cs)
{
  LB_ASSURE(th == cs->th);
  
  if (cs->peer) {
    cs->peer->peer = NULL;
    lb_switch_destruct_thread(th, cs->peer->th);
  }
  
  lb_alloc_free(cs->gs->ac, cs, sizeof(*cs));
}

static void m2_handler_send_cleanup(lb_thread_t* th, m2_cs* cs)
{
  m2_cs** ptr = &(cs->gs->senders);
  while (true) {
    if (*ptr == cs) {
      *ptr = (*ptr)->next_sender;
      break;
    }
    ptr = &((*ptr)->next_sender);
  }
  
  m2_handler_recv_cleanup(th, cs);
}

static void m2_handler_recv(lb_thread_t* th, m2_cs* cs)
{
  TR;
  
  lb_switch_push_cleanup_m2_cs_p(th, m2_handler_recv_cleanup, cs);
  
  cs->th = th;
  
  m2_ll** chain = &(cs->gs->chain);
  
  lb_io_set_nonblocking(cs->fd);
  lb_io_push_cleanup_close_fd(th, cs->fd);
  
  while (true) {
    byte buf[4096];
    uintptr_t amt = lb_io_read(th, cs->fd, buf, sizeof(buf));
    LB_PV(amt);
    LB_PV(cs->gs->ac);
    byte* dup = (byte*)lb_alloc(AC, amt);
    lb_memcpy(dup, buf, amt);
    LB_ALLOC_DECL(m2_ll, ll, AC);
    ll->next = NULL;
    ll->buf = dup;
    ll->len = amt;
    TR;
    while (*chain != NULL) {
      TR;
      CHAIN_FWD;
      TR;
    }
    *chain = ll;
    //fprintf(stderr, "appended link with length=%lu\n", ll->len);
    {
      m2_cs* sender = cs->gs->senders;
      while (sender) {
        TR;
        //fprintf(stderr, "disturbing %lu\n", U(sender->th));
        lb_switch_disturb(th, sender->th);
        TR;
        sender = sender->next_sender;
      }
    }
    TR;
  }
}

static void m2_handler_send(lb_thread_t* th, m2_cs* cs)
{
  TR;
  
  lb_switch_push_cleanup_m2_cs_p(th, m2_handler_send_cleanup, cs);
  
  cs->th = th;
  
  cs->next_sender = cs->gs->senders;
  cs->gs->senders = cs;
  
  m2_ll** chain = &(cs->gs->chain);
  
  lb_io_set_nonblocking(cs->fd);
  lb_io_push_cleanup_close_fd(th, cs->fd);
  
  //fprintf(stderr, "new sender %lu\n", U(th));
  while (true) {
    //fprintf(stderr, "sender woke up %lu\n", U(th));
    
    m2_ll* last = NULL;
    while (*chain != NULL) {
      //fprintf(stderr, "*chain != NULL, *chain=%lu, len=%lu\n", U(*chain), ((*chain)->len));
      //fprintf(stderr, "writing (%lu) '%.*s'\n", (*chain)->len, (int)((*chain)->len), (*chain)->buf);
      //fprintf(stderr, "*chain != NULL, *chain=%lu, len=%lu\n", U(*chain), ((*chain)->len));
      last = *chain;
      //fprintf(stderr, "*chain != NULL, *chain=%lu, len=%lu\n", U(last), (last->len));
      lb_io_write_fully(th, cs->fd, (*chain)->buf, (*chain)->len);
      CHAIN_FWD;
    }
    if (last != NULL) {
      //fprintf(stderr, "*chain != NULL, *chain=%lu, len=%lu\n", U(last), (last->len));
    }
    //fprintf(stderr, "sender slumbering %lu\n", U(th));
    lb_switch_slumber(th);
  }
}

#undef AC
#define AC gs->ac

LB_SWITCH_CREATE_THREAD_VARIANT_1(m2_handler, m2_cs*);

static void m2_starter(lb_thread_t* th, void* parm LB_UNUSED)
{
  TR;
  uintptr_t fd_server = LBT_OK(lbt_socket(lbt_AF_UNIX, lbt_SOCK_STREAM, 0));
  TR;
  {
    lbt_sockaddr_un_t sun;
    sun.family = lbt_AF_UNIX;
    char const* hardpath = "/tmp/lb_test";
    lb_memcpy(sun.path, hardpath, (lb_strlen(hardpath)+1));
    lbt_unlink("/tmp/lb_test");
    LBT_OK(lbt_bind(fd_server, &sun, sizeof(sun)));
    LBT_OK(lbt_listen(fd_server, 1));
  }
  TR;
  lb_io_set_nonblocking(fd_server);
  TR;
  LB_ALLOC_DECL(m2_gs, gs, ((lb_alloc_t*)(parm)));
  gs->ac = ((lb_alloc_t*)(parm));
  //lb_breakpoint(LB_U(&(gs.ac)));
  //fprintf(stderr, "ac=%lu\n", U(gs.ac));
  gs->chain = NULL;
  gs->senders = NULL;
  TR;
  while (true) {
    uintptr_t fd_client = lb_io_accept(th, fd_server);
    TR;
    uintptr_t fd_recv = fd_client;
    uintptr_t fd_send = LBT_OK(lbt_dup(fd_client));
    TR;
    LB_ALLOC_DECL(m2_cs, cs_recv, AC);
    LB_ALLOC_DECL(m2_cs, cs_send, AC);
    
    cs_recv->gs = gs;
    cs_recv->fd = fd_recv;
    cs_recv->peer = cs_send;
    
    cs_send->gs = gs;
    cs_send->fd = fd_send;
    cs_send->peer = cs_recv;
    
    lb_switch_create_thread_m2_handler(th->sw, m2_handler_recv, cs_recv);
    lb_switch_create_thread_m2_handler(th->sw, m2_handler_send, cs_send);
  }
}

static void main_004(MAIN_PARM)
{
  lbt_block_signal_simple_SIGPIPE();
  
  lb_sbrk_t sb_;
  lb_sbrk_t* sb = (&(sb_));
  lb_sbrk_initialize(sb);
  lb_alloc_t ac_;
  lb_alloc_t* ac = (&(ac_));
  lb_alloc_initialize(ac, sb);
  
  //fprintf(stderr, "ac=%lu\n", U(ac));
  lb_alloc(ac, 5);
  lb_alloc(ac, 15);
  lb_alloc(ac, 55);
  lb_alloc(ac, 65);
  
  lb_switch_t sw_;
  lb_switch_t* sw = (&(sw_));
  TR;
  lb_switch_initialize(sw, ac);
  TR;
  lb_switch_create_thread(sw, m2_starter, ac);
  TR;
  lb_switch_event_loop(sw, NULL, NULL);
  TR;
  LB_ABORT;
}

#endif

#ifdef MAIN_EN_005

static void main_005(MAIN_PARM)
{
  /*
    test sysdep functions implemented with tricky string instructions.
  */
  
  {
#define PVAB(expr) { lbi_string_result_t sr = (expr); lb_print("ss", #expr, "\n"); LB_PV(sr.unread); LB_PV(sr.flags); }
    
    PVAB(lbi_memcmp(LB_U("ABCDEF-"), LB_U("abcdef~"), 6));
    PVAB(lbi_memcmp(LB_U("ABCDEF-"), LB_U("Abcdef~"), 6));
    PVAB(lbi_memcmp(LB_U("ABCDEF-"), LB_U("ABcdef~"), 6));
    PVAB(lbi_memcmp(LB_U("ABCDEF-"), LB_U("ABCdef~"), 6));
    PVAB(lbi_memcmp(LB_U("ABCDEF-"), LB_U("ABCDef~"), 6));
    PVAB(lbi_memcmp(LB_U("ABCDEF-"), LB_U("ABCDEf~"), 6));
    PVAB(lbi_memcmp(LB_U("ABCDEF-"), LB_U("ABCDEF~"), 6));
    
    LB_PV(lb_memcmp("ABCDEF-", "abcdef~", 6));
    LB_PV(lb_memcmp("ABCDEF-", "Abcdef~", 6));
    LB_PV(lb_memcmp("ABCDEF-", "ABcdef~", 6));
    LB_PV(lb_memcmp("ABCDEF-", "ABCdef~", 6));
    LB_PV(lb_memcmp("ABCDEF-", "ABCDef~", 6));
    LB_PV(lb_memcmp("ABCDEF-", "ABCDEf~", 6));
    LB_PV(lb_memcmp("ABCDEF-", "ABCDEF~", 6));
    
    LB_PV(lb_memcmp("A", "a", 0));
    LB_PV(lb_memcmp("A", "a", 1));
    LB_PV(lb_memcmp("A", "A", 1));
    
    LB_PV(lb_memcmp(NULL, NULL, 0));
    
    char const* abc = "ABCDEF-";
    LB_PV((char*)lb_memchr(abc, 'A', 0));
    LB_PV((char*)lb_memchr(abc, 'A', 1) - abc);
    LB_PV((char*)lb_memchr(abc, 'A', 2) - abc);
    LB_PV((char*)lb_memchr(abc, 'F', 0));
    LB_PV((char*)lb_memchr(abc, 'F', 5));
    LB_PV((char*)lb_memchr(abc, 'F', 6) - abc);
    LB_PV((char*)lb_memchr(abc, 'F', 7) - abc);
  }
  
  for (uintptr_t i = 0; i < 16; i++) {
    uintptr_t j = i;
    lb_print("s", "\n");
    LB_PV(j); LB_PV(lb_alloc_bin(&j)); LB_PV(j);
  }
  
  for (uintptr_t e = 4; e < 64; e++) {
    for (uintptr_t o = -1UL; o != 2UL; o++) {
      uintptr_t j = (1UL << e) + o;
      lb_print("s", "\n");
      LB_PV(j); LB_PV(lb_alloc_bin(&j)); LB_PV(j);
    }
  }
  
  // also test getenv
  lb_print("sss", "PATH='", lb_misc_getenv(envp, "PATH"), "'\n");
}

#endif

#ifdef MAIN_EN_006

struct struct_m6_ll;
typedef struct struct_m6_ll m6_ll;

struct struct_m6_gs;
typedef struct struct_m6_gs m6_gs;

struct struct_m6_cs;
typedef struct struct_m6_cs m6_cs;

typedef unsigned char byte;

struct struct_m6_ll
{
  m6_ll* next;
  
  byte*     buf;
  uintptr_t len;
};

struct struct_m6_gs // global state
{
  lb_alloc_t* ac;
  m6_ll* chain;
  m6_cs* senders;
};

struct struct_m6_cs // connection state
{
  m6_gs* gs;
  uintptr_t fd;
  lb_thread_t* th;
  m6_cs* peer;
  m6_cs* next_sender; // set only if sender
};

#define CHAIN_FWD ({ chain = &((*chain)->next); })

#undef  AC
#define AC (cs->gs->ac)

LB_SWITCH_PUSH_CLEANUP_VARIANT_1(m6_cs_p, m6_cs*);

static void m6_handler_recv_cleanup(lb_thread_t* th, m6_cs* cs)
{
  //LB_ASSURE(th == cs->th);
  
  if (cs->peer) {
    cs->peer->peer = NULL;
    lb_switch_destruct_thread(th, cs->peer->th);
  }
  
  lb_alloc_free(cs->gs->ac, cs, sizeof(*cs));
}

static void m6_handler_send_cleanup(lb_thread_t* th, m6_cs* cs)
{
  m6_cs** ptr = &(cs->gs->senders);
  while (true) {
    if (*ptr == cs) {
      *ptr = (*ptr)->next_sender;
      break;
    }
    ptr = &((*ptr)->next_sender);
  }
  
  m6_handler_recv_cleanup(th, cs);
}

#define M6_TIMEOUT 50
#define M6_TICKLE lb_watchdog_tickle(wd)

static void m6_handler_recv(lb_thread_t* th, m6_cs* cs)
{
  TR;
  
  lb_switch_push_cleanup_m6_cs_p(th, m6_handler_recv_cleanup, cs);
  lb_watchdog_t* wd = lb_watchdog_irreversible(th, AC);
  lb_watchdog_interval(wd, M6_TIMEOUT);
  
  //cs->th = th;
  
  m6_ll** chain = &(cs->gs->chain);
  
  lb_io_set_nonblocking(cs->fd);
  lb_io_push_cleanup_close_fd(th, cs->fd);
  
  while (true) {
    byte buf[4096];
    uintptr_t amt = lb_io_read(th, cs->fd, buf, sizeof(buf));
    M6_TICKLE;
    LB_PV(amt);
    LB_PV(cs->gs->ac);
    byte* dup = (byte*)lb_alloc(AC, amt);
    lb_memcpy(dup, buf, amt);
    LB_ALLOC_DECL(m6_ll, ll, AC);
    ll->next = NULL;
    ll->buf = dup;
    ll->len = amt;
    TR;
    while (*chain != NULL) {
      TR;
      CHAIN_FWD;
      TR;
    }
    *chain = ll;
    //fprintf(stderr, "appended link with length=%lu\n", ll->len);
    {
      m6_cs* sender = cs->gs->senders;
      while (sender) {
        TR;
        //fprintf(stderr, "disturbing %lu\n", U(sender->th));
        lb_switch_disturb(th, sender->th);
        TR;
        sender = sender->next_sender;
      }
    }
    TR;
  }
}

static void m6_handler_send(lb_thread_t* th, m6_cs* cs)
{
  TR;
  
  lb_switch_push_cleanup_m6_cs_p(th, m6_handler_send_cleanup, cs);
  lb_watchdog_t* wd = lb_watchdog_irreversible(th, AC);
  lb_watchdog_interval(wd, M6_TIMEOUT);
  
  //cs->th = th;
  
  cs->next_sender = cs->gs->senders;
  cs->gs->senders = cs;
  
  m6_ll** chain = &(cs->gs->chain);
  
  lb_io_set_nonblocking(cs->fd);
  lb_io_push_cleanup_close_fd(th, cs->fd);
  
  //fprintf(stderr, "new sender %lu\n", U(th));
  while (true) {
    //fprintf(stderr, "sender woke up %lu\n", U(th));
    
    m6_ll* last = NULL;
    while (*chain != NULL) {
      //fprintf(stderr, "*chain != NULL, *chain=%lu, len=%lu\n", U(*chain), ((*chain)->len));
      //fprintf(stderr, "writing (%lu) '%.*s'\n", (*chain)->len, (int)((*chain)->len), (*chain)->buf);
      //fprintf(stderr, "*chain != NULL, *chain=%lu, len=%lu\n", U(*chain), ((*chain)->len));
      last = *chain;
      //fprintf(stderr, "*chain != NULL, *chain=%lu, len=%lu\n", U(last), (last->len));
      M6_TICKLE;
      lb_io_write_fully(th, cs->fd, (*chain)->buf, (*chain)->len);
      lb_watchdog_disable(wd);
      CHAIN_FWD;
    }
    if (last != NULL) {
      //fprintf(stderr, "*chain != NULL, *chain=%lu, len=%lu\n", U(last), (last->len));
    }
    //fprintf(stderr, "sender slumbering %lu\n", U(th));
    lb_switch_slumber(th);
  }
}

#undef AC
#define AC gs->ac

LB_SWITCH_CREATE_THREAD_VARIANT_1(m6_handler, m6_cs*);

static void m6_starter(lb_thread_t* th, void* parm LB_UNUSED)
{
  TR;
  uintptr_t fd_server = LBT_OK(lbt_socket(lbt_AF_UNIX, lbt_SOCK_STREAM, 0));
  TR;
  {
    lbt_sockaddr_un_t sun;
    sun.family = lbt_AF_UNIX;
    char const* hardpath = "/tmp/lb_test";
    lb_memcpy(sun.path, hardpath, (lb_strlen(hardpath)+1));
    lbt_unlink("/tmp/lb_test");
    LBT_OK(lbt_bind(fd_server, &sun, sizeof(sun)));
    LBT_OK(lbt_listen(fd_server, 1));
  }
  TR;
  lb_io_set_nonblocking(fd_server);
  TR;
  LB_ALLOC_DECL(m6_gs, gs, ((lb_alloc_t*)(parm)));
  gs->ac = ((lb_alloc_t*)(parm));
  //lb_breakpoint(LB_U(&(gs.ac)));
  //fprintf(stderr, "ac=%lu\n", U(gs.ac));
  gs->chain = NULL;
  gs->senders = NULL;
  TR;
  while (true) {
    uintptr_t fd_client = lb_io_accept(th, fd_server);
    TR;
    uintptr_t fd_recv = fd_client;
    uintptr_t fd_send = LBT_OK(lbt_dup(fd_client));
    TR;
    LB_ALLOC_DECL(m6_cs, cs_recv, AC);
    LB_ALLOC_DECL(m6_cs, cs_send, AC);
    
    cs_recv->gs = gs;
    cs_recv->fd = fd_recv;
    cs_recv->peer = cs_send;
    
    cs_send->gs = gs;
    cs_send->fd = fd_send;
    cs_send->peer = cs_recv;
    
    cs_recv->th = lb_switch_create_thread_m6_handler(th->sw, m6_handler_recv, cs_recv);
    cs_send->th = lb_switch_create_thread_m6_handler(th->sw, m6_handler_send, cs_send);
  }
}

static void main_006(MAIN_PARM)
{
  lbt_block_signal_simple_SIGPIPE();
  
  lb_sbrk_t sb_;
  lb_sbrk_t* sb = (&(sb_));
  lb_sbrk_initialize(sb);
  lb_alloc_t ac_;
  lb_alloc_t* ac = (&(ac_));
  lb_alloc_initialize(ac, sb);
  
  //fprintf(stderr, "ac=%lu\n", U(ac));
  lb_alloc(ac, 5);
  lb_alloc(ac, 15);
  lb_alloc(ac, 55);
  lb_alloc(ac, 65);
  
  lb_switch_t sw_;
  lb_switch_t* sw = (&(sw_));
  TR;
  lb_switch_initialize(sw, ac);
  TR;
  lb_switch_create_thread(sw, m6_starter, ac);
  TR;
  lb_switch_event_loop(sw, NULL, NULL);
  TR;
  LB_ABORT;
}

#endif

#define MAXSCAN (65536-1)

/*
  WARNING: stops when EOF is encountered without yielding buffered
  lines
*/
static char* scan(void)
{
  static uintptr_t len = 0;
  static char      buf[MAXSCAN+1];
  static uintptr_t shm = 0;
  static bool      eof = false;
  
  /*
    shift out previous contents
  */
  lb_memcpy(buf, (buf + shm), (len - shm));
  len -= shm;
  
  /*
    refill buffer
  */
  while (((!eof) && (len < (sizeof(buf)-1)))) {
    intptr_t amt = lbt_read(0, (buf+len), ((sizeof(buf)-1)-len));
    if (amt == 0) { eof = true; break; }
    LB_ASSURE_GTZ(amt);
    len += LB_U(amt);
  }
  
  char* end = ((char*)(lb_memchr(buf, '\n', len)));
  //LB_PV(end);
  
  if (end) {
    *end = '\0';
    shm = (LB_PTRDIF(end, (&(buf[0]))) + 1);
    return buf;
  } else {
    if (eof && (len == 0)) {
      return NULL;
    } else {
      LB_ILLEGAL;
    }
  }
}

#ifdef MAIN_EN_007

static void main_007(MAIN_PARM)
{
  char* line;
  
  lb_sbrk_t sb_;
  lb_sbrk_t* sb = (&(sb_));
  lb_sbrk_initialize(sb);
  lb_alloc_t ac_;
  lb_alloc_t* ac = (&(ac_));
  lb_alloc_initialize(ac, sb);
  
  lb_stack_t* sk = lb_stack_create(ac);
  LB_TR;
  while ((line = scan())) {
    //lb_print("sss", "line='", line, "'\n");
    char line0 = line[0];
    line += 2;
    uintptr_t v = lb_atou_64(line);
    //LB_PV(v);
    
    /****/ if (line0 == '+') {
      //LB_PV(v);
      lb_stack_push(sk, ((void*)(v)));
    } else if (line0 == '-') {
      uintptr_t v_ = LB_U(lb_stack_pull(sk));
      //LB_PV(v_);
      LB_ASSURE(v_ == v);
    } else if (line0 == '=') {
      //LB_PV(lb_stack_size(sk));
      //LB_PV(v);
      LB_ASSURE(lb_stack_size(sk) == v);
    } else {
      LB_ILLEGAL;
    }
  }
}

#endif

#ifdef MAIN_EN_008

static void main_008(MAIN_PARM)
{
  char* line;
  
  lb_sbrk_t sb_;
  lb_sbrk_t* sb = (&(sb_));
  lb_sbrk_initialize(sb);
  lb_alloc_t ac_;
  lb_alloc_t* ac = (&(ac_));
  lb_alloc_initialize(ac, sb);
  
  lb_queue_t* qu = lb_queue_create(ac);
  LB_TR;
  while ((line = scan())) {
    //lb_print("sss", "line='", line, "'\n");
    char line0 = line[0];
    line += 2;
    uintptr_t v = lb_atou_64(line);
    //LB_PV(v);
    
    /****/ if (line0 == '+') {
      //LB_PV(v);
      lb_queue_push(qu, ((void*)(v)));
    } else if (line0 == '-') {
      uintptr_t v_ = LB_U(lb_queue_pull(qu));
      //LB_PV(v_);
      LB_ASSURE(v_ == v);
    } else if (line0 == '=') {
      //LB_PV(lb_stack_size(sk));
      //LB_PV(v);
      LB_ASSURE(lb_queue_size(qu) == v);
    } else {
      LB_ILLEGAL;
    }
  }
}

#endif

#ifdef MAIN_EN_009

static void main_009(MAIN_PARM)
{
  char* line;
  
  lb_sbrk_t sb_;
  lb_sbrk_t* sb = (&(sb_));
  lb_sbrk_initialize(sb);
  lb_alloc_t ac_;
  lb_alloc_t* ac = (&(ac_));
  lb_alloc_initialize(ac, sb);
  
  uintptr_t super_x = 0xDEADBEEFCAFEBABE;
  lb_breakpoint(LB_U(&(super_x)));
  LB_PV("'");
  LB_PV((&(super_x)));
  LB_PV(sizeof(super_x));
  LB_PV("'\n");
  lb_print("sms", "'", &(super_x), sizeof(super_x), "'\n");
  
  while ((line = scan())) {
    lb_print("sss", "line='", line, "'\n");
  }
}

#endif

#ifdef MAIN_EN_010

LB_MISC_QSORT_VARIANT(m10, char);
LB_MISC_BSEARCH_VARIANT(m10, char);

static bool cmp10(char const* A, char const* B)
{
  char const* oA = A;
  char const* oB = B;
  
  while (*A && *B) {
    /****/ if (*A < *B) {
      lb_print("ssss", oA, "<", oB, "\n");
      return true;
    } else if (*A > *B) {
      lb_print("ssss", oA, ">=", oB, "\n");
      return false;
    }
    
    A++;
    B++;
  }
  
  if (!*A && *B) {
    lb_print("ssss", oA, "<", oB, "\n");
    return true;
  }
  
  if (!*B && *A) {
    lb_print("ssss", oA, ">=", oB, "\n");
    return false;
  }
  
  lb_print("ssss", oA, ">==", oB, "\n");
  return false;
}

static void main_010(MAIN_PARM)
{
  char* line;
  
  lb_sbrk_t sb_;
  lb_sbrk_t* sb = (&(sb_));
  lb_sbrk_initialize(sb);
  lb_alloc_t ac_;
  lb_alloc_t* ac = (&(ac_));
  lb_alloc_initialize(ac, sb);
  
  LB_PV(cmp10("a", "b"));
  LB_PV(cmp10("b", "a"));
  LB_PV(cmp10("aa", "ab"));
  LB_PV(cmp10("ab", "aa"));
  LB_PV(cmp10("aa", "a"));
  
  lb_stack_t* sk = lb_stack_create(ac);
  
  while ((line = scan())) {
    lb_stack_push(sk, lb_misc_strdup(line, ac));
  }
  
  uintptr_t len = lb_stack_size(sk);
  char* arr[len];
  lb_stack_pull_many(sk, ((void**)(arr)), len);
  lb_misc_qsort_m10(arr, len, cmp10);
  
  for (uintptr_t i = 1; i < len; i++) {
    LB_ASSURE(!(cmp10(arr[i], arr[i-1])));
  }
  
  for (uintptr_t i = 0; i < len; i++) {
    lb_print("sss", "'", arr[i], "'\n");
  }
  
  lb_breakpoint(1);
  LB_AV(lb_memcmp("", "", 1), "memcmp test");
  
  /* now let's test bsearch */
  for (uintptr_t i = 0; i < len; i++) {
    char* found = lb_misc_bsearch_m10(arr, len, arr[i], cmp10);
    //lb_print("sssusssus", "while searching for '", arr[i], "'[", lb_strlen(arr[i]), "] found '", found, "'[", lb_strlen(found), "]\n");
    //LB_PV(lb_memcmp(found, arr[i], (len+1)));
    uintptr_t found_len = lb_strlen(found);
    uintptr_t arr_i_len = lb_strlen(arr[i]);
    LB_ASSURE(found_len == arr_i_len);
    LB_ASSURE(lb_memcmp(found, arr[i], found_len));
    char* arr_i_app = lb_misc_strdup_append(arr[i], "@", ac);
    LB_ASSURE(!(lb_misc_bsearch_m10(arr, len, arr_i_app, cmp10)));
    lb_misc_strdup_free(arr_i_app, ac);
  }
  
}

#endif

#ifdef MAIN_EN_011

static void main_011(MAIN_PARM)
{
  lb_sbrk_t sb_;
  lb_sbrk_t* sb = (&(sb_));
  lb_sbrk_initialize(sb);
  lb_alloc_t ac_;
  lb_alloc_t* ac = (&(ac_));
  lb_alloc_initialize(ac, sb);
  
  uintptr_t fd = LBT_OK(lbt_open("/tmp/dutdutdut", lbt_O_RDWR, 0000));
  LB_PV(fd);
}

#endif

#ifdef MAIN_EN_012

static void main_012(MAIN_PARM)
{
  char* line;
  
  lb_sbrk_t sb_;
  lb_sbrk_t* sb = (&(sb_));
  lb_sbrk_initialize(sb);
  lb_alloc_t ac_;
  lb_alloc_t* ac = (&(ac_));
  lb_alloc_initialize(ac, sb);
  
  while ((line = scan())) {
    lb_print("sssus", "line='", line, "' u64='", lb_atou_64(line), "'\n");
  }
}

#endif

#ifdef MAIN_EN_013

static void main_013(MAIN_PARM)
{
  char* line;
  
  lb_sbrk_t sb_;
  lb_sbrk_t* sb = (&(sb_));
  lb_sbrk_initialize(sb);
  lb_alloc_t ac_;
  lb_alloc_t* ac = (&(ac_));
  lb_alloc_initialize(ac, sb);
  
  while ((line = scan())) {
    lb_print("sss", "line='", line, "'\n");
    uintptr_t ll = lb_strlen(line);
    LB_ASSURE_EQZ(ll % 16);
    lb_bignum_t* bn = lb_bignum_create(ll/4, ac);
    lb_bignum_load(bn, line);
    lb_bignum_print(bn);
    char buf[65536];
    uintptr_t len = lb_bignum_save(bn, buf);
    buf[len] = '\0';
    lb_print("sss", "buf='", buf, "'\n");
  }
}

#endif

#ifdef MAIN_EN_014

static void m14_r2c(lb_bignum_t* r, lb_bignum_t* c)
{
  lb_bignum_mov_trunc(c, r);
}

static void main_014(MAIN_PARM)
{
  char* line;
  
  lb_sbrk_t sb_;
  lb_sbrk_t* sb = (&(sb_));
  lb_sbrk_initialize(sb);
  lb_alloc_t ac_;
  lb_alloc_t* ac = (&(ac_));
  lb_alloc_initialize(ac, sb);
  
  uintptr_t num = 0;
  
  while ((line = scan())) {
    /****/ if (lb_strcmp(line, "id")) {
      /*
        identity test - check that bignums save and load correctly.
      */
      
      line = scan();
      uintptr_t bignum_bits = lb_atou_64(line);
      
      lb_bignum_t* bn = lb_bignum_create(bignum_bits, ac);
      
      line = scan();
      lb_bignum_load(bn, line);
      
      char buf[65536];
      uintptr_t len = lb_bignum_save(bn, buf);
      buf[len] = '\0';
      
      LB_ASSURE(lb_strcmp(buf, line));
      
      lb_bignum_delete(bn, ac);
      
      num++;
    } else if (lb_strcmp(line, "ag")) {
      /*
        agreement test.
      */
      
      line = scan();
      uintptr_t x = lb_atou_64(line);
      line = scan();
      lb_bignum_t* bn = lb_bignum_create(64, ac);
      lb_bignum_load(bn, line);
      LB_ASSURE(bn->len == 1);
      LB_ASSURE(bn->arr[0] == x);
      
      num++;
    } else if (lb_strcmp(line, "cmp")) {
      /*
        comparison test
      */
      
      line = scan();
      uintptr_t bignum_bits = lb_atou_64(line);
      
      line = scan();
      lb_bignum_t* a = lb_bignum_create(bignum_bits, ac);
      lb_bignum_load(a, line);
      
      line = scan();
      lb_bignum_t* b = lb_bignum_create(bignum_bits, ac);
      lb_bignum_load(b, line);
      
      line = scan();
      switch (lb_bignum_compare_signum(a, b)) {
      case -1: LB_ASSURE(lb_strcmp(line, "<")); break;
      case 00: LB_ASSURE(lb_strcmp(line, "=")); break;
      case +1: LB_ASSURE(lb_strcmp(line, ">")); break;
      default: LB_ILLEGAL;
      }
      
#if 0
      TR; lb_bignum_print(a);
      TR; lb_bignum_print(b);
      TR; lb_bignum_print(c);
      TR; lb_bignum_print(d);
#endif
      
      num++;
    } else if (lb_strcmp(line, "add")) {
      /*
        addition test
      */
      
      line = scan();
      uintptr_t bignum_bits = lb_atou_64(line);
      
      line = scan();
      lb_bignum_t* a = lb_bignum_create(bignum_bits, ac);
      lb_bignum_load(a, line);
      
      line = scan();
      lb_bignum_t* b = lb_bignum_create(bignum_bits, ac);
      lb_bignum_load(b, line);
      
      line = scan();
      lb_bignum_t* c = lb_bignum_create(bignum_bits, ac);
      lb_bignum_load(c, line);
      
      lb_bignum_t* d = lb_bignum_create(bignum_bits, ac);
      lb_bignum_mov(d, a);
      lb_bignum_add(d, b);
      
#if 0
      TR; lb_bignum_print(a);
      TR; lb_bignum_print(b);
      TR; lb_bignum_print(c);
      TR; lb_bignum_print(d);
#endif
      
      LB_ASSURE(lb_bignum_compare_eq(d, c));
      
      num++;
    } else if (lb_strcmp(line, "sub")) {
      /*
        subtraction test
      */
      
      line = scan();
      uintptr_t bignum_bits = lb_atou_64(line);
      
      line = scan();
      lb_bignum_t* a = lb_bignum_create(bignum_bits, ac);
      lb_bignum_load(a, line);
      
      line = scan();
      lb_bignum_t* b = lb_bignum_create(bignum_bits, ac);
      lb_bignum_load(b, line);
      
      line = scan();
      lb_bignum_t* c = lb_bignum_create(bignum_bits, ac);
      lb_bignum_load(c, line);
      
      lb_bignum_t* d = lb_bignum_create(bignum_bits, ac);
      lb_bignum_mov(d, a);
      lb_bignum_sub(d, b);
      
#if 0
      TR; lb_bignum_print(a);
      TR; lb_bignum_print(b);
      TR; lb_bignum_print(c);
      TR; lb_bignum_print(d);
#endif
      
      LB_ASSURE(lb_bignum_compare_eq(d, c));
      
      num++;
    } else if (lb_strcmp(line, "shl")) {
      /*
        left-shift test
      */
      
      line = scan();
      uintptr_t bignum_bits = lb_atou_64(line);
      
      line = scan();
      lb_bignum_t* a = lb_bignum_create(bignum_bits, ac);
      lb_bignum_load(a, line);
      
      line = scan();
      uintptr_t b = lb_atou_64(line);
      
      line = scan();
      lb_bignum_t* ab = lb_bignum_create(bignum_bits, ac);
      lb_bignum_load(ab, line);
      
      lb_bignum_t* t = lb_bignum_create(bignum_bits, ac);
      lb_bignum_mov(t, a);
      lb_bignum_shl(t, b);
      
#if 0
      TR; lb_bignum_print(a);
      LB_PV(b);
      TR; lb_bignum_print(t);
      TR; lb_bignum_print(ab);
#endif
      
      LB_ASSURE(lb_bignum_compare_eq(t, ab));
      
      num++;
    } else if (lb_strcmp(line, "shr")) {
      /*
        right-shift test
      */
      
      line = scan();
      uintptr_t bignum_bits = lb_atou_64(line);
      
      line = scan();
      lb_bignum_t* a = lb_bignum_create(bignum_bits, ac);
      lb_bignum_load(a, line);
      
      line = scan();
      uintptr_t b = lb_atou_64(line);
      
      line = scan();
      lb_bignum_t* ab = lb_bignum_create(bignum_bits, ac);
      lb_bignum_load(ab, line);
      
      lb_bignum_t* t = lb_bignum_create(bignum_bits, ac);
      lb_bignum_mov(t, a);
      lb_bignum_shr(t, b);
      
#if 0
      TR; lb_bignum_print(a);
      LB_PV(b);
      TR; lb_bignum_print(t);
      TR; lb_bignum_print(ab);
#endif
      
      LB_ASSURE(lb_bignum_compare_eq(t, ab));
      
      num++;
    } else if (lb_strcmp(line, "mul")) {
      /*
        multiplication test
      */
      
      line = scan();
      uintptr_t bignum_bits_small = lb_atou_64(line);
      
      line = scan();
      uintptr_t bignum_bits_large = lb_atou_64(line);
      
      line = scan();
      lb_bignum_t* a = lb_bignum_create(bignum_bits_small, ac);
      lb_bignum_load(a, line);
      
      line = scan();
      lb_bignum_t* b = lb_bignum_create(bignum_bits_small, ac);
      lb_bignum_load(b, line);
      
      line = scan();
      lb_bignum_t* c = lb_bignum_create(bignum_bits_large, ac);
      lb_bignum_load(c, line);
      
      lb_bignum_t* d = lb_bignum_create(bignum_bits_large, ac);
      lb_bignum_t* t = lb_bignum_create(bignum_bits_large, ac);
      lb_bignum_mul(d, a, b, t);
      
#if 0
      TR; lb_bignum_print(a);
      TR; lb_bignum_print(b);
      TR; lb_bignum_print(c);
      TR; lb_bignum_print(d);
      TR; lb_bignum_print(t);
#endif
      
      LB_ASSURE(lb_bignum_compare_eq(d, c));
      
      num++;
    } else if (lb_strcmp(line, "bar")) {
      /*
        barrett reduction test
      */
      
      line = scan();
      uintptr_t k = lb_atou_64(line);
      
      line = scan();
      lb_bignum_t* x = lb_bignum_create((k+k), ac);
      lb_bignum_load(x, line);
      
      line = scan();
      lb_bignum_t* m = lb_bignum_create(k, ac);
      lb_bignum_load(m, line);
      
      line = scan();
      lb_bignum_t* u = lb_bignum_create((k+1), ac);
      lb_bignum_load(u, line);
      
      line = scan();
      lb_bignum_t* r_expected = lb_bignum_create(k, ac);
      lb_bignum_load(r_expected, line);
      
      lb_bignum_t* r = lb_bignum_create(k, ac);
      lb_bignum_barrett(k, x, m, u, r, ac);
      
#if 0
      LB_PV(k);
      TR; lb_bignum_print(x);
      TR; lb_bignum_print(m);
      TR; lb_bignum_print(u);
      TR; lb_bignum_print(r_expected);
      TR; lb_bignum_print(r);
#endif
      
      LB_ASSURE(lb_bignum_compare_eq(r, r_expected));
      
      num++;
    } else if (lb_strcmp(line, "exp")) {
      /*
        modular exponentiation test
      */
      
      line = scan();
      uintptr_t k = lb_atou_64(line);
      
      line = scan();
      lb_bignum_t* b = lb_bignum_create(k, ac);
      lb_bignum_load(b, line);
      
      line = scan();
      lb_bignum_t* e = lb_bignum_create(k, ac);
      lb_bignum_load(e, line);
      
      line = scan();
      lb_bignum_t* m = lb_bignum_create(k, ac);
      lb_bignum_load(m, line);
      
      line = scan();
      lb_bignum_t* u = lb_bignum_create((k+1), ac);
      lb_bignum_load(u, line);
      
      line = scan();
      lb_bignum_t* r_expected = lb_bignum_create(k, ac);
      lb_bignum_load(r_expected, line);
      
      /**/
      
      lb_bignum_t* r = lb_bignum_create(k, ac);
      lb_bignum_modexp(k, b, e, m, u, r, ac);
      
#if 0
      LB_PV(k);
      TR; lb_bignum_print(x);
      TR; lb_bignum_print(m);
      TR; lb_bignum_print(u);
      TR; lb_bignum_print(r_expected);
      TR; lb_bignum_print(r);
#endif
      
      LB_ASSURE(lb_bignum_compare_eq(r, r_expected));
      
      num++;
    } else if (lb_strcmp(line, "shn")) {
      /*
        shnorr verification test. not a real test, just a time trial.
      */
      
      line = scan();
      uintptr_t kq = lb_atou_64(line);
      
      line = scan();
      uintptr_t kp = lb_atou_64(line);
      
      line = scan();
      uintptr_t g = lb_atou_64(line);
      
      line = scan();
      lb_bignum_t* p = lb_bignum_create(kp, ac);
      TR; lb_bignum_load(p, line); TR;
      
      line = scan();
      lb_bignum_t* u = lb_bignum_create((kp+1), ac);
      TR; lb_bignum_load(u, line); TR;
      
      line = scan();
      lb_bignum_t* y = lb_bignum_create(kp, ac);
      TR; lb_bignum_load(y, line); TR;
      
      line = scan();
      lb_bignum_t* c = lb_bignum_create(kq, ac);
      TR; lb_bignum_load(c, line); TR;
      
      line = scan();
      lb_bignum_t* s = lb_bignum_create(kq, ac);
      TR; lb_bignum_load(s, line); TR;
      
      /**/
      
      bool vfy = lb_bignum_schnorr_verify(kq, kp, g, p, u, y, c, s, m14_r2c, ac);
      
      lb_print("sss", "vfy=", vfy ? "true" : "false", "\n");
      
#if 0
      LB_PV(k);
      TR; lb_bignum_print(x);
      TR; lb_bignum_print(m);
      TR; lb_bignum_print(u);
      TR; lb_bignum_print(r_expected);
      TR; lb_bignum_print(r);
#endif
      
      num++;
    } else {
      LB_ILLEGAL;
    }
  }
  
  lb_print("sus", "completed ", num, " tests\n");
}

#endif

LB_MAIN_SPEC
{
  lb_print("s", "here we go!\n");
  
  LB_ASSURE(argc >= 2);
  
  switch (lb_atou_64(argv[1])) {
#ifdef MAIN_EN_001
  case  1:
    main_001(MAIN_PASS);
    break;
#endif
#ifdef MAIN_EN_002
  case  2:
    main_002(MAIN_PASS);
    break;
#endif
#ifdef MAIN_EN_003
  case  3:
    main_003(MAIN_PASS);
    break;
#endif
#ifdef MAIN_EN_004
  case  4:
    main_004(MAIN_PASS);
    break;
#endif
#ifdef MAIN_EN_005
  case  5:
    main_005(MAIN_PASS);
    break;
#endif
#ifdef MAIN_EN_006
  case  6:
    main_006(MAIN_PASS);
    break;
#endif
#ifdef MAIN_EN_007
  case  7:
    main_007(MAIN_PASS);
    break;
#endif
#ifdef MAIN_EN_008
  case  8:
    main_008(MAIN_PASS);
    break;
#endif
#ifdef MAIN_EN_009
  case  9:
    main_009(MAIN_PASS);
    break;
#endif
#ifdef MAIN_EN_010
  case 10:
    main_010(MAIN_PASS);
    break;
#endif
#ifdef MAIN_EN_011
  case 11:
    main_011(MAIN_PASS);
    break;
#endif
#ifdef MAIN_EN_012
  case 12:
    main_012(MAIN_PASS);
    break;
#endif
#ifdef MAIN_EN_013
  case 13:
    main_013(MAIN_PASS);
    break;
#endif
#ifdef MAIN_EN_014
  case 14:
    main_014(MAIN_PASS);
    break;
#endif
#ifdef MAIN_EN_015
  case 15:
    main_015(MAIN_PASS);
    break;
#endif
#ifdef MAIN_EN_016
  case 16:
    main_016(MAIN_PASS);
    break;
#endif
#ifdef MAIN_EN_017
  case 17:
    main_017(MAIN_PASS);
    break;
#endif
#ifdef MAIN_EN_018
  case 18:
    main_018(MAIN_PASS);
    break;
#endif
#ifdef MAIN_EN_019
  case 19:
    main_019(MAIN_PASS);
    break;
#endif
    
  default:
    LB_ILLEGAL;
  }
  
  lb_print("s", "clean exit!\n");
  
  lbt_exit_simple(0);
}
