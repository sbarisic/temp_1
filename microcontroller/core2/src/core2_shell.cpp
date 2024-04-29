#include <ESPTelnet.h>
#include <EscapeCodes.h>
#include <core2.h>
#include <getopt.h>

// https://github.com/LennartHennigs/ESPTelnet
// MIT License

#define DISABLE_TELNET
#define TELNET_PORT 1123

// ESPTelnet telnet;
EscapeCodes ansi;

core2_shell_cmd_t shell_commands[64];
core2_array_t *shell_cvars; // array of core2_shell_cvar_t*

#ifndef DISABLE_TELNET
#define tprintfln(...)                                                                                                 \
    do                                                                                                                 \
    {                                                                                                                  \
        telnet.printf(__VA_ARGS__);                                                                                    \
        telnet.print("\r\n");                                                                                          \
    } while (false)
#endif

void core2_shell_register(const char *func_name, core2_shell_func func)
{
    for (size_t i = 0; i < (sizeof(shell_commands) / sizeof(*shell_commands)); i++)
    {
        if (shell_commands[i].name == NULL)
        {
            shell_commands[i].name = func_name; // TODO: Copy string?
            shell_commands[i].func = func;
            return;
        }

        if (shell_commands[i].name != NULL && !strcmp(shell_commands[i].name, func_name))
        {
            dprintf("core2_shell_register() fail, function already exists '%s'\n", func_name);
            return;
        }
    }
}

void core2_shell_cvar_register(core2_shell_cvar_t *cvar, const char *var_name, void *var_ptr, core2_cvar_type var_type)
{
    if (cvar == NULL)
    {
        dprintf("core2_shell_register_var(\"%s\", cvar) fail, cvar is NULL", var_name);
        return;
    }

    if (core2_shell_cvar_find(var_name) != NULL)
    {
        eprintf("core2_shell_register_var() - cvar \"%s\" already exists\n", var_name);
        return;
    }

    cvar->name = var_name;
    cvar->var_ptr = var_ptr;
    cvar->var_type = var_type;

    core2_array_insert_end(shell_cvars, &cvar);
}

size_t core2_shell_cvar_count()
{
    return shell_cvars->length;
}

core2_shell_cvar_t *core2_shell_cvar_get(int idx)
{
    core2_shell_cvar_t **cvar = NULL;

    core2_array_get(shell_cvars, idx, &cvar);

    return *cvar;
}

core2_shell_cvar_t *core2_shell_cvar_find(const char *var_name)
{
    size_t len = core2_shell_cvar_count();

    for (size_t i = 0; i < len; i++)
    {
        core2_shell_cvar_t *cvar = core2_shell_cvar_get(i);

        if (!strcmp(cvar->name, var_name))
            return cvar;
    }

    return NULL;
}

bool core2_shell_cvar_tostring(core2_shell_cvar_t *cvar, char *buf)
{
    if (cvar == NULL)
    {
        eprintf("core2_shell_cvar_tostring() - cvar is NULL\n");
        return false;
    }

    if (buf == NULL)
    {
        eprintf("core2_shell_cvar_tostring() - buf is NULL\n");
        return false;
    }

    switch (cvar->var_type)
    {
    case CORE2_CVAR_STRING:
        sprintf(buf, "%s = \"%s\"", cvar->name, cvar->var_ptr);
        return true;

    case CORE2_CVAR_INT32:
        sprintf(buf, "%s = %d", cvar->name, (int32_t)cvar->var_ptr);
        return true;

    default:
        eprintf("core2_shell_cvar_tostring() - unkown var_type %d\n", cvar->var_type);
        return false;
    }

    return true;
}

void core2_shellcmd_help(core2_shell_func_params_t *params)
{
    size_t buf_len = 128;
    char *buf = (char *)core2_malloc(buf_len);

    for (size_t i = 0; i < (sizeof(shell_commands) / sizeof(*shell_commands)); i++)
    {
        if (shell_commands[i].name == NULL)
            return;

        memset(buf, 0, buf_len);
        sprintf(buf, " %s @ %p\n", shell_commands[i].name, (void *)shell_commands[i].func);
        params->print(params, buf);
    }

    core2_free(buf);
}

void core2_shell_init_commands()
{
    for (size_t i = 0; i < (sizeof(shell_commands) / sizeof(*shell_commands)); i++)
    {
        shell_commands[i] = {0};
    }

    core2_shell_register("help", core2_shellcmd_help);
}

bool core2_shell_invoke(const char *full_command, core2_shell_func_params_t *params)
{
    dprintf("core2_shell_invoke(\"%s\")\n", full_command);

    const char *func_name = full_command; // TODO: Parse properly

    for (size_t i = 0; i < (sizeof(shell_commands) / sizeof(*shell_commands)); i++)
    {
        if (shell_commands[i].name != NULL && !strcmp(shell_commands[i].name, func_name))
        {
            dprintf(">> Executing '%s'\n", func_name);

            shell_commands[i].func(params);
            return true;
        }
    }

    return false;
}

void core2_shell_init_cvars()
{
    shell_cvars = core2_array_create(sizeof(core2_shell_cvar_t *));
}

void core2_shell_save_cvars()
{
    int len = core2_shell_cvar_count();
    for (size_t i = 0; i < len; i++)
    {
        core2_shell_cvar_t *cvar = core2_shell_cvar_get(i);
        core2_flash_cvar_store(cvar);
    }
}

// ================================================================================

#ifndef DISABLE_TELNET
void print_prompt()
{
    telnet.print(ansi.setFG(ANSI_BRIGHT_WHITE));
    telnet.print("core$");
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
    if (str == "quit")
    {
        tprintfln("Leaving shell prompt");
        telnet.disconnectClient();
        return;
    }

    if (!core2_shell_invoke(str))
    {
        tprintfln("Unknown command '%s'", str.c_str());
    }

    tprintfln("");
    print_prompt();
}

void c2_telnet_task(void *params)
{
    for (;;)
    {
        telnet.loop();
        // vTaskDelay(pdMS_TO_TICKS(10));
    }

    vTaskDelete(NULL);
}

#endif
// ================================================================================

void core2_shell_init()
{
    dprintf("core2_shell_init()\n");
    core2_shell_init_commands();
    core2_shell_init_cvars();

#ifndef DISABLE_TELNET
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
#endif
}