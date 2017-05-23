#!/usr/bin/env bash

ARG1="$1"

set -o xtrace
set -o errexit
set -o nounset
set -o pipefail
shopt -s failglob
shopt -s nullglob

if [ "$ARG1" == "ref" ]
then
  #cat ./libaboon/{definitions,syscall,abort,sysdeps,string,printer,start,context,linux,alloc,switch,io,watchdog,misc}.c | egrep '^static' | tee /tmp/ref
  #cat ./libaboon/everything.c | egrep '#include' | sed -e 's/^.\{12\}//' -e 's/.$//' | xargs cat -n | egrep 
  cat ./libaboon/everything.c | egrep '#include' | sed -e 's/^.\{12\}//' -e 's/.$//' | ( cd ./libaboon ; xargs grep -n "" ) | egrep '^[^:]*:[^:]*:static' | egrep -v 'lbi_' | tee /tmp/ref
  exit 0
fi

if [ "$ARG1" == "lint" ]
then
  cat ./libaboon/*.c | egrep '^[^ ]' | egrep -v '^(#|static |extern |struct |typedef )' | egrep -v '^(/\*|\*/|\{|\})$'
  exit 0
fi

[ -d ./build ] || sudo mkdir -m 0000 ./build
sudo mountpoint -q ./build || sudo mount -t tmpfs none ./build
cd ./build

function lb_linux()
{
  (
    set +o xtrace
    
    echo "#define _GNU_SOURCE"
    
    for i in inttypes.h stdio.h "${INCLUDES[@]}"
    do
      echo "#include <${i}>"
    done
    echo -n '
int main(void)
{
'
    for i in "${SYMBOLS[@]}"
    do
      echo "  printf(\"#define lbt_${i} %\" PRId64 \"\\n\", ((int64_t)(${i})));"
    done
    echo -n '
  return 0;
}
'
  ) >./tuxenv.c
  gcc -o ./tuxenv ./tuxenv.c
  ./tuxenv >./tuxenv.c
}

