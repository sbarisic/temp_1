#include <core2.h>

#include <dirent.h>
#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"

bool core2_spi_create(sdmmc_host_t* host, int MOSI, int MISO, int CLK)
{
    dprintf("core2_spi_create(MOSI = %d, MISO = %d, CLK = %d)\n", MOSI, MISO, CLK);

    spi_bus_config_t bus_cfg = {
        .mosi_io_num = MOSI,
        .miso_io_num = MISO,
        .sclk_io_num = CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4000,
    };

    esp_err_t ret = spi_bus_initialize((spi_host_device_t)host->slot, &bus_cfg, SDSPI_DEFAULT_DMA);

    if (ret != ESP_OK)
    {
        dprintf("core2_spi_create() - Failed to initialize bus\n");
        return false;
    }

    return true;
}
