#pragma once

#define pure __attribute__((pure))
#define allocatorfunc __attribute__((malloc))
#define allocatorfunc2(dealloc) __attribute__((malloc(dealloc)))
#define cleanup(f) __attribute__((cleanup(f)))
#define nonstring __attribute__((nonstring))
#define packed __attribute__((packed))
#define unused __attribute__((unused))
#define used __attribute__((used))
#define interrupt __attribute__((interrupt))

#ifdef __cplusplus
extern "C" {
#endif

int shitty_cpp_func();

#ifdef __cplusplus
}
#endif