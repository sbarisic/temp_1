#include <core2.h>

#include <dirent.h>
#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"

// #define SOC_SDMMC_USE_GPIO_MATRIX
#include "driver/sdmmc_host.h"

// Pin assignments can be set in menuconfig, see "SD SPI Example Configuration" menu.
// You can also change the pin assignments here by changing the following 4 lines.
#define PIN_NUM_MISO GPIO_NUM_35 // GPIO_NUM_2
#define PIN_NUM_MOSI GPIO_NUM_23 // GPIO_NUM_15
#define PIN_NUM_CLK  GPIO_NUM_32 // GPIO_NUM_14
#define PIN_NUM_CS   GPIO_NUM_25 // GPIO_NUM_13

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

bool core2_filesystem_init()
{
    dprintf("core2_filesystem_init() - Initializing SD card, using SPI\n");

    esp_err_t ret;
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {.format_if_mount_failed = false, .max_files = 5, .allocation_unit_size = 16 * 1024};
    sdmmc_card_t *card;

    const char mount_point[] = "/sd";

    sdmmc_host_t host = SDSPI_HOST_DEFAULT();

    spi_bus_config_t bus_cfg = {
        .mosi_io_num = PIN_NUM_MOSI,
        .miso_io_num = PIN_NUM_MISO,
        .sclk_io_num = PIN_NUM_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4000,
    };

    ret = spi_bus_initialize((spi_host_device_t)host.slot, &bus_cfg, SDSPI_DEFAULT_DMA);

    if (ret != ESP_OK)
    {
        dprintf("core2_filesystem_init() - Failed to initialize bus\n");
        return false;
    }

    // This initializes the slot without card detect (CD) and write protect (WP) signals.
    // Modify slot_config.gpio_cd and slot_config.gpio_wp if your board has these signals.
    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = PIN_NUM_CS;
    slot_config.host_id = (spi_host_device_t)host.slot;

    dprintf("core2_filesystem_init() - Mounting filesystem\n");
    ret = esp_vfs_fat_sdspi_mount(mount_point, &host, &slot_config, &mount_config, &card);

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