INCLUDES=(unistd.h sysexits.h sys/syscall.h sys/types.h sys/stat.h fcntl.h signal.h sys/epoll.h errno.h sys/{socket,un}.h netinet/in.h arpa/inet.h sys/timerfd.h sys/eventfd.h linux/aio_abi.h sys/ioctl.h net/if.h linux/if_packet.h net/ethernet.h net/route.h sys/mount.h)
SYMBOLS=(
  SYS_{exit_group,open,close,read,write,rt_sigprocmask,gettid,tgkill}
  SYS_{statfs,setns,ftruncate,time,sendfile,rt_sigpending,getuid,security,get_mempolicy,newfstatat,sched_get_priority_min,io_submit,reboot,getsockopt,request_key,modify_ldt,rt_sigreturn,inotify_rm_watch,sched_setaffinity,msgctl,sync,flock,tkill,munmap,query_module,clock_getres,mremap,inotify_init,removexattr,chroot,preadv,pwrite64,msgrcv,mq_timedreceive,setuid,readlink,setresgid,fanotify_init,pause,vmsplice,get_thread_area,semop,signalfd4,openat,shmat,open,getcpu,inotify_init1,fchdir,shmdt,setpgid,utimes,getcwd,process_vm_readv,epoll_pwait,lgetxattr,read,accept,kcmp,clock_adjtime,geteuid,flistxattr,link,getresgid,access,setitimer,getgid,mkdir,clock_nanosleep,fchown,execve,vfork,utime,splice,dup,poll,iopl,rt_sigaction,prctl,timer_delete,lookup_dcookie,fgetxattr,getitimer,epoll_create1,getpgid,timerfd_gettime,getpgrp,futimesat,linkat,recvfrom,mount,wait4,keyctl,mlockall,fstatfs,clone,getpriority,kexec_load,getegid,llistxattr,fsync,close,quotactl,getrusage,clock_settime,chmod,setgid,_sysctl,getsockname,set_robust_list,mbind,setfsuid,write,mlock2,setsockopt,lseek,afs_syscall,sched_setattr,sched_getaffinity,setxattr,add_key,writev,timerfd_settime,adjtimex,msgsnd,rt_tgsigqueueinfo,name_to_handle_at,epoll_wait,symlinkat,times,connect,rt_sigqueueinfo,mkdirat,sendto,fchownat,utimensat,recvmmsg,eventfd2,swapoff,set_mempolicy,set_tid_address,ioprio_get,clock_gettime,getsid,readahead,lstat,rename,io_cancel,get_kernel_syms,setreuid,unlink,fsetxattr,sched_getattr,setgroups,membarrier,pipe2,uselib,sched_setscheduler,sched_yield,tee,setfsgid,mlock,umask,epoll_ctl,rmdir,perf_event_open,acct,sysinfo,sched_setparam,inotify_add_watch,listen,mmap,timerfd_create,restart_syscall,ptrace,shmctl,setresuid,renameat2,signalfd,getgroups,nfsservctl,io_destroy,unshare,bind,getxattr,pwritev,dup2,dup3,pipe,timer_create,tuxcall,mincore,settimeofday,fallocate,shmget,fchmod,vhangup,socket,kexec_file_load,stat,pread64,faccessat,syncfs,fcntl,setsid,exit_group,sched_getscheduler,alarm,tgkill,sendmmsg,futex,bpf,capget,listxattr,getpid,fadvise64,brk,setrlimit,setregid,waitid,execveat,creat,userfaultfd,personality,semtimedop,epoll_wait_old,fork,ioctl,getpeername,ioperm,madvise,recvmsg,pivot_root,gettimeofday,socketpair,getppid,remap_file_pages,symlink,set_thread_area,getrlimit,swapon,fdatasync,rt_sigtimedwait,get_robust_list,gettid,open_by_handle_at,msgget,sched_getparam,migrate_pages,ioprio_set,memfd_create,exit,process_vm_writev,seccomp,sigaltstack,lchown,renameat,ppoll,nanosleep,semctl,timer_settime,kill,fanotify_mark,timer_getoverrun,ustat,finit_module,epoll_create,msync,readlinkat,semget,getdents,mq_getsetattr,readv,mq_unlink,sysfs,delete_module,mknodat,chdir,eventfd,rt_sigsuspend,unlinkat,vserver,create_module,setdomainname,uname,getrandom,epoll_ctl_old,prlimit64,io_setup,putpmsg,lremovexattr,fstat,sync_file_range,select,sethostname,truncate,syslog,timer_gettime,io_getevents,munlock,mq_notify,chown,init_module,munlockall,capset,move_pages,getpmsg,accept4,mprotect,mknod,lsetxattr,fremovexattr,sendmsg,shutdown,setpriority,rt_sigprocmask,sched_rr_get_interval,fchmodat,mq_open,getdents64,arch_prctl,pselect6,umount2,mq_timedsend,getresuid,sched_get_priority_max}
  O_{CREAT,TRUNC,RDONLY,WRONLY,RDWR,NONBLOCK}
  EX_SOFTWARE
  SIG{HUP,INT,QUIT,ILL,TRAP,ABRT,BUS,FPE,KILL,USR1,SEGV,USR2,PIPE,ALRM,TERM,STKFLT,CHLD,CONT,STOP,TSTP,TTIN,TTOU,URG,XCPU,XFSZ,VTALRM,PROF,WINCH,IO,PWR,SYS,RTMIN,RTMAX}
  SIG_{,UN}BLOCK
  EPOLL{IN,OUT}
  EPOLL_CTL_{ADD,DEL}
  F_{GET,SET}FL
  E{DEADLK,NAMETOOLONG,NOLCK,NOSYS,NOTEMPTY,LOOP,WOULDBLOCK,NOMSG,IDRM,CHRNG,L2NSYNC,L3HLT,L3RST,LNRNG,UNATCH,NOCSI,L2HLT,BADE,BADR,XFULL,NOANO,BADRQC,BADSLT,DEADLOCK,BFONT,NOSTR,NODATA,TIME,NOSR,NONET,NOPKG,REMOTE,NOLINK,ADV,SRMNT,COMM,PROTO,MULTIHOP,DOTDOT,BADMSG,OVERFLOW,NOTUNIQ,BADFD,REMCHG,LIBACC,LIBBAD,LIBSCN,LIBMAX,LIBEXEC,ILSEQ,RESTART,STRPIPE,USERS,NOTSOCK,DESTADDRREQ,MSGSIZE,PROTOTYPE,NOPROTOOPT,PROTONOSUPPORT,SOCKTNOSUPPORT,OPNOTSUPP,PFNOSUPPORT,AFNOSUPPORT,ADDRINUSE,ADDRNOTAVAIL,NETDOWN,NETUNREACH,NETRESET,CONNABORTED,CONNRESET,NOBUFS,ISCONN,NOTCONN,SHUTDOWN,TOOMANYREFS,TIMEDOUT,CONNREFUSED,HOSTDOWN,HOSTUNREACH,ALREADY,INPROGRESS,STALE,UCLEAN,NOTNAM,NAVAIL,ISNAM,REMOTEIO,DQUOT,NOMEDIUM,MEDIUMTYPE,CANCELED,NOKEY,KEYEXPIRED,KEYREVOKED,KEYREJECTED,OWNERDEAD,NOTRECOVERABLE,RFKILL,HWPOISON}
  E{AGAIN,EXIST}
  AF_{UNIX,INET,PACKET} PF_{INET,PACKET} IPPROTO_{RAW,IP} ETH_P_IP
  SOCK_{RAW,DGRAM,STREAM}
  SOL_SOCKET
  SO_REUSEADDR
  CLOCK_{REALTIME,MONOTONIC} TFD_{NONBLOCK,CLOEXEC} EFD_{NONBLOCK,CLOEXEC,SEMAPHORE}
  IOCB_{CMD_P{READ,WRITE},CMD_F{,D}SYNC,FLAG_RESFD}
  IFNAMSIZ
  SIOCGIF{INDEX,HWADDR}
  SIOC{G,S}IF{FLAGS,ADDR,NETMASK} IFF_{UP,BROADCAST,RUNNING,MULTICAST}
  SIOCADDRT RTF_{UP,GATEWAY}
  MS_SILENT
)

