#include <core2.h>

//core2_array_t* shell_cmds;  // array of core2_shell_cmd_t*
//core2_array_t* shell_cvars; // array of core2_shell_cvar_t*

core2_shell_cmd_t* shell_commands;
size_t shell_command_count;
size_t shell_command_count_max;

core2_shell_cvar_t* shell_convars;
size_t shell_convar_count;
size_t shell_convar_count_max;


size_t core2_shell_cmd_count()
{
	//return shell_cmds->length;
	return shell_command_count;
}

core2_shell_cmd_t* core2_shell_cmd_get(int idx)
{
	/*core2_shell_cmd_t* cmd = NULL;
	core2_array_get(shell_cmds, idx, &cmd);
	return cmd;*/

	return &shell_commands[idx];
}

core2_shell_cmd_t* core2_shell_cmd_find(const char* var_name)
{
	size_t len = core2_shell_cmd_count();

	for (size_t i = 0; i < len; i++)
	{
		core2_shell_cmd_t* cvar = core2_shell_cmd_get(i);

		if (!strcmp(cvar->name, var_name))
			return cvar;
	}

	return NULL;
}

void core2_shell_register(const char* func_name, core2_shell_func func)
{
	dprintf("core2_shell_register(\"%s\", %p)\n", func_name, func);

	// size_t cmd_count = core2_shell_cmd_count();

	core2_shell_cmd_t* cmd = core2_shell_cmd_find(func_name);
	if (cmd != NULL)
	{
		eprintf("core2_shell_register() fail, function already exists '%s'\n", func_name);
		return;
	}

	core2_shell_cmd_t new_cmd;
	new_cmd.name = core2_string_copy(func_name);
	new_cmd.func = func;
	//core2_array_insert_end(shell_cmds, &new_cmd);


	if (shell_command_count >= shell_command_count_max) {
		shell_command_count_max += 10;
		shell_commands = (core2_shell_cmd_t*)core2_realloc(shell_commands, shell_command_count_max * sizeof(core2_shell_cmd_t));
	}

	shell_command_count++;
	shell_commands[shell_command_count - 1] = new_cmd;
}

void core2_shell_cvar_register(core2_shell_cvar_t** cvar, const char* var_name, void* var_ptr, core2_cvar_type var_type)
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

	if (shell_convar_count >= shell_convar_count_max) {
		shell_convar_count_max += 10;
		shell_convars = (core2_shell_cvar_t*)core2_realloc(shell_convars, shell_convar_count_max * sizeof(core2_shell_cvar_t));
	}

	shell_convar_count++;
	core2_shell_cvar_t* cvar_ptr = &shell_convars[shell_convar_count - 1];
	*cvar = cvar_ptr;

	dprintf("core2_shell_cvar_register(%p, \"%s\", %p, %d)\n", cvar_ptr, var_name, var_ptr, var_type);
	cvar_ptr->name = core2_string_copy(var_name);
	cvar_ptr->var_ptr = var_ptr;
	cvar_ptr->var_type = var_type;

	if (var_type == CORE2_CVAR_STRING)
		cvar_ptr->var_ptr = core2_string_copy((char*)var_ptr);

	//core2_array_insert_end(shell_cvars, cvar);


}

size_t core2_shell_cvar_count()
{
	//return shell_cvars->length;
	return shell_convar_count;
}

core2_shell_cvar_t* core2_shell_cvar_get(int idx)
{
	/*core2_shell_cvar_t* cvar = NULL;
	core2_array_get(shell_cvars, idx, &cvar);
	return cvar;*/

	return &shell_convars[idx];
}

core2_shell_cvar_t* core2_shell_cvar_find(const char* var_name)
{
	size_t len = core2_shell_cvar_count();

	for (size_t i = 0; i < len; i++)
	{
		core2_shell_cvar_t* cvar = core2_shell_cvar_get(i);

		if (!strcmp(cvar->name, var_name))
			return cvar;
	}

	return NULL;
}

bool core2_shell_cvar_tostring(core2_shell_cvar_t* cvar, char* buf)
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

