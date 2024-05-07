extern "C" {
#include "core2.h"
}

void* core2_malloc(size_t sz) {
	void* ptr = malloc(sz);

	if (ptr == NULL) {
		eprintf("malloc(%zu) failed", sz);
		return NULL;
	}

	memset(ptr, 0, sz);
	return ptr;
}

void* core2_realloc(void* ptr, size_t sz) {
	void* new_ptr = realloc(ptr, sz);

	if (new_ptr == NULL) {
		eprintf("realloc(%p ,%zu) failed", ptr, sz);
	}

	return new_ptr;
}

void core2_free(void* ptr) {
	free(ptr);
}

// Resulting string needs to be free'd after use
char* core2_string_concat(const char* a, const char* b) {
	size_t len = strlen(a) + strlen(b) + 1;
	char* res = (char*)core2_malloc(len);

	if (res == NULL)
		return NULL;

	memset(res, 0, len);
	strcpy(res, a);
	strcat(res, b);

	return res;
}

char* core2_string_copy_len(const char* str, size_t len) {
	char* mem = (char*)core2_malloc(len + 1);
	memcpy(mem, str, len);
	return mem;
}

char* core2_string_copy(const char* str) {
	return core2_string_copy_len(str, strlen(str));
}

void core2_strncpyz(char* dest, const char* src, int destsize) {
	dprintf("core2_strncpyz(\"%s\")\n", src);
	strncpy(dest, src, destsize - 1);
	dest[destsize - 1] = 0;
}


bool core2_string_ends_with(const char* str, const char* end) {
	if (str == NULL || end == NULL)
		return false;

	size_t lenstr = strlen(str);
	size_t lensuffix = strlen(end);

	if (lensuffix > lenstr)
		return false;

	return strncmp(str + lenstr - lensuffix, end, lensuffix) == 0;
}

// array ===============================================================================================================================================================================



core2_array_t* core2_array_create(size_t element_size) {
#ifdef CORE2_DEBUG_ARRAY
	dprintf("core2_array_create(%d)\n", element_size);
#endif

	core2_array_t* array = (core2_array_t*)core2_malloc(sizeof(core2_array_t));
	array->element_size = element_size;
	array->length = 0;
	array->memory = NULL;
	return array;
}

void core2_array_realloc(core2_array_t* array, size_t new_length) {
	size_t new_byte_len = array->element_size * new_length;

#ifdef CORE2_DEBUG_ARRAY
	dprintf("core2_array_realloc(array, %d) - (%d bytes)\n", new_length, new_byte_len);
#endif

	if (new_length <= 0) {
		eprintf("core2_array_realloc(array, %d) - new_length <= 0!\n", new_length);
		return;
	}

	/*void* new_memory = core2_malloc(new_byte_len);
	memcpy(new_memory, array->memory, array->length);

	core2_free(array->memory);
	array->memory = new_memory;*/

	array->memory = core2_realloc(array->memory, array->element_size * array->length);

	if (array->memory == NULL) {
		array->length = 0;

		eprintf("core2_array_realloc failed with NULL\n");
		return;
	}

	array->length = new_length;
}

void core2_array_delete(core2_array_t* array) {
#ifdef CORE2_DEBUG_ARRAY
	dprintf("core2_array_delete(array)\n");
#endif

	if (array->memory != NULL) {
		core2_free(array->memory);
		array->memory = NULL;
	}

	core2_free(array);
}

bool c2_array_index_check(core2_array_t* array, int index) {
	if (index < 0 || index >= array->length) {
		eprintf("c2_array_index_check() - out of bounds array access, index %d\n", index);
		return false;
	}

	return true;
}

void core2_array_get(core2_array_t* array, int index, void* element_ptr) {
#ifdef CORE2_DEBUG_ARRAY
	dprintf("core2_array_get(array, %d, %p)\n", index, element_ptr);
#endif

	if (!c2_array_index_check(array, index)) {
		*(void**)element_ptr = NULL;
		return;
	}

	size_t offset = index * array->element_size;
	*(void**)element_ptr = (void*)((char*)array->memory + offset);
}

