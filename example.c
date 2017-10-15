#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

uint64_t get_kaslr_offset(void);

int main(void) {
	uint64_t offset = get_kaslr_offset();

	if (offset == -1)
		exit(-1);
	
	printf("offset = %lx\n", offset);

	/*
		exploit magic ...
	*/
}
