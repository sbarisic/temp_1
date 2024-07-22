#include <core2.h>
#include <dirent.h>
#include <sys/stat.h>

#ifndef CORE2_WINDOWS
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include <dirent.h>
#include <string.h>
#include <sys/unistd.h>
#else
#include <shlwapi.h>
#endif

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
#ifdef CORE2_FILESYSTEM_VERBOSE_OUTPUT
		dprintf("core2_file_open(\"%s\", \"w\")", filename);
#endif

		f = fopen(filename, "w");
	}
	else
	{
#ifdef CORE2_FILESYSTEM_VERBOSE_OUTPUT
		dprintf("core2_file_open(\"%s\", \"%s\")", filename, type);
#endif

		f = fopen(filename, type);
	}

#ifdef CORE2_FILESYSTEM_VERBOSE_OUTPUT
	dprintf(" - %p\n", (void *)f);
#endif

	return f;
}

bool core2_file_close(FILE *f)
{
#ifdef CORE2_FILESYSTEM_VERBOSE_OUTPUT
	dprintf("core2_file_close(%p)\n", (void *)f);
#endif

	fflush(f);

	if (!fclose(f))
		return true;

	return false;
}

bool core2_file_delete(const char *filename)
{
#ifdef CORE2_FILESYSTEM_VERBOSE_OUTPUT
	dprintf("core2_file_delete(%s)\n", filename);
#endif

	if (remove(filename) == 0)
		return true;

	return false;
}

const char *core2_basename(const char *pth)
{
#ifndef MAX_PATH
#define MAX_PATH 128
#endif

	char *fname = (char *)core2_malloc(MAX_PATH);
	strcpy(fname, pth);

#ifdef CORE2_WINDOWS
	PathStripPathA(fname);
	return fname;
#else
	return basename(pth);
#endif
}

bool core2_file_move(const char *full_file_path, const char *new_directory)
{
#if defined(CORE2_FILESYSTEM_VERBOSE_OUTPUT) || defined(CORE2_FILESYSTEM_SIMPLE_OUTPUT)
	dprintf("core2_file_move(\"%s\") to \"%s\"\n", full_file_path, new_directory);
#endif

	const char *fname = core2_basename(full_file_path);
	char *full_dest_filename = core2_string_concat(new_directory, fname);
	core2_free((void *)fname);

#if defined(CORE2_FILESYSTEM_VERBOSE_OUTPUT)
	dprintf("core2_file_move new filename is \"%s\"\n", full_dest_filename);
#endif

	if (rename(full_file_path, full_dest_filename) == -1)
	{
#if defined(CORE2_FILESYSTEM_VERBOSE_OUTPUT) || defined(CORE2_FILESYSTEM_SIMPLE_OUTPUT)
		dprintf("core2_file_move FAILED\n");
#endif

		core2_free(full_dest_filename);
		return false;
	}

#if defined(CORE2_FILESYSTEM_VERBOSE_OUTPUT) || defined(CORE2_FILESYSTEM_SIMPLE_OUTPUT)
	dprintf("core2_file_move OK\n");
#endif

	core2_free(full_dest_filename);
	return true;
}

bool core2_file_write(const char *filename, const char *data, size_t len)
{
#if defined(CORE2_FILESYSTEM_VERBOSE_OUTPUT) || defined(CORE2_FILESYSTEM_SIMPLE_OUTPUT)
	dprintf("core2_file_write(\"%s\")\n", filename);
#endif

	FILE *f = core2_file_open(filename, "w");
	if (f == NULL)
	{
#if defined(CORE2_FILESYSTEM_VERBOSE_OUTPUT) || defined(CORE2_FILESYSTEM_SIMPLE_OUTPUT)
		dprintf("core2_file_write FAIL\n");
#endif

		return false;
	}

	fwrite(data, 1, len, f);
	core2_file_close(f);

#if defined(CORE2_FILESYSTEM_VERBOSE_OUTPUT) || defined(CORE2_FILESYSTEM_SIMPLE_OUTPUT)
	dprintf("core2_file_write OK\n");
#endif

	return true;
}

// @brief Adds current time as file name suffix
bool core2_file_write_timesuffix(const char *filename, const char *data, size_t len)
{
	dprintf("TODO! core2_file_write_timesuffix\n");

	char cur_time[21];
	core2_clock_time_fmt(cur_time, sizeof(cur_time), "%d%m%Y_%H%M%S");

	char new_filename[512];
	sprintf(new_filename, filename, cur_time);

	return core2_file_write(new_filename, data, len);
}

bool core2_file_append(const char *filename, const char *data, size_t len)
{
#if defined(CORE2_FILESYSTEM_VERBOSE_OUTPUT) || defined(CORE2_FILESYSTEM_SIMPLE_OUTPUT)
	dprintf("core2_file_append(\"%s\")\n", filename);
#endif

	FILE *f = core2_file_open(filename, "a");
	if (f == NULL)
	{
#if defined(CORE2_FILESYSTEM_VERBOSE_OUTPUT) || defined(CORE2_FILESYSTEM_SIMPLE_OUTPUT)
		dprintf("core2_file_append FAIL\n");
#endif

		return false;
	}

	fwrite(data, 1, len, f);
	core2_file_close(f);

#if defined(CORE2_FILESYSTEM_VERBOSE_OUTPUT) || defined(CORE2_FILESYSTEM_SIMPLE_OUTPUT)
	dprintf("core2_file_append OK\n");
#endif

	return true;
}

