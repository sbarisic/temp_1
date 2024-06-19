#include <core3.h>
#include <stdio.h>
#include <string.h>

// Reference counting ==============================================================

void ref_init_object(core3_object obj) {
	obj->gc_info.collected = false;
	obj->gc_info.ref_count = 0;
	obj->gc_info.objtype = obj_type_default;
	obj->gc_info.dtor = NULL;
}

core3_object ref_take(core3_object obj) {
	obj->gc_info.ref_count++;
	return obj;
}

bool ref_release(core3_object obj) {
	if (obj == NULL)
		return false;

	obj->gc_info.ref_count--;

	if (obj->gc_info.ref_count < 0 && !obj->gc_info.collected) {
		obj->gc_info.collected = true;

		core3_state* state = core3_get_state();

		if (obj->gc_info.dtor != NULL) {
			obj->gc_info.dtor(obj);
		}

		if (obj->memory != NULL) {
			state->free(obj->memory);
			obj->memory = NULL;
			obj->len = 0;
		}

		state->free(obj);
		return true;
	}

	return false;
}

void core3_ref_cleanup(core3_object* objptr) {
	ref_release(*objptr);
	*objptr = NULL;
}

void obj_destructor(core3_object obj, dtor_func dtor) {
	obj->gc_info.dtor = dtor;
}

// Object creation/destruction ==============================================================

core3_object obj_create(size_t len) {
	if (len < 0)
		return NULL;

	core3_state* state = core3_get_state();

	core3_object obj = (core3_object)state->alloc(sizeof(struct core3_object_def));
	ref_init_object(obj);

	obj->len = len;
	obj->memory = NULL;

	if (len > 0) {
		obj->memory = state->alloc(len);
	}

	return obj;
}

void obj_destroy(core3_object obj) {
	obj->gc_info.ref_count = 0;
	ref_release(obj);
}

// Strings ==============================================================

core3_object obj_create_string(const char* cstr) {
	size_t len = strlen(cstr) + 1;

	core3_object obj = obj_create(len);
	obj->gc_info.objtype = obj_type_string;

	memcpy(obj->memory, cstr, len - 1);
	return obj;
}

core3_object obj_realloc(core3_object obj, size_t new_len) {
	core3_state* state = core3_get_state();
	size_t copy_len = obj->len;

	if (obj->len > new_len) {
		copy_len = new_len;
	}

	void* new_buffer = state->alloc(new_len);
	memcpy(new_buffer, obj->memory, copy_len);

	obj->len = new_len;

	state->free(obj->memory);
	obj->memory = new_buffer;

	return obj;
}