#include <core2.h>
#include <ESPTelnet.h>
#include <EscapeCodes.h>

// https://github.com/LennartHennigs/ESPTelnet
// MIT License

#define TELNET_PORT 1123

ESPTelnet telnet;
EscapeCodes ansi;

#define tprintfln(...)              \
    do                              \
    {                               \
        telnet.printf(__VA_ARGS__); \
        telnet.print("\r\n");       \
    } while (false)

// ================================================================================

void print_prompt()
{
    telnet.print(ansi.setFG(ANSI_BRIGHT_WHITE));
    telnet.print("core2$");
    telnet.print(ansi.reset());
    telnet.print(" ");
}

void onTelnetConnect(String ip)
{
    dprintf("# Telnet: %s connected\n", ip.c_str());

    tprintfln("Welcome %s, 'quit' to disconnect", telnet.getIP().c_str());
    tprintfln("");
    print_prompt();
}

void onTelnetDisconnect(String ip)
{
    dprintf("# Telnet: %s disconnected\n", ip.c_str());
}

void onTelnetReconnect(String ip)
{
    dprintf("# Telnet: %s reconnected\n", ip.c_str());

    tprintfln("Welcome back");
    tprintfln("");
    print_prompt();
}

void onTelnetConnectionAttempt(String ip)
{
    dprintf("# Telnet: %s tried to connected\n", ip.c_str());
}

void onTelnetInput(String str)
{
    // checks for a certain command
    if (str == "ping")
    {
        tprintfln("Pong");
        dprintf("Telnet: ping-pong\n");
    }
    else if (str == "quit")
    {
        tprintfln("Leaving shell prompt");
        telnet.disconnectClient();
    }
    else
    {
        tprintfln("Unknown command '%s'", str.c_str());
    }

    tprintfln("");
    print_prompt();
}

// ================================================================================

void c2_telnet_task(void *params)
{
    for (;;)
    {
        telnet.loop();
        //vTaskDelay(pdMS_TO_TICKS(10));
    }

    vTaskDelete(NULL);
}

void core2_shell_init()
{
    dprintf("core2_shell_init()\n");

    telnet.onConnect(onTelnetConnect);
    telnet.onConnectionAttempt(onTelnetConnectionAttempt);
    telnet.onReconnect(onTelnetReconnect);
    telnet.onDisconnect(onTelnetDisconnect);
    telnet.onInputReceived(onTelnetInput);

    if (telnet.begin(TELNET_PORT, false))
    {
        dprintf("Telnet server running on port %d\n", TELNET_PORT);
    }

    xTaskCreate(c2_telnet_task, "c2_telnet_task", 4096, NULL, 1, NULL);
}