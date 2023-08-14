#include <core2.h>

#include <dirent.h>
#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"

#undef ESP_LOGI
#define ESP_LOGI(a, ...)      \
    do                        \
    {                         \
        dprintf(__VA_ARGS__); \
        dprintf("\n");        \
    } while (0)

FILE *core2_file_open(const char *filename, const char *type)
{
    FILE *f = NULL;

    if (type == NULL)
    {
        dprintf("core2_file_open(\"%s\", \"w\")", filename);
        f = fopen(filename, "w");
    }
    else
    {
        dprintf("core2_file_open(\"%s\", \"%s\")", filename, type);
        f = fopen(filename, type);
    }
    dprintf(" - %p\n", (void *)f);
    return f;
}

bool core2_file_close(FILE *f)
{
    dprintf("core2_file_close(%p)\n", (void *)f);
    if (!fclose(f))
        return true;

    return false;
}

bool core2_file_move(const char *oldf, const char *newf)
{
    if (rename(oldf, newf) == -1)
        return false;

    return true;
}

bool core2_file_write(const char *filename, const char *data, size_t len)
{
    dprintf("core2_write_file(\"%s\") - ", filename);

    FILE *f = fopen(filename, "w");
    if (f == NULL)
    {
        dprintf("FAIL\n");
        return false;
    }

    fwrite(data, 1, len, f);
    fclose(f);

    dprintf("OK\n");
    return true;
}

bool core2_file_append(const char *filename, const char *data, size_t len)
{
    dprintf("core2_write_append(\"%s\") - ", filename);

    FILE *f = fopen(filename, "a");
    if (f == NULL)
    {
        dprintf("FAIL\n");
        return false;
    }

    fwrite(data, 1, len, f);
    fclose(f);

    dprintf("OK\n");
    return true;
}

bool core2_file_mkdir(const char *dirname, mode_t mode)
{
    if (mode == 0)
        mode = 0777;

    dprintf("core2_file_mkdir(\"%s\", %o) - ", dirname, mode);

    if (mkdir(dirname, mode) == -1)
    {
        dprintf("FAIL\n");
        return false;
    }

    dprintf("OK\n");
    return true;
}

void core2_file_list(const char *dirname)
{
    struct dirent *dir;
    DIR *d = opendir(dirname);

    if (d)
    {
        while ((dir = readdir(d)) != NULL)
        {
            dprintf("FOUND: %s\n", dir->d_name);
        }

        closedir(d);
    }
}

bool core2_filesystem_init(sdmmc_host_t *host, int CS)
{
    dprintf("core2_filesystem_init()\n");
    const char mount_point[] = "/sd";

    esp_vfs_fat_sdmmc_mount_config_t mount_config = {.format_if_mount_failed = false, .max_files = 5, .allocation_unit_size = 16 * 1024};

    // This initializes the slot without card detect (CD) and write protect (WP) signals.
    // Modify slot_config.gpio_cd and slot_config.gpio_wp if your board has these signals.
    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = (gpio_num_t)CS;
    slot_config.host_id = (spi_host_device_t)host->slot;

    dprintf("core2_filesystem_init() - Mounting filesystem\n");
    sdmmc_card_t *card;
    esp_err_t ret = esp_vfs_fat_sdspi_mount(mount_point, host, &slot_config, &mount_config, &card);

    if (ret != ESP_OK)
    {
        if (ret == ESP_FAIL)
            dprintf("Failed to mount filesystem. If you want the card to be formatted, set the CONFIG_EXAMPLE_FORMAT_IF_MOUNT_FAILED menuconfig option\n");
        else
            dprintf("Failed to initialize the card (%s). Make sure SD card lines have pull-up resistors in place\n", esp_err_to_name(ret));

        return false;
    }

    dprintf("core2_filesystem_init() - Filesystem mounted @ \"%s\"\n", mount_point);
    sdmmc_card_print_info(stdout, card);

    dprintf("core2_filesystem_init() - Creating folders\n");
    core2_file_mkdir("/sd/logs");       // Log directory
    core2_file_mkdir("/sd/processing"); // Temporary processing directory, files which have not been uploaded?

    return true;
}
