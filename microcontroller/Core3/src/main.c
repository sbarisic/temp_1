#include <core3.h>
#include <stdio.h>
#include <stdlib.h>

void destructor(core3_object obj) {
	printf("Destroying object of length %ld\n", obj->len);
}

int main() {
	core3_init(NULL, NULL);

	managed core3_object obj = obj_create(16);
	obj_destructor(obj, destructor);

	printf("Done!\n");
	return 0;
}