void Cmd_TokenizeString(const char* text_in, tokenize_info_t* tok_info)
{
	//dprintf("Cmd_TokenizeString(\"%s\")\n", text_in);

	const char* text;
	char* textOut;

	// clear previous args
	tok_info->cmd_argc = 0;

	if (!text_in)
	{
		return;
	}

	core2_strncpyz(tok_info->cmd_cmd, text_in, sizeof(tok_info->cmd_cmd));

	text = text_in;
	textOut = tok_info->cmd_tokenized;

	while (1)
	{
		if (tok_info->cmd_argc == MAX_STRING_TOKENS)
		{
			return; // this is usually something malicious
		}

		while (1)
		{
			// skip whitespace
			while (*text && *text <= ' ')
			{
				text++;
			}
			if (!*text)
			{
				return; // all tokens parsed
			}

			// skip // comments
			if (text[0] == '/' && text[1] == '/')
			{
				return; // all tokens parsed
			}

			// skip /* */ comments
			if (text[0] == '/' && text[1] == '*')
			{
				while (*text && (text[0] != '*' || text[1] != '/'))
				{
					text++;
				}
				if (!*text)
				{
					return; // all tokens parsed
				}
				text += 2;
			}
			else
			{
				break; // we are ready to parse a token
			}
		}

		// handle quoted strings
		// NOTE TTimo this doesn't handle \" escaping
		if (*text == '"')
		{
			tok_info->cmd_argv[tok_info->cmd_argc] = textOut;
			tok_info->cmd_argc++;
			text++;
			while (*text && *text != '"')
			{
				*textOut++ = *text++;
			}
			*textOut++ = 0;
			if (!*text)
			{
				return; // all tokens parsed
			}
			text++;
			continue;
		}

		// regular token
		tok_info->cmd_argv[tok_info->cmd_argc] = textOut;
		tok_info->cmd_argc++;

		// skip until whitespace, quote, or command
		while (*text > ' ')
		{
			if (text[0] == '"')
			{
				break;
			}

			if (text[0] == '/' && text[1] == '/')
			{
				break;
			}

			// skip /* */ comments
			if (text[0] == '/' && text[1] == '*')
			{
				break;
			}

			*textOut++ = *text++;
		}

		*textOut++ = 0;

		if (!*text)
		{
			return; // all tokens parsed
		}
	}
}

bool core2_shell_invoke(const char* full_command, core2_shell_func_params_t* params)
{
	//dprintf("core2_shell_invoke(\"%s\")\n", full_command);

	tokenize_info_t* tok_info = (tokenize_info_t*)core2_malloc(sizeof(tokenize_info_t));
	Cmd_TokenizeString(full_command, tok_info);
	//dprintf("argc %d\n", tok_info->cmd_argc);

	// params->tok_info = &tok_info;

	if (tok_info->cmd_argc == 0)
	{
		core2_free(tok_info);
		return false;
	}

	/*for (size_t i = 0; i < tok_info.cmd_argc; i++)
	{
		dprintf("argc(%d) = %s\n", i, tok_info.cmd_argv[i]);
	}*/

	const char* func_name = tok_info->cmd_argv[0]; // TODO: Parse properly
	//dprintf("invoking %s\n", func_name);

	size_t cmd_count = core2_shell_cmd_count();
	for (size_t i = 0; i < cmd_count; i++)
	{
		core2_shell_cmd_t* cmd = core2_shell_cmd_get(i);

		if (cmd->name != NULL && !strcmp(cmd->name, func_name))
		{
			dprintf(">> Executing '%s'\n", full_command);

			if (cmd->func == NULL)
				eprintf("cmd->func is NULL\n");
			else
				cmd->func(params, tok_info->cmd_argc, tok_info->cmd_argv);

			core2_free(tok_info);
			return true;
		}
	}

	core2_free(tok_info);
	return false;
}

