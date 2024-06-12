#include <core3.h>

core3_state stat;

void core3_init(core3_state state) {
	stat = state;
}

core3_state* core3_get_state() {
	return &stat;
}