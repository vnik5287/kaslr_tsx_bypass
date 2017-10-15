/**
 * Linux exploit dev training - module 6 
 * vnik@cyseclabs.com
 **/

#define _GNU_SOURCE
#include <stdio.h>
#include <limits.h>
#include <unistd.h>
#include <sys/types.h>
#include <sched.h>
#include <pthread.h>
#include <cpuid.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sched.h>

#define RTM_BIT (1 << 11)

cpu_set_t cpuset;

int cont;

/* number of logical processors */
int get_nprocs(void) {
	return (int)sysconf(_SC_NPROCESSORS_ONLN);
}

void *loop(void *ptr) {
	while (cont) ;
}

void start_cpu_load(void) {
	int i, ncores;
	cpu_set_t set;

	CPU_ZERO(&set);

	ncores = get_nprocs() / 2; /* assuming Intel hyperthreading for now */
	pthread_t threads[ncores];

	for (i = 0; i < ncores; i++) {
		CPU_SET(i, &set);
	}

	cont = 1;	
	
	for (int i = 0; i < ncores; i++) {
		pthread_create(&threads[i], NULL, loop, NULL);
	}
}

void stop_cpu_load(void) {
	cont = 0;
}


int pin_cpu(void) {
	cpu_set_t set;

	CPU_ZERO(&set);
	CPU_SET(0, &set);

	sched_getaffinity(0, sizeof(cpuset), &cpuset); 

	if (sched_setaffinity(0, sizeof(set), &set) == -1) {
		return -1;
	}
}

int unpin_cpu(void) {
	if (sched_setaffinity(0, sizeof(cpuset), &cpuset) == -1)
		return -1;
}

/* CPU supports RTM execution if CPUID.07H.EBX.RTM [bit 11] = 1 */
int cpu_has_rtm(void) {
	if (__get_cpuid_max(0, NULL) >= 7) {
		unsigned a, b, c, d;
		__cpuid_count(7, 0, a, b, c, d);

		return (b & RTM_BIT);
	}
	return 0;

}

int get_ncores(void) {
	if (__get_cpuid_max(0, NULL) >= 7) {
		unsigned eax = 0, ebx = 0, ecx = 0, edx = 0;
		__get_cpuid(0xb, &eax, &ebx, &ecx, &edx);

		printf("ebx = %d\n", ebx);
		return (ebx >> 16) & 0xff ;
	}
	return 0;
}
