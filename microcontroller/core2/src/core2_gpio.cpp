#include <core2.h>
#include <driver/gpio.h>

xQueueHandle q_gpio0;

bool core2_gpio_get_interrupt0()
{
    int num;

    if (core2_queue_receive(q_gpio0, &num))
    {
        return true;
    }

    return false;
}

bool core2_gpio_set_interrupt0()
{
    int num = 1;
    core2_queue_send(q_gpio0, &num);
    return true;
}

static void IRAM_ATTR gpio_interrupt_handler(void *args)
{
    core2_gpio_set_interrupt0();
}

bool core2_gpio_init()
{
    dprintf("core2_gpio_init()\n");

    gpio_num_t INPUT_PIN = GPIO_NUM_0;
    q_gpio0 = core2_queue_create(10, sizeof(int));

    gpio_pad_select_gpio(INPUT_PIN);
    gpio_set_direction(INPUT_PIN, GPIO_MODE_INPUT);
    gpio_pulldown_en(INPUT_PIN);
    gpio_pullup_dis(INPUT_PIN);
    gpio_set_intr_type(INPUT_PIN, GPIO_INTR_POSEDGE);

    esp_err_t err = gpio_install_isr_service(0);

    char buf[30];
    core2_err_tostr(err, buf);
    dprintf("gpio_install_isr_service(0) = %s\n", buf);

    gpio_isr_handler_add(INPUT_PIN, gpio_interrupt_handler, (void *)INPUT_PIN);

    return true;
}