void core2_shell_exec(const char* script, core2_shell_func_params_t* params) {
	const char* delimiter = "\n";

	char* buf = core2_string_copy(script);
	char* pch = strtok(buf, delimiter);

	while (pch != NULL) {
		dprintf("LINE> %s\n", pch);
	
		pch = strtok(NULL, delimiter);
	}

	core2_free(buf);
}

void core2_shell_save_cvars()
{
	int len = core2_shell_cvar_count();
	for (size_t i = 0; i < len; i++)
	{
		core2_shell_cvar_t* cvar = core2_shell_cvar_get(i);

#ifdef CORE2_WINDOWS
#else
		core2_flash_cvar_store(cvar);
#endif
	}
}

void core2_shell_load_cvars()
{
	int len = core2_shell_cvar_count();
	for (size_t i = 0; i < len; i++)
	{
		core2_shell_cvar_t* cvar = core2_shell_cvar_get(i);

#ifdef CORE2_WINDOWS
#else
		core2_flash_cvar_load(cvar);
#endif
	}
}

//================ Console functions =========================================================

void shell_help(core2_shell_func_params_t* params, int argc, char** argv)
{
	dprintf("shell_help!\n");

	size_t buf_len = 128;
	char* buf = (char*)core2_malloc(buf_len);

	size_t cmd_count = core2_shell_cmd_count();
	for (size_t i = 0; i < cmd_count; i++)
	{
		core2_shell_cmd_t* cmd = core2_shell_cmd_get(i);

		if (cmd->name == NULL)
			return;

		memset(buf, 0, buf_len);
		sprintf(buf, "%s @ %p\n", cmd->name, (void*)cmd->func);

		//dprintf(buf);
		params->print(params, buf);
	}

	core2_free(buf);
}

void shell_list_cvar(core2_shell_func_params_t* params, int argc, char** argv)
{
	dprintf("shell_list_cvar!\n");

	size_t buf_len = 128;
	char* buf = (char*)core2_malloc(buf_len);

	size_t cvar_count = core2_shell_cvar_count();
	for (size_t i = 0; i < cvar_count; i++)
	{
		core2_shell_cvar_t* cvar = core2_shell_cvar_get(i);

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

void shell_set(core2_shell_func_params_t* params, int argc, char** argv)
{
	//dprintf("shell_set!\n");

	if (argc < 3)
	{
		eprintf("shell_set argc < 3\n");
		return;
	}

	//dprintf("set %s %s\n", argv[1], argv[2]);

	core2_shell_cvar_t* cvar = core2_shell_cvar_find(argv[1]);
	//dprintf("Found: %s, %d\n", cvar->name, cvar->var_type);

	switch (cvar->var_type)
	{
	case CORE2_CVAR_INT32: {
		int num = atoi(argv[2]);
		cvar->var_ptr = (void*)num;
		break;
	}

	case CORE2_CVAR_STRING:
		core2_free(cvar->var_ptr);
		cvar->var_ptr = core2_string_copy(argv[2]);
		break;

	default:
		eprintf("set cvar type not implemented: %d\n", cvar->var_type);
		break;
	}
}

core2_shell_func_params_t* core2_shell_create_default_params() {
	core2_shell_func_params_t* params = (core2_shell_func_params_t*)core2_malloc(sizeof(core2_shell_func_params_t));
	params->print = core2_print;
	params->printf = core2_printf;
	return params;
}

void core2_shell_init()
{
	dprintf("core2_shell_init()\n");

	//shell_cmds = core2_array_create(sizeof(core2_shell_cmd_t*));
	//shell_cvars = core2_array_create(sizeof(core2_shell_cvar_t*));

	shell_command_count = 0;
	shell_command_count_max = 10;
	shell_commands = (core2_shell_cmd_t*)core2_malloc(shell_command_count_max * sizeof(core2_shell_cmd_t));

	shell_convar_count = 0;
	shell_convar_count_max = 10;
	shell_convars = (core2_shell_cvar_t*)core2_malloc(shell_command_count_max * sizeof(core2_shell_cvar_t)); ;

	core2_shell_register("help", shell_help);
	core2_shell_register("list_cvar", shell_list_cvar);
	core2_shell_register("set", shell_set);
}
