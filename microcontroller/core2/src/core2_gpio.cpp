#include <core2.h>
#include <driver/gpio.h>

xQueueHandle q;

static void IRAM_ATTR gpio_interrupt_handler(void *args)
{
    int num = 1;
    core2_queue_send(q, &num);
}

bool core2_gpio_init()
{
    gpio_num_t INPUT_PIN = GPIO_NUM_0;
    q = core2_queue_create(10, sizeof(int));

    gpio_pad_select_gpio(INPUT_PIN);
    gpio_set_direction(INPUT_PIN, GPIO_MODE_INPUT);
    gpio_pulldown_en(INPUT_PIN);
    gpio_pullup_dis(INPUT_PIN);
    gpio_set_intr_type(INPUT_PIN, GPIO_INTR_NEGEDGE);

    gpio_install_isr_service(0);
    gpio_isr_handler_add(INPUT_PIN, gpio_interrupt_handler, (void *)INPUT_PIN);

    printf("ISR installation OK\n");

    int pressed_cnt = 0;

    while (true)
    {
        int num = 0;

        if (core2_queue_receive(q, &num))
        {
            pressed_cnt++;
            printf("Button pressed %d times!\n", pressed_cnt);
        }

        vTaskDelay(pdMS_TO_TICKS(50));
    }

    return true;
}