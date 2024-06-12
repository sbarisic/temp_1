#include <core3.h>
#include <stdlib.h>

core3_state state;

void core3_init(alloc_func falloc, free_func ffree) {
	if (falloc != NULL && ffree != NULL) {
		state.alloc = falloc;
		state.free = ffree;
	} else {
        state.alloc = malloc;
        state.free = free;
    }
}

core3_state* core3_get_state() {
	return &state;
}