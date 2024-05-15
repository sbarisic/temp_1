#include <core2.h>

#if defined(CORE2_DOOR_CONTROLLER)

const core2_gpio_mode_t gpio_input_mode = CORE2_GPIO_MODE_PULLDOWN;
const gpio_num_t PIN_A = GPIO_NUM_34;
const gpio_num_t PIN_B = GPIO_NUM_23;
const gpio_num_t PIN_C = GPIO_NUM_35;
const gpio_num_t PIN_D = GPIO_NUM_33;

const gpio_num_t PIN_DIR_A = GPIO_NUM_27;
const gpio_num_t PIN_DIR_B = GPIO_NUM_25;

const core2_gpio_mode_t gpio_output_mode = CORE2_GPIO_MODE_PULLDOWN;
const int32_t OUT_LEVEL_ACTIVE = HIGH;
const int32_t OUT_LEVEL_INACTIVE = LOW;

int32_t time_dir_a;
int32_t time_dir_b;

bool debounce_read(gpio_num_t pin)
{
    bool val = core2_gpio_read(pin);

    if (val)
    {
        for (size_t i = 0; i < 3; i++)
        {
            core2_sleep(8);
            val = core2_gpio_read(pin);

            if (!val)
                return false;
        }

        return true;
    }

    return false;
}

void set_dir_a()
{
    if (core2_clock_seconds_since(time_dir_a) > 1)
    {
        time_dir_a = core2_clock_bootseconds();
        core2_gpio_write(PIN_DIR_A, OUT_LEVEL_ACTIVE);
    }
}

void set_dir_b()
{
    if (core2_clock_seconds_since(time_dir_b) > 1)
    {
        time_dir_b = core2_clock_bootseconds();
        core2_gpio_write(PIN_DIR_B, OUT_LEVEL_ACTIVE);
    }
}

void set_no_dir()
{
    core2_gpio_write(PIN_DIR_A, OUT_LEVEL_INACTIVE);
    core2_gpio_write(PIN_DIR_B, OUT_LEVEL_INACTIVE);
    time_dir_a = core2_clock_bootseconds();
    time_dir_b = core2_clock_bootseconds();
}

void copy_remote_buttons_to_out()
{
    bool A = debounce_read(PIN_A);
    bool B = debounce_read(PIN_B);

    if (A && B)
    {
        set_no_dir();
    }
    else if (A && !B)
    {
        set_dir_a();
    }
    else if (!A && B)
    {
        set_dir_b();
    }
    else
    {
        set_no_dir();
    }
}

void core2_door_controller_setup()
{
    core2_gpio_set_output(PIN_DIR_A, gpio_output_mode);
    core2_gpio_write(PIN_DIR_A, OUT_LEVEL_INACTIVE);

    core2_gpio_set_output(PIN_DIR_B, gpio_output_mode);
    core2_gpio_write(PIN_DIR_B, OUT_LEVEL_INACTIVE);

    core2_gpio_set_input(PIN_A, gpio_input_mode);
    core2_gpio_set_input(PIN_B, gpio_input_mode);
    core2_gpio_set_input(PIN_C, gpio_input_mode);
    core2_gpio_set_input(PIN_D, gpio_input_mode);
}

void core2_door_controller_main()
{
    printf("Hello World!\n");
    int32_t wait_delay = 100;

    for (;;)
    {
        copy_remote_buttons_to_out();
        core2_sleep(wait_delay);
    }
}

#endif