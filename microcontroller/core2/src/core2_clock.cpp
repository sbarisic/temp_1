#include <core2.h>

#ifdef CORE2_WINDOWS
#include <time.h>
#else
#include <rtc.h>
#endif

#define _tzset tzset

// @brief Get seconds since boot
int32_t core2_clock_bootseconds()
{
#ifdef CORE2_WINDOWS
    return 0;
#else
    return (int32_t)(esp_timer_get_time() / 1000000);
#endif
}

int64_t core2_clock_bootms()
{
    return (int64_t)(esp_timer_get_time() / 1000);
}

// @brief Seconds since lastTime
int32_t core2_clock_seconds_since(int32_t lastTime)
{
    return core2_clock_bootseconds() - lastTime;
}

// @brief Formats time to DD.MM.YYYY. HH:MM:SS, buffer requires 20 chars
void core2_clock_time_now(char *strftime_buf)
{
    // dprintf("core2_clock_time_now()\n");
    time_t now = time(NULL);
    struct tm *timeinfo = localtime(&now);
    strftime(strftime_buf, 21, "%d.%m.%Y. %H:%M:%S", timeinfo);
    // dprintf("%s\n", strftime_buf);
}

void core2_clock_time_fmt(char *strftime_buf, size_t max_size, const char *fmt)
{
    time_t now = time(NULL);
    struct tm *timeinfo = localtime(&now);
    strftime(strftime_buf, max_size, fmt, timeinfo);
}

#ifndef CORE2_WINDOWS
void core2_clock_update_from_ntp()
{
    const char *ntpServer = "pool.ntp.org";
    const long gmtOffset_sec = 3600;
    const int daylightOffset_sec = 3600;
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

    core2_sleep(10);

    struct tm timeinfo;
    while (!getLocalTime(&timeinfo))
    {
        core2_sleep(50);
    }

    setenv("TZ", "UTC-2", 1);
    _tzset();
}
#endif

bool core2_clock_init()
{
    dprintf("core2_clock_init()\n");

#ifndef CORE2_WINDOWS
#ifndef CORE2_DISABLE_WIFI
    core2_wifi_yield_until_connected();
#endif

    core2_clock_update_from_ntp();
#endif

    return true;
}