#include <core2.h>
#include <getopt.h>

core2_array_t *shell_cmds;  // array of core2_shell_cmd_t*
core2_array_t *shell_cvars; // array of core2_shell_cvar_t*

size_t core2_shell_cmd_count()
{
    return shell_cmds->length;
}

core2_shell_cmd_t *core2_shell_cmd_get(int idx)
{
    core2_shell_cmd_t *cmd = NULL;
    core2_array_get(shell_cmds, idx, &cmd);
    return cmd;
}

core2_shell_cmd_t *core2_shell_cmd_find(const char *var_name)
{
    size_t len = core2_shell_cmd_count();

    for (size_t i = 0; i < len; i++)
    {
        core2_shell_cmd_t *cvar = core2_shell_cmd_get(i);

        if (!strcmp(cvar->name, var_name))
            return cvar;
    }

    return NULL;
}

void core2_shell_register(const char *func_name, core2_shell_func func)
{
    dprintf("core2_shell_register(\"%s\", %p)\n", func_name, func);

    // size_t cmd_count = core2_shell_cmd_count();

    core2_shell_cmd_t *cmd = core2_shell_cmd_find(func_name);
    if (cmd != NULL)
    {
        eprintf("core2_shell_register() fail, function already exists '%s'\n", func_name);
        return;
    }

    core2_shell_cmd_t new_cmd;
    new_cmd.name = core2_string_copy(func_name);
    new_cmd.func = func;
    core2_array_insert_end(shell_cmds, &new_cmd);
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

    dprintf("core2_shell_cvar_register(%p, \"%s\", %p, %d)\n", cvar, var_name, var_ptr, var_type);
    cvar->name = core2_string_copy(var_name);
    cvar->var_ptr = var_ptr;
    cvar->var_type = var_type;

    core2_array_insert_end(shell_cvars, cvar);
}

size_t core2_shell_cvar_count()
{
    return shell_cvars->length;
}

core2_shell_cvar_t *core2_shell_cvar_get(int idx)
{
    core2_shell_cvar_t *cvar = NULL;
    core2_array_get(shell_cvars, idx, &cvar);
    return cvar;
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

bool core2_shell_invoke(const char *full_command, core2_shell_func_params_t *params)
{
    dprintf("core2_shell_invoke(\"%s\")\n", full_command);

    const char *func_name = full_command; // TODO: Parse properly

    size_t cmd_count = core2_shell_cmd_count();
    for (size_t i = 0; i < cmd_count; i++)
    {
        core2_shell_cmd_t *cmd = core2_shell_cmd_get(i);

        if (cmd->name != NULL && !strcmp(cmd->name, func_name))
        {
            dprintf(">> Executing '%s'\n", func_name);

            cmd->func(params);
            return true;
        }
    }

    return false;
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

void core2_shell_load_cvars()
{
    int len = core2_shell_cvar_count();
    for (size_t i = 0; i < len; i++)
    {
        core2_shell_cvar_t *cvar = core2_shell_cvar_get(i);
        core2_flash_cvar_load(cvar);
    }
}

//================ Console functions =========================================================

void shell_help(core2_shell_func_params_t *params)
{
    dprintf("shell_help!\n");

    size_t buf_len = 128;
    char *buf = (char *)core2_malloc(buf_len);

    size_t cmd_count = core2_shell_cmd_count();
    for (size_t i = 0; i < cmd_count; i++)
    {
        core2_shell_cmd_t *cmd = core2_shell_cmd_get(i);

        if (cmd->name == NULL)
            return;

        memset(buf, 0, buf_len);
        sprintf(buf, "%s @ %p\n", cmd->name, (void *)cmd->func);

        dprintf(buf);
        params->print(params, buf);
    }

    core2_free(buf);
}

void shell_list_cvar(core2_shell_func_params_t *params)
{
    dprintf("shell_list_cvar!\n");

    size_t buf_len = 128;
    char *buf = (char *)core2_malloc(buf_len);

    size_t cvar_count = core2_shell_cvar_count();
    for (size_t i = 0; i < cvar_count; i++)
    {
        core2_shell_cvar_t *cvar = core2_shell_cvar_get(i);

        if (cvar->name == NULL)
            return;

        memset(buf, 0, buf_len);

        // sprintf(buf, " %s @ %p\n", cvar->name, (void *)cmd->func);
        core2_shell_cvar_tostring(cvar, buf);

        dprintf(buf);
        dprintf("\n");

        params->print(params, buf);
        params->print(params, "\n");
    }

    core2_free(buf);
}

void core2_shell_init()
{
    dprintf("core2_shell_init()\n");

    shell_cmds = core2_array_create(sizeof(core2_shell_cmd_t *));
    shell_cvars = core2_array_create(sizeof(core2_shell_cvar_t *));

    core2_shell_register("help", shell_help);
    core2_shell_register("list_cvar", shell_list_cvar);
}
