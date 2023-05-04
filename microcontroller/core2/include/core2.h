#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/queue.h>
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

xQueueHandle core2_queue_create(int count, int elementSize);
BaseType_t core2_queue_send(xQueueHandle q, const void* item);
BaseType_t core2_queue_receive(xQueueHandle q, void* buffer);

bool core2_oled_init();
void core2_oled_print(const char *txt);

bool core2_wifi_init();
bool core2_wifi_isconnected();
IPAddress core2_wifi_getip();
void core2_wifi_yield_until_connected();

bool core2_clock_init();
int32_t core2_clock_bootseconds();
int32_t core2_clock_seconds_since(int32_t lastTime);
void core2_clock_time_now(char *strftime_buf);
void core2_clock_update_from_ntp();

bool core2_gpio_init();