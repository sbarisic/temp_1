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
    dprintf("core2_clock_time_now()\n");

    time_t now;
    char strftime_buf[64];
    struct tm timeinfo;

    dprintf("time()\n");
    time(&now);
    
    dprintf("setenv()\n");
    setenv("TZ", "GMT+1", 1);

    dprintf("tzset()\n");
    tzset();

    dprintf("localtime_r()\n");
    localtime_r(&now, &timeinfo);

    dprintf("strftime()\n");
    strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);

    dprintf("Current date/time is: %s\n", strftime_buf);
}