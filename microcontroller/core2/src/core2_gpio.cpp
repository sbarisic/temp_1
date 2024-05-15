#include <core2.h>
#include <driver/adc.h>
#include <driver/gpio.h>

xQueueHandle q_gpio0 = NULL;
volatile bool GPIO_INT0_ENABLED;

bool core2_gpio_enable_interrupt0(bool enable)
{
    bool last_state = GPIO_INT0_ENABLED;
    GPIO_INT0_ENABLED = enable;

    return last_state;
}

bool core2_gpio_get_interrupt0()
{
    if (q_gpio0 == NULL)
        return false;

    int num;

    if (core2_queue_receive(q_gpio0, &num))
        return true;

    return false;
}

bool core2_gpio_set_interrupt0()
{
    if (q_gpio0 == NULL)
        return false;

    int num = 1;
    core2_queue_send(q_gpio0, &num);

    return true;
}

void core2_gpio_clear_interrupt0()
{
    if (q_gpio0 == NULL)
        return;

    core2_queue_reset(q_gpio0);
}

static void IRAM_ATTR gpio_interrupt_handler(void *args)
{
    gpio_num_t INT_PIN = (gpio_num_t)(int)args;

    if (INT_PIN == CORE2_GPIO_INT0_PIN)
    {
        if (GPIO_INT0_ENABLED)
            core2_gpio_set_interrupt0();
    }
}

QueueHandle_t CreateInterruptQueue()
{
    QueueHandle_t q = core2_queue_create(10, sizeof(int));
    return q;
}

void core2_gpio_set_input(gpio_num_t pin, core2_gpio_mode_t mode)
{
    gpio_pad_select_gpio(pin);
    gpio_intr_disable(pin);

    gpio_set_direction(pin, GPIO_MODE_INPUT);

    gpio_pulldown_dis(pin);
    gpio_pullup_dis(pin);

    switch (mode)
    {
    case CORE2_GPIO_MODE_NONE:
        break;

    case CORE2_GPIO_MODE_PULLDOWN:
        gpio_pulldown_en(pin);
        break;

    case CORE2_GPIO_MODE_PULLUP:
        gpio_pullup_en(pin);
        break;

    default:
        eprintf("core2_gpio_set_input(%d, %d) - Unknown out mode %d\n", pin, mode, mode);
        break;
    }
}

void core2_gpio_set_output(gpio_num_t pin, core2_gpio_mode_t mode)
{
    gpio_pad_select_gpio(pin);
    gpio_intr_disable(pin);
    gpio_set_direction(pin, GPIO_MODE_OUTPUT);

    gpio_pulldown_dis(pin);
    gpio_pullup_dis(pin);

    switch (mode)
    {
    case CORE2_GPIO_MODE_NONE:
        break;

    case CORE2_GPIO_MODE_PULLDOWN:
        gpio_pulldown_en(pin);
        break;

    case CORE2_GPIO_MODE_PULLUP:
        gpio_pullup_en(pin);
        break;

    default:
        eprintf("core2_gpio_set_output(%d, %d) - Unknown out mode %d\n", pin, mode, mode);
        break;
    }
}

bool core2_gpio_read(gpio_num_t pin)
{
    int lvl = gpio_get_level(pin);
    // dprintf("core2_gpio_read(%d) = %d\n", pin, lvl);

    return lvl > 0;
}

void core2_gpio_write(gpio_num_t pin, int state)
{
    gpio_set_level(pin, state);
}

void CreateInterrupt(gpio_num_t INPUT_PIN)
{
    gpio_pad_select_gpio(INPUT_PIN);
    gpio_set_direction(INPUT_PIN, GPIO_MODE_INPUT);

    gpio_pulldown_en(INPUT_PIN);
    gpio_pullup_dis(INPUT_PIN);

    gpio_set_intr_type(INPUT_PIN, GPIO_INTR_POSEDGE);

    gpio_isr_handler_add(INPUT_PIN, gpio_interrupt_handler, (void *)INPUT_PIN);
}

int core2_gpio_hall_read()
{
#if CONFIG_IDF_TARGET_ESP32
    esp_err_t err = adc1_config_width(ADC_WIDTH_12Bit);

    if (err != ESP_OK)
        return 0;

    int sleep_ms = 1;
    size_t samples = 10;
    int sample_sum = 0;

    for (size_t i = 0; i < samples; i++)
    {
        sample_sum += hall_sensor_read();

        if (i < samples - 1)
            vTaskDelay(pdMS_TO_TICKS(5));
    }

    return sample_sum / samples;
#endif

    return 0;
}

bool core2_gpio_init()
{
    dprintf("core2_gpio_init()\n");

    // Install global interrupt handler routine
    esp_err_t err = gpio_install_isr_service(0);

    char buf[30];
    core2_err_tostr(err, buf);
    dprintf("gpio_install_isr_service(0) = %s\n", buf);

    q_gpio0 = CreateInterruptQueue();
    CreateInterrupt(CORE2_GPIO_INT0_PIN);
    return true;
}