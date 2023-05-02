#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

#define CORE2_DEBUG

#ifdef CORE2_DEBUG
#define dprintf printf
#else
#define dprintf(...)
#endif

void core2_init();

SemaphoreHandle_t core2_lock_create();
void core2_lock_begin(SemaphoreHandle_t lock);
void core2_lock_end(SemaphoreHandle_t lock);

bool core2_oled_init();
void core2_oled_print(const char *txt);

bool core2_wifi_init();
