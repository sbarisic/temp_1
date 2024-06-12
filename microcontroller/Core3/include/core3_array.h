#pragma once

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct core3_array_def {
	size_t len;
	size_t element_size;
	void* memory;

} core3_array;

#ifdef __cplusplus
}
#endif