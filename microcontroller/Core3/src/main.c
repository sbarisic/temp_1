#include <core3.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>


int main() {
	core3_state state = {.alloc = malloc, .free = free};
	core3_init(state);



	printf("Done!\n");
	return 0;
}