bool core2_file_mkdir(const char *dirname, mode_t mode)
{
	if (mode == 0)
		mode = 0777;

#if defined(CORE2_FILESYSTEM_VERBOSE_OUTPUT) || defined(CORE2_FILESYSTEM_SIMPLE_OUTPUT)
	dprintf("core2_file_mkdir(\"%s\", %o) - ", dirname, mode);
#endif

	struct stat st = {0};
	if (stat(dirname, &st) == -1)
	{
		int res = 0;

#ifdef CORE2_WINDOWS
		res = CreateDirectoryA(dirname, NULL) ? 1 : -1;
#else
		res = mkdir(dirname, mode);
#endif

		if (res == -1)
		{
#if defined(CORE2_FILESYSTEM_VERBOSE_OUTPUT) || defined(CORE2_FILESYSTEM_SIMPLE_OUTPUT)
			dprintf("FAIL\n");
#endif

			return false;
		}
		else
		{
#if defined(CORE2_FILESYSTEM_VERBOSE_OUTPUT) || defined(CORE2_FILESYSTEM_SIMPLE_OUTPUT)
			dprintf("OK\n");
#endif

			return true;
		}
	}
	else
	{
#if defined(CORE2_FILESYSTEM_VERBOSE_OUTPUT) || defined(CORE2_FILESYSTEM_SIMPLE_OUTPUT)
		dprintf("DIR_EXISTS\n");
#endif

		return false;
	}

	return false;
}

void core2_file_list(const char *dirname, onFileFoundFn onFileFound)
{
	struct dirent *dir;
	char buf[PATH_MAX + 1];

	DIR *d = opendir(dirname);

	if (d)
	{
		while ((dir = readdir(d)) != NULL)
		{
			memset(buf, 0, sizeof(buf));
			strcat(buf, dirname);
			strcat(buf, "/");
			strcat(buf, dir->d_name);

			dprintf("FOUND: \"%s\"; ", buf);

			const char *filename = core2_basename(buf);
			dprintf("basename = \"%s\"\n", filename);

			onFileFound(buf, filename);
			core2_free((void *)filename);
		}

		closedir(d);
	}
}

bool core2_file_exists(const char *filename)
{
#if defined(CORE2_FILESYSTEM_VERBOSE_OUTPUT) || defined(CORE2_FILESYSTEM_SIMPLE_OUTPUT)
	dprintf("core2_file_exists(\"%s\") - ", filename);
#endif

	FILE *f = core2_file_open(filename, "r");

	if (f != NULL)
	{
#if defined(CORE2_FILESYSTEM_VERBOSE_OUTPUT) || defined(CORE2_FILESYSTEM_SIMPLE_OUTPUT)
		dprintf("TRUE\n");
#endif

		core2_file_close(f);
		return true;
	}

#if defined(CORE2_FILESYSTEM_VERBOSE_OUTPUT) || defined(CORE2_FILESYSTEM_SIMPLE_OUTPUT)
	dprintf("FALSE\n");
#endif
	return false;
}

size_t core2_file_length(FILE *f)
{
	fseek(f, 0, SEEK_END);
	size_t len = ftell(f);
	fseek(f, 0, SEEK_SET);
	return len;
}

void *core2_file_read_all(const char *filename, size_t *len)
{
#if defined(CORE2_FILESYSTEM_VERBOSE_OUTPUT) || defined(CORE2_FILESYSTEM_SIMPLE_OUTPUT)
	dprintf("core2_file_read_all(\"%s\")\n", filename);
#endif

	if (core2_file_exists(filename))
	{
		FILE *f = core2_file_open(filename, "r");
		*len = core2_file_length(f);

		void *buf = core2_malloc(*len);
		if (buf == NULL)
		{
			fclose(f);
			return NULL;
		}

		fread(buf, 1, *len, f);
		fclose(f);
		return buf;
	}

	return NULL;
}

bool core2_filesystem_init(sdmmc_host_t *host, int CS)
{
	dprintf("core2_filesystem_init()\n");
	const char mount_point[] = "/sd";

#ifndef CORE2_WINDOWS
	esp_vfs_fat_sdmmc_mount_config_t mount_config = {
		.format_if_mount_failed = false, .max_files = 5, .allocation_unit_size = 16 * 1024};

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
			dprintf("Failed to mount filesystem. If you want the card to be formatted, set the "
					"CONFIG_EXAMPLE_FORMAT_IF_MOUNT_FAILED menuconfig option\n");
		else
			dprintf("Failed to initialize the card (%s). Make sure SD card lines have pull-up resistors in place\n",
					esp_err_to_name(ret));

		return false;
	}

	dprintf("core2_filesystem_init() - Filesystem mounted @ \"%s\"\n", mount_point);
	sdmmc_card_print_info(stdout, card);
#endif

	dprintf("core2_filesystem_init() - Creating folders\n");
	core2_file_mkdir("/sd/logs", 0);	   // Log directory
	core2_file_mkdir("/sd/processing", 0); // Temporary processing directory, files which have not been uploaded?
	return true;
}
