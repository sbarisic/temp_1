#include <core3.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>

void* gc_allocate(size_t sz) {
	void* mem = malloc(sz);
	printf("Allocating memory! %p\n", mem);
	return mem;
}

void gc_free(void* ptr) {
	void* mem = *(void**)ptr;
	printf("Freeing memory! %p\n", mem);
	free(mem);
}

int main() {
	cleanup(gc_free) char* ptr = (char*)gc_allocate(32);

	memset(ptr, 0, 32);
	sprintf(ptr, "Hello %s\n", "World!");
	printf("%s\n", ptr);

	printf("%s\n", "Done!");
	return 0;
}