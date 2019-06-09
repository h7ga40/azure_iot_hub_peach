#define __SYSCALL_LL_E(x) \
((union { long long ll; long l[2]; }){ .ll = x }).l[0], \
((union { long long ll; long l[2]; }){ .ll = x }).l[1]
#define __SYSCALL_LL_O(x) 0, __SYSCALL_LL_E((x))

#if 0
#define __asm_syscall(...) do { \
	__asm__ __volatile__ ( "push { lr }" ); \
	__asm__ __volatile__ ( "svc 0" \
	: "=r"(r0) : __VA_ARGS__ : "memory"); \
	__asm__ __volatile__ ( "pop { lr }" ); \
	return r0; \
	} while (0)

static inline long __syscall0(long n)
{
	register long r7 __asm__("r7") = n;
	register long r0 __asm__("r0");
	__asm_syscall("r"(r7));
}

static inline long __syscall1(long n, long a)
{
	register long r7 __asm__("r7") = n;
	register long r0 __asm__("r0") = a;
	__asm_syscall("r"(r7), "0"(r0));
}

static inline long __syscall2(long n, long a, long b)
{
	register long r7 __asm__("r7") = n;
	register long r0 __asm__("r0") = a;
	register long r1 __asm__("r1") = b;
	__asm_syscall("r"(r7), "0"(r0), "r"(r1));
}

static inline long __syscall3(long n, long a, long b, long c)
{
	register long r7 __asm__("r7") = n;
	register long r0 __asm__("r0") = a;
	register long r1 __asm__("r1") = b;
	register long r2 __asm__("r2") = c;
	__asm_syscall("r"(r7), "0"(r0), "r"(r1), "r"(r2));
}

static inline long __syscall4(long n, long a, long b, long c, long d)
{
	register long r7 __asm__("r7") = n;
	register long r0 __asm__("r0") = a;
	register long r1 __asm__("r1") = b;
	register long r2 __asm__("r2") = c;
	register long r3 __asm__("r3") = d;
	__asm_syscall("r"(r7), "0"(r0), "r"(r1), "r"(r2), "r"(r3));
}

static inline long __syscall5(long n, long a, long b, long c, long d, long e)
{
	register long r7 __asm__("r7") = n;
	register long r0 __asm__("r0") = a;
	register long r1 __asm__("r1") = b;
	register long r2 __asm__("r2") = c;
	register long r3 __asm__("r3") = d;
	register long r4 __asm__("r4") = e;
	__asm_syscall("r"(r7), "0"(r0), "r"(r1), "r"(r2), "r"(r3), "r"(r4));
}

static inline long __syscall6(long n, long a, long b, long c, long d, long e, long f)
{
	register long r7 __asm__("r7") = n;
	register long r0 __asm__("r0") = a;
	register long r1 __asm__("r1") = b;
	register long r2 __asm__("r2") = c;
	register long r3 __asm__("r3") = d;
	register long r4 __asm__("r4") = e;
	register long r5 __asm__("r5") = f;
	__asm_syscall("r"(r7), "0"(r0), "r"(r1), "r"(r2), "r"(r3), "r"(r4), "r"(r5));
}
#else
#define __scc(X) ((long) (X))
typedef long syscall_arg_t;

__attribute__((visibility("hidden")))
long __syscall_ret(unsigned long), __syscall_nr(long nr, ...);

#define __syscall0(n) n()
#define __syscall1(n,a) n(__scc(a))
#define __syscall2(n,a,b) n(__scc(a),__scc(b))
#define __syscall3(n,a,b,c) n(__scc(a),__scc(b),__scc(c))
#define __syscall4(n,a,b,c,d) n(__scc(a),__scc(b),__scc(c),__scc(d))
#define __syscall5(n,a,b,c,d,e) n(__scc(a),__scc(b),__scc(c),__scc(d),__scc(e))
#define __syscall6(n,a,b,c,d,e,f) n(__scc(a),__scc(b),__scc(c),__scc(d),__scc(e),__scc(f))
#define __syscall7(n,a,b,c,d,e,f,g) n(__scc(a),__scc(b),__scc(c),__scc(d),__scc(e),__scc(f),__scc(g))

#define __syscall_cp0(n) n()
#define __syscall_cp1(n,a) n(__scc(a))
#define __syscall_cp2(n,a,b) n(__scc(a),__scc(b))
#define __syscall_cp3(n,a,b,c) n(__scc(a),__scc(b),__scc(c))
#define __syscall_cp4(n,a,b,c,d) n(__scc(a),__scc(b),__scc(c),__scc(d))
#define __syscall_cp5(n,a,b,c,d,e) n(__scc(a),__scc(b),__scc(c),__scc(d),__scc(e))
#define __syscall_cp6(n,a,b,c,d,e,f) n(__scc(a),__scc(b),__scc(c),__scc(d),__scc(e),__scc(f))
#define __syscall_cp7(n,a,b,c,d,e,f,g) n(__scc(a),__scc(b),__scc(c),__scc(d),__scc(e),__scc(f),__scc(g))

#define __SYSCALL_NARGS_X(a,b,c,d,e,f,g,h,n,...) n
#define __SYSCALL_NARGS(...) __SYSCALL_NARGS_X(__VA_ARGS__,7,6,5,4,3,2,1,0,)
#define __SYSCALL_CONCAT_X(a,b) a##b
#define __SYSCALL_CONCAT(a,b) __SYSCALL_CONCAT_X(a,b)
#define __SYSCALL_DISP(b,...) __SYSCALL_CONCAT(b,__SYSCALL_NARGS(__VA_ARGS__))(__VA_ARGS__)

#define syscall(...) __syscall_ret(__SYSCALL_DISP(__syscall, __VA_ARGS__))
#define syscall_cp(...) __syscall_ret(__SYSCALL_DISP(__syscall_cp, __VA_ARGS__))
#endif