void core2_array_set(core2_array_t* array, int index, void* element_ptr) {
#ifdef CORE2_DEBUG_ARRAY
	dprintf("core2_array_set(array, %d, %p)\n", index, element_ptr);
#endif

	if (!c2_array_index_check(array, index)) {
		return;
	}

	char* dst_buf = (char*)array->memory;
	size_t offset = index * array->element_size;

	memcpy(dst_buf + offset, element_ptr, array->element_size);
}

void core2_array_insert_end(core2_array_t* array, void* element_ptr) {
#ifdef CORE2_DEBUG_ARRAY
	dprintf("core2_array_insert_end(array, %p)\n", element_ptr);
#endif

	core2_array_realloc(array, array->length + 1);
	core2_array_set(array, array->length - 1, element_ptr);
}

//====================== Tests ==========================================================

typedef struct {
	const char* str;
	int num;
} test_struct;

void core2_array_run_tests() {
	core2_array_t* arr = core2_array_create(sizeof(test_struct));

	test_struct ts1 = { .str = "Element 1", .num = 42 };
	core2_array_insert_end(arr, &ts1);

	test_struct ts2 = { .str = "Second Element", .num = 69 };
	core2_array_insert_end(arr, &ts2);

	for (size_t i = 0; i < 10; i++) {
		test_struct ts3;
		ts3.num = i;
		ts3.str = "for loop test";
		core2_array_insert_end(arr, &ts3);
	}

	printf("Array length = %d\n", arr->length);

	for (size_t i = 0; i < arr->length; i++) {
		test_struct* elptr = NULL;
		core2_array_get(arr, i, &elptr);

		printf("elptr = %p\n", elptr);
		printf("%d - str = \"%s\", num = %d\n", i, elptr->str, elptr->num);
	}

	core2_array_delete(arr);
}


// shell ===============================================================================================================================================================================


core2_array_t* shell_cmds;  // array of core2_shell_cmd_t*
core2_array_t* shell_cvars; // array of core2_shell_cvar_t*

size_t core2_shell_cmd_count() {
	return shell_cmds->length;
}

core2_shell_cmd_t* core2_shell_cmd_get(int idx) {
	core2_shell_cmd_t* cmd = NULL;
	core2_array_get(shell_cmds, idx, &cmd);
	return cmd;
}

core2_shell_cmd_t* core2_shell_cmd_find(const char* var_name) {
	size_t len = core2_shell_cmd_count();

	for (size_t i = 0; i < len; i++) {
		core2_shell_cmd_t* cvar = core2_shell_cmd_get(i);

		if (!strcmp(cvar->name, var_name))
			return cvar;
	}

	return NULL;
}

core2_shell_cmd_t* core2_shell_register(const char* func_name, core2_shell_func func) {
	dprintf("core2_shell_register(\"%s\", %p)\n", func_name, func);

	// size_t cmd_count = core2_shell_cmd_count();


	core2_shell_cmd_t* cmd = core2_shell_cmd_find(func_name);
	if (cmd != NULL) {
		eprintf("core2_shell_register() fail, function already exists '%s'\n", func_name);
		return NULL;;
	}

	core2_shell_cmd_t* new_cmd = (core2_shell_cmd_t*)core2_malloc(sizeof(core2_shell_cmd_t));
	new_cmd->name = core2_string_copy(func_name);
	new_cmd->func = func;
	//dprintf("%p; %s - 0x%p\n", new_cmd, new_cmd->name, new_cmd->func);
	core2_array_insert_end(shell_cmds, new_cmd);

	return new_cmd;
}

