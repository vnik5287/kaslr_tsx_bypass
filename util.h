#ifndef _UTIL_H
#define _UTIL_H

#define _XBEGIN_STARTED	(~0u)
#define _NROUNDS 10000
#define _SAFE_MARGIN 10 /* Kaby Lake CPUs */
#define _STD_DEV_THRESHOLD 5

int pin_cpu(void);
int unpin_cpu(void);
int cpu_has_rtm(void);
int get_nprocs(void);
void start_cpu_load(void);
void stop_cpu_load(void);

/* XXX: need a barrier? */
static __attribute__((__always_inline__)) inline uint64_t rdtscp() {
	uint32_t aux;
	uint64_t rax, rdx;

	asm volatile ("rdtscp\n" : "=a" (rax), "=d" (rdx), "=c" (aux) ::);

	return (rdx << 32) + rax;
}

static __attribute__((__always_inline__)) inline int _xbegin(void) {
	int ret = _XBEGIN_STARTED;

	asm volatile(".byte 0xc7,0xf8;" \
                     ".long 0" : "+a" (ret) :: "memory");
	return ret;
}

static __attribute__((__always_inline__)) inline void _xend(void) {
	 asm volatile(".byte 0x0f,0x01,0xd5" ::: "memory");
}

#endif
