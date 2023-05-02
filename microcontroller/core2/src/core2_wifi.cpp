#include <core2.h>
#include <WiFi.h>

void c2_wifi_task(void *params)
{
    dprintf("c2_wifi_task() STARTED\n");
    bool DoNothing = false;
    bool DoConnect = true;

    for (;;)
    {
        wl_status_t WiFiStatus = WiFi.status();
        dprintf("WiFi.status = %d\n", (int)WiFiStatus);

        switch (WiFiStatus)
        {
        case WL_IDLE_STATUS:
            break;

        case WL_NO_SSID_AVAIL:
            break;

        case WL_SCAN_COMPLETED:
            break;

        case WL_CONNECTED:
            DoNothing = true;
            break;

        case WL_CONNECT_FAILED:
            break;

        case WL_CONNECTION_LOST:
            break;

        case WL_DISCONNECTED:
            break;

        case WL_NO_SHIELD:
        default:
            // TODO: Handle
            break;
        }

        if (DoNothing)
        {
            DoNothing = false;
            dprintf("DoNothing\n");

            vTaskDelay(pdMS_TO_TICKS(2000));
            continue;
        }

        if (DoConnect)
        {
            DoConnect = false;
            dprintf("DoConnect\n");

            WiFi.mode(WIFI_STA);
            WiFi.begin("TEST", "123456789");
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

bool core2_wifi_init()
{
    dprintf("core2_wifi_init() BEGIN\n");

    xTaskCreate(c2_wifi_task, "c2_wifi_task", 4096, NULL, 1, NULL);

    dprintf("core2_wifi_init() END\n");
    return true;
}