void core2_shell_cvar_register(core2_shell_cvar_t* cvar, const char* var_name, void* var_ptr, core2_cvar_type var_type) {
	if (cvar == NULL) {
		dprintf("core2_shell_register_var(\"%s\", cvar) fail, cvar is NULL", var_name);
		return;
	}

	if (core2_shell_cvar_find(var_name) != NULL) {
		eprintf("core2_shell_register_var() - cvar \"%s\" already exists\n", var_name);
		return;
	}

	dprintf("core2_shell_cvar_register(%p, \"%s\", %p, %d)\n", cvar, var_name, var_ptr, var_type);
	cvar->name = core2_string_copy(var_name);
	cvar->var_ptr = var_ptr;
	cvar->var_type = var_type;

	if (var_type == CORE2_CVAR_STRING)
		cvar->var_ptr = core2_string_copy((char*)var_ptr);

	core2_array_insert_end(shell_cvars, cvar);
}

size_t core2_shell_cvar_count() {
	return shell_cvars->length;
}

core2_shell_cvar_t* core2_shell_cvar_get(int idx) {
	core2_shell_cvar_t* cvar = NULL;
	core2_array_get(shell_cvars, idx, &cvar);
	return cvar;
}

core2_shell_cvar_t* core2_shell_cvar_find(const char* var_name) {
	size_t len = core2_shell_cvar_count();

	for (size_t i = 0; i < len; i++) {
		core2_shell_cvar_t* cvar = core2_shell_cvar_get(i);

		if (!strcmp(cvar->name, var_name))
			return cvar;
	}

	return NULL;
}

