#pragma once

#include <stddef.h>


#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	obj_type_default = 0,
	obj_type_string
} core3_object_type;

typedef struct core3_object_def {
	struct core3_gc_info {
		bool collected;
		int ref_count;
		core3_object_type objtype;
		void (*dtor)(struct core3_object_def* obj);
	} gc_info;

	// core3_gc_info ;
	size_t len;
	void* memory;

} * core3_object;

typedef void (*dtor_func)(core3_object obj);

core3_object ref_take(core3_object obj);
bool ref_release(core3_object obj);
void core3_ref_cleanup(core3_object* objptr);
void obj_destructor(core3_object obj, dtor_func dtor);

core3_object obj_create(size_t len);
core3_object obj_create_string(const char* cstr);

core3_object obj_realloc(core3_object obj, size_t new_len);

void obj_destroy(core3_object obj);



#ifdef __cplusplus
}
#endif



#define managed __attribute__((cleanup(core3_ref_cleanup)))
