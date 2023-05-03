#include <core2.h>
#include <rtc.h>

// @brief Get seconds since boot
int32_t core2_clock_bootseconds()
{
    return (int32_t)(esp_timer_get_time() / 1000000);
}

// @brief Seconds since lastTime
int32_t core2_clock_seconds_since(int32_t lastTime)
{
    return core2_clock_bootseconds() - lastTime;
}

void core2_clock_time_now()
{
    time_t now;
    char strftime_buf[64];
    struct tm timeinfo;

    time(&now);
    // Set timezone to China Standard Time
    setenv("TZ", "GMT+1", 1);
    tzset();

    localtime_r(&now, &timeinfo);
    strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);

    printf("Current date/time is: %s", strftime_buf);
}