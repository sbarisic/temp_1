#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <WiFi.h>

#define CORE2_DEBUG
#define CORE2_DEBUG_WIFI

#ifdef CORE2_DEBUG
#define dprintf printf
#else
#define dprintf(...)
#endif

void core2_init();
void core2_print_status();

SemaphoreHandle_t core2_lock_create();
void core2_lock_begin(SemaphoreHandle_t lock);
void core2_lock_end(SemaphoreHandle_t lock);

bool core2_oled_init();
void core2_oled_print(const char *txt);

bool core2_wifi_init();
bool core2_wifi_isconnected();
IPAddress core2_wifi_getip();

int32_t core2_clock_bootseconds();
int32_t core2_clock_seconds_since(int32_t lastTime);
void core2_clock_time_now();