lb_linux

OPT="$(echo --std=c99 -fdiagnostics-color=always -W{error,all,extra,conversion,shadow,{strict,missing}-prototypes,c++-compat,missing-field-initializers,switch-default,inline} --all-warnings -Wno-unused-function)"
OPL="$(echo -no{startfiles,stdlib,defaultlibs})"
GCC="gcc $OPT"

FIL=test.c

cp ./../"$FIL" .
cp ./../local/cond.c .
ln -vsf -t . ./../local/libaboon

if [ "$ARG1" == "ppc" ]
then
  for i in "normal:" "parkour-record:-DLBT_PARKOUR -DLBT_PARKOUR_RECORD" "parkour-replay:-DLBT_PARKOUR -DLBT_PARKOUR_REPLAY"
  do
    NAME="${i/:*/}"
    OPTS="${i/*:/}"
    
    $GCC $OPTS -E -o ./minkey-gcc-E-"$NAME" ./"$FIL"
    
    (
      cat ./minkey-gcc-E-"$NAME" | egrep -v '^# '
    ) >./minkey-pp-"$NAME".c
    
    $GCC -o /dev/null -x c ./minkey-pp-"$NAME".c $OPL
  done
  
  exit
fi

if [ "$ARG1" == "asm" ]
then
  $GCC -S                                  -o ./asm-minkey.s           -x c ./"$FIL" $OPL
  
  exit
fi

$GCC -g                                    -o ./minkey                 -x c ./"$FIL" $OPL
$GCC -Os                                   -o ./minkey-basic-optimized -x c ./"$FIL" $OPL
$GCC -O2                                   -o ./minkey-speed-optimized -x c ./"$FIL" $OPL

#$GCC -g -DLBT_PARKOUR -DLBT_PARKOUR_RECORD -o ./minkey-parkour-record  -x c ./"$FIL" $OPL
#$GCC -g -DLBT_PARKOUR -DLBT_PARKOUR_REPLAY -o ./minkey-parkour-replay  -x c ./"$FIL" $OPL

if [ "$ARG1" == "spo" ]
then
  $GCC -DLB_SPACE_OPTIMIZED -Os  -fomit-frame-pointer           -o ./minkey-space-optimized -x c ./"$FIL" $OPL
  ls -l ./minkey-space-optimized
  strip                                       ./minkey-space-optimized
  ls -l ./minkey-space-optimized
  for i in .note.gnu.build-id .eh_frame{,_hdr} .comment
  do
    objcopy --remove-section "$i" ./minkey-space-optimized
    ls -l ./minkey-space-optimized
  done
  xz -9 -e <./minkey-space-optimized >./xz-minkey-space-optimized
  ls -l ./xz-minkey-space-optimized
fi

#( objdump -x ./minkey || true ) | ( egrep '^start' || true ) | egrep -o '0x.*$' | tee ./sa