bool core2_shell_cvar_tostring(core2_shell_cvar_t* cvar, char* buf) {
	if (cvar == NULL) {
		eprintf("core2_shell_cvar_tostring() - cvar is NULL\n");
		return false;
	}

	if (buf == NULL) {
		eprintf("core2_shell_cvar_tostring() - buf is NULL\n");
		return false;
	}

	switch (cvar->var_type) {
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

void Cmd_TokenizeString(const char* text_in, tokenize_info_t* tok_info) {
	dprintf("Cmd_TokenizeString(\"%s\")\n", text_in);

	const char* text;
	char* textOut;

	// clear previous args
	tok_info->cmd_argc = 0;

	if (!text_in) {
		return;
	}

	core2_strncpyz(tok_info->cmd_cmd, text_in, sizeof(tok_info->cmd_cmd));

	text = text_in;
	textOut = tok_info->cmd_tokenized;

	while (1) {
		if (tok_info->cmd_argc == MAX_STRING_TOKENS) {
			return; // this is usually something malicious
		}

		while (1) {
			// skip whitespace
			while (*text && *text <= ' ') {
				text++;
			}
			if (!*text) {
				return; // all tokens parsed
			}

			// skip // comments
			if (text[0] == '/' && text[1] == '/') {
				return; // all tokens parsed
			}

			// skip /* */ comments
			if (text[0] == '/' && text[1] == '*') {
				while (*text && (text[0] != '*' || text[1] != '/')) {
					text++;
				}
				if (!*text) {
					return; // all tokens parsed
				}
				text += 2;
			} else {
				break; // we are ready to parse a token
			}
		}

		// handle quoted strings
		// NOTE TTimo this doesn't handle \" escaping
		if (*text == '"') {
			tok_info->cmd_argv[tok_info->cmd_argc] = textOut;
			tok_info->cmd_argc++;
			text++;
			while (*text && *text != '"') {
				*textOut++ = *text++;
			}
			*textOut++ = 0;
			if (!*text) {
				return; // all tokens parsed
			}
			text++;
			continue;
		}

		// regular token
		tok_info->cmd_argv[tok_info->cmd_argc] = textOut;
		tok_info->cmd_argc++;

		// skip until whitespace, quote, or command
		while (*text > ' ') {
			if (text[0] == '"') {
				break;
			}

			if (text[0] == '/' && text[1] == '/') {
				break;
			}

			// skip /* */ comments
			if (text[0] == '/' && text[1] == '*') {
				break;
			}

			*textOut++ = *text++;
		}

		*textOut++ = 0;

		if (!*text) {
			return; // all tokens parsed
		}
	}
}

bool core2_shell_invoke(const char* full_command, core2_shell_func_params_t* params) {
	dprintf("core2_shell_invoke(\"%s\")\n", full_command);

	tokenize_info_t* tok_info = (tokenize_info_t*)core2_malloc(sizeof(tokenize_info_t));
	Cmd_TokenizeString(full_command, tok_info);
	dprintf("argc %d\n", tok_info->cmd_argc);

	// params->tok_info = &tok_info;

	if (tok_info->cmd_argc == 0) {
		core2_free(tok_info);
		return false;
	}

	/*for (size_t i = 0; i < tok_info.cmd_argc; i++)
	{
		dprintf("argc(%d) = %s\n", i, tok_info.cmd_argv[i]);
	}*/

	const char* func_name = tok_info->cmd_argv[0]; // TODO: Parse properly
	dprintf("invoking %s\n", func_name);

	size_t cmd_count = core2_shell_cmd_count();
	for (size_t i = 0; i < cmd_count; i++) {
		core2_shell_cmd_t* cmd = core2_shell_cmd_get(i);

		if (cmd->name != NULL && !strcmp(cmd->name, func_name)) {
			dprintf(">> Executing '%s'\n", func_name);

			if (cmd->func == NULL)
				eprintf("cmd->func is NULL\n");
			else
				cmd->func(params, tok_info->cmd_argc, tok_info->cmd_argv);

			//core2_free(tok_info);
			return true;
		}
	}

	core2_free(tok_info);
	return false;
}

void core2_shell_save_cvars() {
	int len = core2_shell_cvar_count();
	for (size_t i = 0; i < len; i++) {
		core2_shell_cvar_t* cvar = core2_shell_cvar_get(i);
		// store
	}
}

void core2_shell_load_cvars() {
	int len = core2_shell_cvar_count();
	for (size_t i = 0; i < len; i++) {
		core2_shell_cvar_t* cvar = core2_shell_cvar_get(i);
		// load
	}
}

//================ Console functions =========================================================

void shell_help(core2_shell_func_params_t* params, int argc, char** argv) {
	dprintf("shell_help!\n");

	size_t buf_len = 128;
	char* buf = (char*)core2_malloc(buf_len);

	size_t cmd_count = core2_shell_cmd_count();
	for (size_t i = 0; i < cmd_count; i++) {
		core2_shell_cmd_t* cmd = core2_shell_cmd_get(i);

		if (cmd->name == NULL)
			return;

		memset(buf, 0, buf_len);
		sprintf(buf, "%s @ %p\n", cmd->name, (void*)cmd->func);

		dprintf(buf);
		params->print(params, buf);
	}

	core2_free(buf);
}

void shell_list_cvar(core2_shell_func_params_t* params, int argc, char** argv) {
	dprintf("shell_list_cvar!\n");

	size_t buf_len = 128;
	char* buf = (char*)core2_malloc(buf_len);

	size_t cvar_count = core2_shell_cvar_count();
	for (size_t i = 0; i < cvar_count; i++) {
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

void shell_set(core2_shell_func_params_t* params, int argc, char** argv) {
	dprintf("shell_set!\n");

	if (argc < 3) {
		eprintf("shell_set argc < 3\n");
		return;
	}

	dprintf("set %s %s\n", argv[1], argv[2]);

	core2_shell_cvar_t* cvar = core2_shell_cvar_find(argv[1]);
	dprintf("Found: %s, %d\n", cvar->name, cvar->var_type);

	switch (cvar->var_type) {
		case CORE2_CVAR_INT32:
		{
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

void core2_shell_init() {
	dprintf("core2_shell_init()\n");

	shell_cmds = core2_array_create(sizeof(core2_shell_cmd_t));
	shell_cvars = core2_array_create(sizeof(core2_shell_cvar_t));

	core2_shell_register("help", shell_help);
	core2_shell_register("list_cvar", shell_list_cvar);
	core2_shell_register("set", shell_set);
}
