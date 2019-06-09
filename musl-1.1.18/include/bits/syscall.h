#define __NR_restart_syscall	0
#define __NR_exit	1
#define __NR_fork	2
#define __NR_read	3
#define __NR_write	4
#define __NR_open	5
#define __NR_close	6
#define __NR_creat	8
#define __NR_link	9
#define __NR_unlink	10
#define __NR_execve	11
#define __NR_chdir	12
#define __NR_mknod	14
#define __NR_chmod	15
#define __NR_lchown	16
#define __NR_lseek	19
#define __NR_getpid	20
#define __NR_mount	21
#define __NR_setuid	23
#define __NR_getuid	24
#define __NR_ptrace	26
#define __NR_pause	29
#define __NR_access	33
#define __NR_nice	34
#define __NR_sync	36
#define __NR_kill	37
#define __NR_rename	38
#define __NR_mkdir	39
#define __NR_rmdir	40
#define __NR_dup	41
#define __NR_pipe	42
#define __NR_times	43
#define __NR_brk	45
#define __NR_setgid	46
#define __NR_getgid	47
#define __NR_geteuid	49
#define __NR_getegid	50
#define __NR_acct	51
#define __NR_umount2	52
#define __NR_ioctl	54
#define __NR_fcntl	55
#define __NR_setpgid	57
#define __NR_umask	60
#define __NR_chroot	61
#define __NR_ustat	62
#define __NR_dup2	63
#define __NR_getppid	64
#define __NR_getpgrp	65
#define __NR_setsid	66
#define __NR_sigaction	67
#define __NR_setreuid	70
#define __NR_setregid	71
#define __NR_sigsuspend	72
#define __NR_sigpending	73
#define __NR_sethostname	74
#define __NR_setrlimit	75
#define __NR_getrusage	77
#define __NR_gettimeofday	78
#define __NR_settimeofday	79
#define __NR_getgroups	80
#define __NR_setgroups	81
#define __NR_symlink	83
#define __NR_readlink	85
#define __NR_uselib	86
#define __NR_swapon	87
#define __NR_reboot	88
#define __NR_munmap	91
#define __NR_truncate	92
#define __NR_ftruncate	93
#define __NR_fchmod	94
#define __NR_fchown	95
#define __NR_getpriority	96
#define __NR_setpriority	97
#define __NR_statfs	99
#define __NR_fstatfs	100
#define __NR_syslog	103
#define __NR_setitimer	104
#define __NR_getitimer	105
#define __NR_stat	106
#define __NR_lstat	107
#define __NR_fstat	108
#define __NR_vhangup	111
#define __NR_wait4	114
#define __NR_swapoff	115
#define __NR_sysinfo	116
#define __NR_fsync	118
#define __NR_sigreturn	119
#define __NR_clone	120
#define __NR_setdomainname	121
#define __NR_uname	122
#define __NR_adjtimex	124
#define __NR_mprotect	125
#define __NR_sigprocmask	126
#define __NR_init_module	128
#define __NR_delete_module	129
#define __NR_quotactl	131
#define __NR_getpgid	132
#define __NR_fchdir	133
#define __NR_bdflush	134
#define __NR_sysfs	135
#define __NR_personality	136
#define __NR_setfsuid	138
#define __NR_setfsgid	139
#define __NR__llseek	140
#define __NR_getdents	141
#define __NR__newselect	142
#define __NR_flock	143
#define __NR_msync	144
#define __NR_readv	145
#define __NR_writev	146
#define __NR_getsid	147
#define __NR_fdatasync	148
#define __NR__sysctl	149
#define __NR_mlock	150
#define __NR_munlock	151
#define __NR_mlockall	152
#define __NR_munlockall	153
#define __NR_sched_setparam	154
#define __NR_sched_getparam	155
#define __NR_sched_setscheduler	156
#define __NR_sched_getscheduler	157
#define __NR_sched_yield	158
#define __NR_sched_get_priority_max	159
#define __NR_sched_get_priority_min	160
#define __NR_sched_rr_get_interval	161
#define __NR_nanosleep	162
#define __NR_mremap	163
#define __NR_setresuid	164
#define __NR_getresuid	165
#define __NR_poll	168
#define __NR_nfsservctl	169
#define __NR_setresgid	170
#define __NR_getresgid	171
#define __NR_prctl	172
#define __NR_rt_sigreturn	173
#define __NR_rt_sigaction	174
#define __NR_rt_sigprocmask	175
#define __NR_rt_sigpending	176
#define __NR_rt_sigtimedwait	177
#define __NR_rt_sigqueueinfo	178
#define __NR_rt_sigsuspend	179
#define __NR_pread64	180
#define __NR_pwrite64	181
#define __NR_chown	182
#define __NR_getcwd	183
#define __NR_capget	184
#define __NR_capset	185
#define __NR_sigaltstack	186
#define __NR_sendfile	187
#define __NR_vfork	190
#define __NR_ugetrlimit	191
#define __NR_mmap2	192
#define __NR_truncate64	193
#define __NR_ftruncate64	194
#define __NR_stat64	195
#define __NR_lstat64	196
#define __NR_fstat64	197
#define __NR_lchown32	198
#define __NR_getuid32	199
#define __NR_getgid32	200
#define __NR_geteuid32	201
#define __NR_getegid32	202
#define __NR_setreuid32	203
#define __NR_setregid32	204
#define __NR_getgroups32	205
#define __NR_setgroups32	206
#define __NR_fchown32	207
#define __NR_setresuid32	208
#define __NR_getresuid32	209
#define __NR_setresgid32	210
#define __NR_getresgid32	211
#define __NR_chown32	212
#define __NR_setuid32	213
#define __NR_setgid32	214
#define __NR_setfsuid32	215
#define __NR_setfsgid32	216
#define __NR_getdents64	217
#define __NR_pivot_root	218
#define __NR_mincore	219
#define __NR_madvise	220
#define __NR_fcntl64	221
#define __NR_gettid	224
#define __NR_readahead	225
#define __NR_setxattr	226
#define __NR_lsetxattr	227
#define __NR_fsetxattr	228
#define __NR_getxattr	229
#define __NR_lgetxattr	230
#define __NR_fgetxattr	231
#define __NR_listxattr	232
#define __NR_llistxattr	233
#define __NR_flistxattr	234
#define __NR_removexattr	235
#define __NR_lremovexattr	236
#define __NR_fremovexattr	237
#define __NR_tkill	238
#define __NR_sendfile64	239
#define __NR_futex	240
#define __NR_sched_setaffinity	241
#define __NR_sched_getaffinity	242
#define __NR_io_setup	243
#define __NR_io_destroy	244
#define __NR_io_getevents	245
#define __NR_io_submit	246
#define __NR_io_cancel	247
#define __NR_exit_group	248
#define __NR_lookup_dcookie	249
#define __NR_epoll_create	250
#define __NR_epoll_ctl	251
#define __NR_epoll_wait	252
#define __NR_remap_file_pages	253
#define __NR_set_tid_address	256
#define __NR_timer_create	257
#define __NR_timer_settime	258
#define __NR_timer_gettime	259
#define __NR_timer_getoverrun	260
#define __NR_timer_delete	261
#define __NR_clock_settime	262
#define __NR_clock_gettime	263
#define __NR_clock_getres	264
#define __NR_clock_nanosleep	265
#define __NR_statfs64	266
#define __NR_fstatfs64	267
#define __NR_tgkill	268
#define __NR_utimes	269
#define __NR_fadvise64_64	270
#define __NR_arm_fadvise64_64	270
#define __NR_pciconfig_iobase	271
#define __NR_pciconfig_read	272
#define __NR_pciconfig_write	273
#define __NR_mq_open	274
#define __NR_mq_unlink	275
#define __NR_mq_timedsend	276
#define __NR_mq_timedreceive	277
#define __NR_mq_notify	278
#define __NR_mq_getsetattr	279
#define __NR_waitid	280
#define __NR_socket	281
#define __NR_bind	282
#define __NR_connect	283
#define __NR_listen	284
#define __NR_accept	285
#define __NR_getsockname	286
#define __NR_getpeername	287
#define __NR_socketpair	288
#define __NR_send	289
#define __NR_sendto	290
#define __NR_recv	291
#define __NR_recvfrom	292
#define __NR_shutdown	293
#define __NR_setsockopt	294
#define __NR_getsockopt	295
#define __NR_sendmsg	296
#define __NR_recvmsg	297
#define __NR_semop	298
#define __NR_semget	299
#define __NR_semctl	300
#define __NR_msgsnd	301
#define __NR_msgrcv	302
#define __NR_msgget	303
#define __NR_msgctl	304
#define __NR_shmat	305
#define __NR_shmdt	306
#define __NR_shmget	307
#define __NR_shmctl	308
#define __NR_add_key	309
#define __NR_request_key	310
#define __NR_keyctl	311
#define __NR_semtimedop	312
#define __NR_vserver	313
#define __NR_ioprio_set	314
#define __NR_ioprio_get	315
#define __NR_inotify_init	316
#define __NR_inotify_add_watch	317
#define __NR_inotify_rm_watch	318
#define __NR_mbind	319
#define __NR_get_mempolicy	320
#define __NR_set_mempolicy	321
#define __NR_openat	322
#define __NR_mkdirat	323
#define __NR_mknodat	324
#define __NR_fchownat	325
#define __NR_futimesat	326
#define __NR_fstatat64	327
#define __NR_unlinkat	328
#define __NR_renameat	329
#define __NR_linkat	330
#define __NR_symlinkat	331
#define __NR_readlinkat	332
#define __NR_fchmodat	333
#define __NR_faccessat	334
#define __NR_pselect6	335
#define __NR_ppoll	336
#define __NR_unshare	337
#define __NR_set_robust_list	338
#define __NR_get_robust_list	339
#define __NR_splice	340
#define __NR_sync_file_range2	341
#define __NR_arm_sync_file_range	341
#define __NR_tee	342
#define __NR_vmsplice	343
#define __NR_move_pages	344
#define __NR_getcpu	345
#define __NR_epoll_pwait	346
#define __NR_kexec_load	347
#define __NR_utimensat	348
#define __NR_signalfd	349
#define __NR_timerfd_create	350
#define __NR_eventfd	351
#define __NR_fallocate	352
#define __NR_timerfd_settime	353
#define __NR_timerfd_gettime	354
#define __NR_signalfd4	355
#define __NR_eventfd2	356
#define __NR_epoll_create1	357
#define __NR_dup3	358
#define __NR_pipe2	359
#define __NR_inotify_init1	360
#define __NR_preadv	361
#define __NR_pwritev	362
#define __NR_rt_tgsigqueueinfo	363
#define __NR_perf_event_open	364
#define __NR_recvmmsg	365
#define __NR_accept4	366
#define __NR_fanotify_init	367
#define __NR_fanotify_mark	368
#define __NR_prlimit64	369
#define __NR_name_to_handle_at	370
#define __NR_open_by_handle_at	371
#define __NR_clock_adjtime	372
#define __NR_syncfs	373
#define __NR_sendmmsg	374
#define __NR_setns	375
#define __NR_process_vm_readv	376
#define __NR_process_vm_writev	377
#define __NR_kcmp		378
#define __NR_finit_module	379
#define __NR_sched_setattr	380
#define __NR_sched_getattr	381
#define __NR_renameat2	382
#define __NR_seccomp	383
#define __NR_getrandom	384
#define __NR_memfd_create	385
#define __NR_bpf	386
#define __NR_execveat	387
#define __NR_userfaultfd	388
#define __NR_membarrier		389
#define __NR_mlock2		390
#define __NR_copy_file_range	391
#define __NR_preadv2	392
#define __NR_pwritev2	393
#define __NR_pkey_mprotect	394
#define __NR_pkey_alloc	395
#define __NR_pkey_free	396

