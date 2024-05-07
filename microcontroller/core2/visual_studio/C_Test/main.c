#include "core2.h"

core2_shell_cvar_t cvar_testString;
core2_shell_cvar_t cvar_testString2;
core2_shell_cvar_t cvar_testInt;

void core2_shellcmd_get_variables(core2_shell_func_params_t* params, int argc, char** argv) {

	size_t count = core2_shell_cvar_count();
	for (size_t i = 0; i < count; i++) {
		core2_shell_cvar_t* cvar = core2_shell_cvar_get(i);


		switch (cvar->var_type) {
			case CORE2_CVAR_STRING:
				dprintf("%s = \"%s\"\n", cvar->name, (const char*)cvar->var_ptr);
				break;

			case CORE2_CVAR_INT32:
				dprintf("%s = %d\n", cvar->name, (int)cvar->var_ptr);
				break;

			default:
				dprintf("%s = %p\n", cvar->name, cvar->var_ptr);
				break;
		}
	}
}

void core2_print(void* self, const char* str) {
	dprintf("%s", str);
}

void main(int argc, const char** argv) {
	core2_shell_init();

	core2_shell_cvar_register(&cvar_testString, "testString", (void*)"Test string value", CORE2_CVAR_STRING);
	core2_shell_cvar_register(&cvar_testInt, "testInt", 0, CORE2_CVAR_INT32);
	core2_shell_load_cvars();

	core2_shell_register("get_variables", core2_shellcmd_get_variables);
	// core2_shell_save_cvars();

	core2_shell_func_params_t* params = (core2_shell_func_params_t*)core2_malloc(sizeof(core2_shell_func_params_t));
	params->print = core2_print;

	// core2_shell_invoke("set testString \"Hello CVar World!\"", &params);
	core2_shell_invoke("help", params);
}