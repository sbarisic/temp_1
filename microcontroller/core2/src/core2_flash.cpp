#include <core2.h>

#include <string.h>
#include <sys/stat.h>
#include <sys/unistd.h>

#include <nvs.h>
#include <nvs_flash.h>

#define STORAGE_NAMESPACE "core2store"

void core2_flash_cvar_store(core2_shell_cvar_t *cvar)
{
    nvs_handle_t nvs_handle;

    esp_err_t err = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &nvs_handle);
    if (err != ESP_OK)
    {
        eprintf("core2_flash_cvar_store() nvs_open() - %d\n", err);
        return;
    }

    switch (cvar->var_type)
    {
    case CORE2_CVAR_STRING:
        err = nvs_set_str(nvs_handle, cvar->name, (const char *)cvar->var_ptr);
        break;

    case CORE2_CVAR_INT32:
        err = nvs_set_i32(nvs_handle, cvar->name, (int32_t)cvar->var_ptr);
        break;

    default:
        break;
    }

    if (err != ESP_OK)
        eprintf("core2_flash_cvar_store() nvs_set_*(\"%s\") - %d\n", cvar->name, err);

    nvs_commit(nvs_handle);
    nvs_close(nvs_handle);
}

void core2_flash_cvar_load(core2_shell_cvar_t *cvar)
{
    nvs_handle_t nvs_handle;

    esp_err_t err = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &nvs_handle);
    if (err != ESP_OK)
    {
        eprintf("core2_flash_cvar_load() nvs_open() - %d\n", err);
        return;
    }

    switch (cvar->var_type)
    {
    case CORE2_CVAR_STRING:
        eprintf("core2_flash_cvar_load() for strings not implemented!");
        break;

    case CORE2_CVAR_INT32:
        err = nvs_get_i32(nvs_handle, cvar->name, (int32_t *)(&(cvar->var_ptr)));
        break;

    default:
        break;
    }

    if (err != ESP_OK)
        eprintf("core2_flash_cvar_load() nvs_get_*(\"%s\") - %d\n", cvar->name, err);

    nvs_close(nvs_handle);
}

bool core2_flash_init()
{
    dprintf("core2_flash_init()\n");

    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        dprintf("Doing nvs_flash_erase()\n");
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    return true;
}
