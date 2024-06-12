#pragma once

#include <stddef.h>
#include <stdbool.h>

#define pure __attribute__((pure))
#define allocatorfunc __attribute__((malloc))
#define allocatorfunc2(dealloc) __attribute__((malloc(dealloc)))
//#define cleanup(f) __attribute__((cleanup(f)))
#define nonstring __attribute__((nonstring))
#define packed __attribute__((packed))
#define unused __attribute__((unused))
#define used __attribute__((used))
#define interrupt __attribute__((interrupt))

#include <core3_array.h>
#include <core3_object.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*action_func)();
typedef void* (*alloc_func)(size_t sz);
typedef void (*free_func)(void* mem);

typedef struct core3_state_struct {
	alloc_func alloc;
	free_func free;

	action_func setup;
	action_func main;
} core3_state;

void core3_init(core3_state state);
core3_state* core3_get_state();

#ifdef __cplusplus
}
#endif