#define __ARM_NR_breakpoint	0x0f0001
#define __ARM_NR_cacheflush	0x0f0002
#define __ARM_NR_usr26		0x0f0003
#define __ARM_NR_usr32		0x0f0004
#define __ARM_NR_set_tls	0x0f0005

#define SYS_restart_syscall SYS_restart_syscall
long SYS_restart_syscall();
#define SYS_exit SYS_exit
long SYS_exit();
#define SYS_fork SYS_fork
long SYS_fork();
#define SYS_read SYS_read
long SYS_read();
#define SYS_write SYS_write
long SYS_write();
#define SYS_open SYS_open
long SYS_open();
#define SYS_close SYS_close
long SYS_close();
#define SYS_creat SYS_creat
long SYS_creat();
#define SYS_link SYS_link
long SYS_link();
#define SYS_unlink SYS_unlink
long SYS_unlink();
#define SYS_execve SYS_execve
long SYS_execve();
#define SYS_chdir SYS_chdir
long SYS_chdir();
#define SYS_mknod SYS_mknod
long SYS_mknod();
#define SYS_chmod SYS_chmod
long SYS_chmod();
#define SYS_lchown SYS_lchown
long SYS_lchown();
#define SYS_lseek SYS_lseek
long SYS_lseek();
#define SYS_getpid SYS_getpid
long SYS_getpid();
#define SYS_mount SYS_mount
long SYS_mount();
#define SYS_setuid SYS_setuid
long SYS_setuid();
#define SYS_getuid SYS_getuid
long SYS_getuid();
#define SYS_ptrace SYS_ptrace
long SYS_ptrace();
#define SYS_pause SYS_pause
long SYS_pause();
#define SYS_access SYS_access
long SYS_access();
#define SYS_nice SYS_nice
long SYS_nice();
#define SYS_sync SYS_sync
long SYS_sync();
#define SYS_kill SYS_kill
long SYS_kill();
#define SYS_rename SYS_rename
long SYS_rename();
#define SYS_mkdir SYS_mkdir
long SYS_mkdir();
#define SYS_rmdir SYS_rmdir
long SYS_rmdir();
#define SYS_dup SYS_dup
long SYS_dup();
#define SYS_pipe SYS_pipe
long SYS_pipe();
#define SYS_times SYS_times
long SYS_times();
#define SYS_brk SYS_brk
long SYS_brk();
#define SYS_setgid SYS_setgid
long SYS_setgid();
#define SYS_getgid SYS_getgid
long SYS_getgid();
#define SYS_geteuid SYS_geteuid
long SYS_geteuid();
#define SYS_getegid SYS_getegid
long SYS_getegid();
#define SYS_acct SYS_acct
long SYS_acct();
#define SYS_umount2 SYS_umount2
long SYS_umount2();
#define SYS_ioctl SYS_ioctl
long SYS_ioctl();
#define SYS_fcntl SYS_fcntl
long SYS_fcntl();
#define SYS_setpgid SYS_setpgid
long SYS_setpgid();
#define SYS_umask SYS_umask
long SYS_umask();
#define SYS_chroot SYS_chroot
long SYS_chroot();
#define SYS_ustat SYS_ustat
long SYS_ustat();
#define SYS_dup2 SYS_dup2
long SYS_dup2();
#define SYS_getppid SYS_getppid
long SYS_getppid();
#define SYS_getpgrp SYS_getpgrp
long SYS_getpgrp();
#define SYS_setsid SYS_setsid
long SYS_setsid();
#define SYS_sigaction SYS_sigaction
long SYS_sigaction();
#define SYS_setreuid SYS_setreuid
long SYS_setreuid();
#define SYS_setregid SYS_setregid
long SYS_setregid();
#define SYS_sigsuspend SYS_sigsuspend
long SYS_sigsuspend();
#define SYS_sigpending SYS_sigpending
long SYS_sigpending();
#define SYS_sethostname SYS_sethostname
long SYS_sethostname();
#define SYS_setrlimit SYS_setrlimit
long SYS_setrlimit();
#define SYS_getrusage SYS_getrusage
long SYS_getrusage();
#define SYS_gettimeofday SYS_gettimeofday
long SYS_gettimeofday();
#define SYS_settimeofday SYS_settimeofday
long SYS_settimeofday();
#define SYS_getgroups SYS_getgroups
long SYS_getgroups();
#define SYS_setgroups SYS_setgroups
long SYS_setgroups();
#define SYS_symlink SYS_symlink
long SYS_symlink();
#define SYS_readlink SYS_readlink
long SYS_readlink();
#define SYS_uselib SYS_uselib
long SYS_uselib();
#define SYS_swapon SYS_swapon
long SYS_swapon();
#define SYS_reboot SYS_reboot
long SYS_reboot();
#define SYS_munmap SYS_munmap
long SYS_munmap();
#define SYS_truncate SYS_truncate
long SYS_truncate();
#define SYS_ftruncate SYS_ftruncate
long SYS_ftruncate();
#define SYS_fchmod SYS_fchmod
long SYS_fchmod();
#define SYS_fchown SYS_fchown
long SYS_fchown();
#define SYS_getpriority SYS_getpriority
long SYS_getpriority();
#define SYS_setpriority SYS_setpriority
long SYS_setpriority();
#define SYS_statfs SYS_statfs
long SYS_statfs();
#define SYS_fstatfs SYS_fstatfs
long SYS_fstatfs();
#define SYS_syslog SYS_syslog
long SYS_syslog();
#define SYS_setitimer SYS_setitimer
long SYS_setitimer();
#define SYS_getitimer SYS_getitimer
long SYS_getitimer();
#define SYS_stat SYS_stat
long SYS_stat();
#define SYS_lstat SYS_lstat
long SYS_lstat();
#define SYS_fstat SYS_fstat
long SYS_fstat();
#define SYS_vhangup SYS_vhangup
long SYS_vhangup();
#define SYS_wait4 SYS_wait4
long SYS_wait4();
#define SYS_swapoff SYS_swapoff
long SYS_swapoff();
#define SYS_sysinfo SYS_sysinfo
long SYS_sysinfo();
#define SYS_fsync SYS_fsync
long SYS_fsync();
#define SYS_sigreturn SYS_sigreturn
long SYS_sigreturn();
#define SYS_clone SYS_clone
long SYS_clone();
#define SYS_setdomainname SYS_setdomainname
long SYS_setdomainname();
#define SYS_uname SYS_uname
long SYS_uname();
#define SYS_adjtimex SYS_adjtimex
long SYS_adjtimex();
#define SYS_mprotect SYS_mprotect
long SYS_mprotect();
#define SYS_sigprocmask SYS_sigprocmask
long SYS_sigprocmask();
#define SYS_init_module SYS_init_module
long SYS_init_module();
#define SYS_delete_module SYS_delete_module
long SYS_delete_module();
#define SYS_quotactl SYS_quotactl
long SYS_quotactl();
#define SYS_getpgid SYS_getpgid
long SYS_getpgid();
#define SYS_fchdir SYS_fchdir
long SYS_fchdir();
#define SYS_bdflush SYS_bdflush
long SYS_bdflush();
#define SYS_sysfs SYS_sysfs
long SYS_sysfs();
#define SYS_personality SYS_personality
long SYS_personality();
#define SYS_setfsuid SYS_setfsuid
long SYS_setfsuid();
#define SYS_setfsgid SYS_setfsgid
long SYS_setfsgid();
#define SYS__llseek SYS__llseek
long SYS__llseek();
#define SYS_getdents SYS_getdents
long SYS_getdents();
#define SYS__newselect SYS__newselect
long SYS__newselect();
#define SYS_flock SYS_flock
long SYS_flock();
#define SYS_msync SYS_msync
long SYS_msync();
#define SYS_readv SYS_readv
long SYS_readv();
#define SYS_writev SYS_writev
long SYS_writev();
#define SYS_getsid SYS_getsid
long SYS_getsid();
#define SYS_fdatasync SYS_fdatasync
long SYS_fdatasync();
#define SYS__sysctl SYS__sysctl
long SYS__sysctl();
#define SYS_mlock SYS_mlock
long SYS_mlock();
#define SYS_munlock SYS_munlock
long SYS_munlock();
#define SYS_mlockall SYS_mlockall
long SYS_mlockall();
#define SYS_munlockall SYS_munlockall
long SYS_munlockall();
#define SYS_sched_setparam SYS_sched_setparam
long SYS_sched_setparam();
#define SYS_sched_getparam SYS_sched_getparam
long SYS_sched_getparam();
#define SYS_sched_setscheduler SYS_sched_setscheduler
long SYS_sched_setscheduler();
#define SYS_sched_getscheduler SYS_sched_getscheduler
long SYS_sched_getscheduler();
#define SYS_sched_yield SYS_sched_yield
long SYS_sched_yield();
#define SYS_sched_get_priority_max SYS_sched_get_priority_max
long SYS_sched_get_priority_max();
#define SYS_sched_get_priority_min SYS_sched_get_priority_min
long SYS_sched_get_priority_min();
#define SYS_sched_rr_get_interval SYS_sched_rr_get_interval
long SYS_sched_rr_get_interval();
#define SYS_nanosleep SYS_nanosleep
long SYS_nanosleep();
#define SYS_mremap SYS_mremap
long SYS_mremap();
#define SYS_setresuid SYS_setresuid
long SYS_setresuid();
#define SYS_getresuid SYS_getresuid
long SYS_getresuid();
#define SYS_poll SYS_poll
long SYS_poll();
#define SYS_nfsservctl SYS_nfsservctl
long SYS_nfsservctl();
#define SYS_setresgid SYS_setresgid
long SYS_setresgid();
#define SYS_getresgid SYS_getresgid
long SYS_getresgid();
#define SYS_prctl SYS_prctl
long SYS_prctl();
#define SYS_rt_sigreturn SYS_rt_sigreturn
long SYS_rt_sigreturn();
#define SYS_rt_sigaction SYS_rt_sigaction
long SYS_rt_sigaction();
#define SYS_rt_sigprocmask SYS_rt_sigprocmask
long SYS_rt_sigprocmask();
#define SYS_rt_sigpending SYS_rt_sigpending
long SYS_rt_sigpending();
#define SYS_rt_sigtimedwait SYS_rt_sigtimedwait
long SYS_rt_sigtimedwait();
#define SYS_rt_sigqueueinfo SYS_rt_sigqueueinfo
long SYS_rt_sigqueueinfo();
#define SYS_rt_sigsuspend SYS_rt_sigsuspend
long SYS_rt_sigsuspend();
#define SYS_pread64 SYS_pread64
long SYS_pread64();
#define SYS_pwrite64 SYS_pwrite64
long SYS_pwrite64();
#define SYS_chown SYS_chown
long SYS_chown();
#define SYS_getcwd SYS_getcwd
long SYS_getcwd();
#define SYS_capget SYS_capget
long SYS_capget();
#define SYS_capset SYS_capset
long SYS_capset();
#define SYS_sigaltstack SYS_sigaltstack
long SYS_sigaltstack();
#define SYS_sendfile SYS_sendfile
long SYS_sendfile();
#define SYS_vfork SYS_vfork
long SYS_vfork();
#define SYS_ugetrlimit SYS_ugetrlimit
long SYS_ugetrlimit();
#define SYS_mmap2 SYS_mmap2
long SYS_mmap2();
#define SYS_truncate64 SYS_truncate64
long SYS_truncate64();
#define SYS_ftruncate64 SYS_ftruncate64
long SYS_ftruncate64();
#define SYS_stat64 SYS_stat64
long SYS_stat64();
#define SYS_lstat64 SYS_lstat64
long SYS_lstat64();
#define SYS_fstat64 SYS_fstat64
long SYS_fstat64();
#define SYS_lchown32 SYS_lchown32
long SYS_lchown32();
#define SYS_getuid32 SYS_getuid32
long SYS_getuid32();
#define SYS_getgid32 SYS_getgid32
long SYS_getgid32();
#define SYS_geteuid32 SYS_geteuid32
long SYS_geteuid32();
#define SYS_getegid32 SYS_getegid32
long SYS_getegid32();
#define SYS_setreuid32 SYS_setreuid32
long SYS_setreuid32();
#define SYS_setregid32 SYS_setregid32
long SYS_setregid32();
#define SYS_getgroups32 SYS_getgroups32
long SYS_getgroups32();
#define SYS_setgroups32 SYS_setgroups32
long SYS_setgroups32();
#define SYS_fchown32 SYS_fchown32
long SYS_fchown32();
#define SYS_setresuid32 SYS_setresuid32
long SYS_setresuid32();
#define SYS_getresuid32 SYS_getresuid32
long SYS_getresuid32();
#define SYS_setresgid32 SYS_setresgid32
long SYS_setresgid32();
#define SYS_getresgid32 SYS_getresgid32
long SYS_getresgid32();
#define SYS_chown32 SYS_chown32
long SYS_chown32();
#define SYS_setuid32 SYS_setuid32
long SYS_setuid32();
#define SYS_setgid32 SYS_setgid32
long SYS_setgid32();
#define SYS_setfsuid32 SYS_setfsuid32
long SYS_setfsuid32();
#define SYS_setfsgid32 SYS_setfsgid32
long SYS_setfsgid32();
#define SYS_getdents64 SYS_getdents64
long SYS_getdents64();
#define SYS_pivot_root SYS_pivot_root
long SYS_pivot_root();
#define SYS_mincore SYS_mincore
long SYS_mincore();
#define SYS_madvise SYS_madvise
long SYS_madvise();
#define SYS_fcntl64 SYS_fcntl64
long SYS_fcntl64();
#define SYS_gettid SYS_gettid
long SYS_gettid();
#define SYS_readahead SYS_readahead
long SYS_readahead();
#define SYS_setxattr SYS_setxattr
long SYS_setxattr();
#define SYS_lsetxattr SYS_lsetxattr
long SYS_lsetxattr();
#define SYS_fsetxattr SYS_fsetxattr
long SYS_fsetxattr();
#define SYS_getxattr SYS_getxattr
long SYS_getxattr();
#define SYS_lgetxattr SYS_lgetxattr
long SYS_lgetxattr();
#define SYS_fgetxattr SYS_fgetxattr
long SYS_fgetxattr();
#define SYS_listxattr SYS_listxattr
long SYS_listxattr();
#define SYS_llistxattr SYS_llistxattr
long SYS_llistxattr();
#define SYS_flistxattr SYS_flistxattr
long SYS_flistxattr();
#define SYS_removexattr SYS_removexattr
long SYS_removexattr();
#define SYS_lremovexattr SYS_lremovexattr
long SYS_lremovexattr();
#define SYS_fremovexattr SYS_fremovexattr
long SYS_fremovexattr();
#define SYS_tkill SYS_tkill
long SYS_tkill();
#define SYS_sendfile64 SYS_sendfile64
long SYS_sendfile64();
#define SYS_futex SYS_futex
long SYS_futex();
#define SYS_sched_setaffinity SYS_sched_setaffinity
long SYS_sched_setaffinity();
#define SYS_sched_getaffinity SYS_sched_getaffinity
long SYS_sched_getaffinity();
#define SYS_io_setup SYS_io_setup
long SYS_io_setup();
#define SYS_io_destroy SYS_io_destroy
long SYS_io_destroy();
#define SYS_io_getevents SYS_io_getevents
long SYS_io_getevents();
#define SYS_io_submit SYS_io_submit
long SYS_io_submit();
#define SYS_io_cancel SYS_io_cancel
long SYS_io_cancel();
#define SYS_exit_group SYS_exit_group
long SYS_exit_group();
#define SYS_lookup_dcookie SYS_lookup_dcookie
long SYS_lookup_dcookie();
#define SYS_epoll_create SYS_epoll_create
long SYS_epoll_create();
#define SYS_epoll_ctl SYS_epoll_ctl
long SYS_epoll_ctl();
#define SYS_epoll_wait SYS_epoll_wait
long SYS_epoll_wait();
#define SYS_remap_file_pages SYS_remap_file_pages
long SYS_remap_file_pages();
#define SYS_set_tid_address SYS_set_tid_address
long SYS_set_tid_address();
#define SYS_timer_create SYS_timer_create
long SYS_timer_create();
#define SYS_timer_settime SYS_timer_settime
long SYS_timer_settime();
#define SYS_timer_gettime SYS_timer_gettime
long SYS_timer_gettime();
#define SYS_timer_getoverrun SYS_timer_getoverrun
long SYS_timer_getoverrun();
#define SYS_timer_delete SYS_timer_delete
long SYS_timer_delete();
#define SYS_clock_settime SYS_clock_settime
long SYS_clock_settime();
#define SYS_clock_gettime SYS_clock_gettime
long SYS_clock_gettime();
#define SYS_clock_getres SYS_clock_getres
long SYS_clock_getres();
#define SYS_clock_nanosleep SYS_clock_nanosleep
long SYS_clock_nanosleep();
#define SYS_statfs64 SYS_statfs64
long SYS_statfs64();
#define SYS_fstatfs64 SYS_fstatfs64
long SYS_fstatfs64();
#define SYS_tgkill SYS_tgkill
long SYS_tgkill();
#define SYS_utimes SYS_utimes
long SYS_utimes();
#define SYS_fadvise64_64 SYS_fadvise64_64
long SYS_fadvise64_64();
#define SYS_arm_fadvise64_64 SYS_arm_fadvise64_64
long SYS_arm_fadvise64_64();
#define SYS_pciconfig_iobase SYS_pciconfig_iobase
long SYS_pciconfig_iobase();
#define SYS_pciconfig_read SYS_pciconfig_read
long SYS_pciconfig_read();
#define SYS_pciconfig_write SYS_pciconfig_write
long SYS_pciconfig_write();
#define SYS_mq_open SYS_mq_open
long SYS_mq_open();
#define SYS_mq_unlink SYS_mq_unlink
long SYS_mq_unlink();
#define SYS_mq_timedsend SYS_mq_timedsend
long SYS_mq_timedsend();
#define SYS_mq_timedreceive SYS_mq_timedreceive
long SYS_mq_timedreceive();
#define SYS_mq_notify SYS_mq_notify
long SYS_mq_notify();
#define SYS_mq_getsetattr SYS_mq_getsetattr
long SYS_mq_getsetattr();
#define SYS_waitid SYS_waitid
long SYS_waitid();
#define SYS_socket SYS_socket
long SYS_socket();
#define SYS_bind SYS_bind
long SYS_bind();
#define SYS_connect SYS_connect
long SYS_connect();
#define SYS_listen SYS_listen
long SYS_listen();
#define SYS_accept SYS_accept
long SYS_accept();
#define SYS_getsockname SYS_getsockname
long SYS_getsockname();
#define SYS_getpeername SYS_getpeername
long SYS_getpeername();
#define SYS_socketpair SYS_socketpair
long SYS_socketpair();
#define SYS_send SYS_send
long SYS_send();
#define SYS_sendto SYS_sendto
long SYS_sendto();
#define SYS_recv SYS_recv
long SYS_recv();
#define SYS_recvfrom SYS_recvfrom
long SYS_recvfrom();
#define SYS_shutdown SYS_shutdown
long SYS_shutdown();
#define SYS_setsockopt SYS_setsockopt
long SYS_setsockopt();
#define SYS_getsockopt SYS_getsockopt
long SYS_getsockopt();
#define SYS_sendmsg SYS_sendmsg
long SYS_sendmsg();
#define SYS_recvmsg SYS_recvmsg
long SYS_recvmsg();
#define SYS_semop SYS_semop
long SYS_semop();
#define SYS_semget SYS_semget
long SYS_semget();
#define SYS_semctl SYS_semctl
long SYS_semctl();
#define SYS_msgsnd SYS_msgsnd
long SYS_msgsnd();
#define SYS_msgrcv SYS_msgrcv
long SYS_msgrcv();
#define SYS_msgget SYS_msgget
long SYS_msgget();
#define SYS_msgctl SYS_msgctl
long SYS_msgctl();
#define SYS_shmat SYS_shmat
long SYS_shmat();
#define SYS_shmdt SYS_shmdt
long SYS_shmdt();
#define SYS_shmget SYS_shmget
long SYS_shmget();
#define SYS_shmctl SYS_shmctl
long SYS_shmctl();
#define SYS_add_key SYS_add_key
long SYS_add_key();
#define SYS_request_key SYS_request_key
long SYS_request_key();
#define SYS_keyctl SYS_keyctl
long SYS_keyctl();
#define SYS_semtimedop SYS_semtimedop
long SYS_semtimedop();
#define SYS_vserver SYS_vserver
long SYS_vserver();
#define SYS_ioprio_set SYS_ioprio_set
long SYS_ioprio_set();
#define SYS_ioprio_get SYS_ioprio_get
long SYS_ioprio_get();
#define SYS_inotify_init SYS_inotify_init
long SYS_inotify_init();
#define SYS_inotify_add_watch SYS_inotify_add_watch
long SYS_inotify_add_watch();
#define SYS_inotify_rm_watch SYS_inotify_rm_watch
long SYS_inotify_rm_watch();
#define SYS_mbind SYS_mbind
long SYS_mbind();
#define SYS_get_mempolicy SYS_get_mempolicy
long SYS_get_mempolicy();
#define SYS_set_mempolicy SYS_set_mempolicy
long SYS_set_mempolicy();
#define SYS_openat SYS_openat
long SYS_openat();
#define SYS_mkdirat SYS_mkdirat
long SYS_mkdirat();
#define SYS_mknodat SYS_mknodat
long SYS_mknodat();
#define SYS_fchownat SYS_fchownat
long SYS_fchownat();
#define SYS_futimesat SYS_futimesat
long SYS_futimesat();
#define SYS_fstatat64 SYS_fstatat64
long SYS_fstatat64();
#define SYS_unlinkat SYS_unlinkat
long SYS_unlinkat();
#define SYS_renameat SYS_renameat
long SYS_renameat();
#define SYS_linkat SYS_linkat
long SYS_linkat();
#define SYS_symlinkat SYS_symlinkat
long SYS_symlinkat();
#define SYS_readlinkat SYS_readlinkat
long SYS_readlinkat();
#define SYS_fchmodat SYS_fchmodat
long SYS_fchmodat();
#define SYS_faccessat SYS_faccessat
long SYS_faccessat();
#define SYS_pselect6 SYS_pselect6
long SYS_pselect6();
#define SYS_ppoll SYS_ppoll
long SYS_ppoll();
#define SYS_unshare SYS_unshare
long SYS_unshare();
#define SYS_set_robust_list SYS_set_robust_list
long SYS_set_robust_list();
#define SYS_get_robust_list SYS_get_robust_list
long SYS_get_robust_list();
#define SYS_splice SYS_splice
long SYS_splice();
#define SYS_sync_file_range2 SYS_sync_file_range2
long SYS_sync_file_range2();
#define SYS_arm_sync_file_range SYS_arm_sync_file_range
long SYS_arm_sync_file_range();
#define SYS_tee SYS_tee
long SYS_tee();
#define SYS_vmsplice SYS_vmsplice
long SYS_vmsplice();
#define SYS_move_pages SYS_move_pages
long SYS_move_pages();
#define SYS_getcpu SYS_getcpu
long SYS_getcpu();
#define SYS_epoll_pwait SYS_epoll_pwait
long SYS_epoll_pwait();
#define SYS_kexec_load SYS_kexec_load
long SYS_kexec_load();
#define SYS_utimensat SYS_utimensat
long SYS_utimensat();
#define SYS_signalfd SYS_signalfd
long SYS_signalfd();
#define SYS_timerfd_create SYS_timerfd_create
long SYS_timerfd_create();
#define SYS_eventfd SYS_eventfd
long SYS_eventfd();
#define SYS_fallocate SYS_fallocate
long SYS_fallocate();
#define SYS_timerfd_settime SYS_timerfd_settime
long SYS_timerfd_settime();
#define SYS_timerfd_gettime SYS_timerfd_gettime
long SYS_timerfd_gettime();
#define SYS_signalfd4 SYS_signalfd4
long SYS_signalfd4();
#define SYS_eventfd2 SYS_eventfd2
long SYS_eventfd2();
#define SYS_epoll_create1 SYS_epoll_create1
long SYS_epoll_create1();
#define SYS_dup3 SYS_dup3
long SYS_dup3();
#define SYS_pipe2 SYS_pipe2
long SYS_pipe2();
#define SYS_inotify_init1 SYS_inotify_init1
long SYS_inotify_init1();
#define SYS_preadv SYS_preadv
long SYS_preadv();
#define SYS_pwritev SYS_pwritev
long SYS_pwritev();
#define SYS_rt_tgsigqueueinfo SYS_rt_tgsigqueueinfo
long SYS_rt_tgsigqueueinfo();
#define SYS_perf_event_open SYS_perf_event_open
long SYS_perf_event_open();
#define SYS_recvmmsg SYS_recvmmsg
long SYS_recvmmsg();
#define SYS_accept4 SYS_accept4
long SYS_accept4();
#define SYS_fanotify_init SYS_fanotify_init
long SYS_fanotify_init();
#define SYS_fanotify_mark SYS_fanotify_mark
long SYS_fanotify_mark();
#define SYS_prlimit64 SYS_prlimit64
long SYS_prlimit64();
#define SYS_name_to_handle_at SYS_name_to_handle_at
long SYS_name_to_handle_at();
#define SYS_open_by_handle_at SYS_open_by_handle_at
long SYS_open_by_handle_at();
#define SYS_clock_adjtime SYS_clock_adjtime
long SYS_clock_adjtime();
#define SYS_syncfs SYS_syncfs
long SYS_syncfs();
#define SYS_sendmmsg SYS_sendmmsg
long SYS_sendmmsg();
#define SYS_setns SYS_setns
long SYS_setns();
#define SYS_process_vm_readv SYS_process_vm_readv
long SYS_process_vm_readv();
#define SYS_process_vm_writev SYS_process_vm_writev
long SYS_process_vm_writev();
#define SYS_kcmp SYS_kcmp
long SYS_kcmp();
#define SYS_finit_module SYS_finit_module
long SYS_finit_module();
#define SYS_sched_setattr SYS_sched_setattr
long SYS_sched_setattr();
#define SYS_sched_getattr SYS_sched_getattr
long SYS_sched_getattr();
#define SYS_renameat2 SYS_renameat2
long SYS_renameat2();
#define SYS_seccomp SYS_seccomp
long SYS_seccomp();
#define SYS_getrandom SYS_getrandom
long SYS_getrandom();
#define SYS_memfd_create SYS_memfd_create
long SYS_memfd_create();
#define SYS_bpf SYS_bpf
long SYS_bpf();
#define SYS_execveat SYS_execveat
long SYS_execveat();
#define SYS_userfaultfd SYS_userfaultfd
long SYS_userfaultfd();
#define SYS_membarrier SYS_membarrier
long SYS_membarrier();
#define SYS_mlock2 SYS_mlock2
long SYS_mlock2();
#define SYS_copy_file_range SYS_copy_file_range
long SYS_copy_file_range();
#define SYS_preadv2 SYS_preadv2
long SYS_preadv2();
#define SYS_pwritev2 SYS_pwritev2
long SYS_pwritev2();
#define SYS_pkey_mprotect SYS_pkey_mprotect
long SYS_pkey_mprotect();
#define SYS_pkey_alloc SYS_pkey_alloc
long SYS_pkey_alloc();
#define SYS_pkey_free SYS_pkey_free
long SYS_pkey_free();

#define ARM_SYS_cacheflush ARM_SYS_cacheflush
long ARM_SYS_cacheflush();
#define ARM_SYS_set_tls ARM_SYS_set_tls
long ARM_SYS_set_tls();
