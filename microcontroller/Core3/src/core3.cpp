#include <core3.h>

#define autofree __attribute__((cleanup(free_stack)))

__attribute__((always_inline)) inline void free_stack(void *ptr)
{
}



void core3_init()
{
    autofree int* test = NULL;
}