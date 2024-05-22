#include <Wire.h>
#include <core2.h>
#include <esp_http_server.h>

volatile bool buzzer_enable = false;
volatile bool leds_initialized = false;
volatile float Volts[200];

bool send_data_to_server(char *json_buffer, size_t json_len)
{
    dprintf("Sending data - ");

    if (!core2_web_json_post("https://demo.sbarisic.com/deviceaccess", json_buffer, strlen(json_buffer)))
    {
        dprintf("Fail\n");
        return false;
    }

    dprintf("OK\n");
    return true;
}

void led_enable(bool r, bool g, bool b)
{
    if (!leds_initialized)
    {
        pinMode(13, OUTPUT);
        pinMode(14, OUTPUT);
        pinMode(12, OUTPUT);
    }

    digitalWrite(14, r ? HIGH : LOW);
    digitalWrite(12, g ? HIGH : LOW);
    digitalWrite(13, b ? HIGH : LOW);
}

void trigger_solenoid()
{
    dprintf("trigger_solenoid()");

    digitalWrite(GPIO_NUM_27, HIGH);
    vTaskDelay(pdMS_TO_TICKS(300));
    digitalWrite(GPIO_NUM_27, LOW);
}

void read_temp_pressure(float *Temp, float *Press)
{
    uint8_t id = 0x28;                   // i2c address
    uint8_t data[7];                     // holds output data
    uint8_t cmd[3] = {0xAA, 0x00, 0x00}; // command to be sent

    double press_counts = 0;     // digital pressure reading [counts]
    double temp_counts = 0;      // digital temperature reading [counts]
    double outputmax = 15099494; // output at maximum pressure [counts]
    double outputmin = 1677722;  // output at minimum pressure [counts]
    double pmax = 10;            // maximum value of pressure range [bar, psi, kPa, etc.]
    double pmin = 0;             // minimum value of pressure range [bar, psi, kPa, etc.]

    float gpressure;
    float gtemperature;

    Wire.beginTransmission(id);
    int stat = Wire.write(cmd, 3); // write command to the sensor
    stat |= Wire.endTransmission();

    core2_sleep(10);

    Wire.requestFrom(id, (uint8_t)7); // read back Sensor data 7 bytes

    for (int i = 0; i < 7; i++)
    {
        data[i] = Wire.read();
    }
    // Wire.end();

    // start read pressure & temperature
    press_counts = data[3] + data[2] * 256 + data[1] * 65536; // calculate digital pressure counts
    temp_counts = data[6] + data[5] * 256 + data[4] * 65536;  // calculate digital temperature counts
    gtemperature = (temp_counts * 200 / 16777215) - 50;       // calculate temperature in deg c

    // calculation of pressure value according to equation 2 of datasheet
    gpressure = ((press_counts - outputmin) * (pmax - pmin)) / (outputmax - outputmin) + pmin;

    *Temp = gtemperature;
    *Press = gpressure;
}

void interrupt_read_data()
{
    dprintf("interrupt_read_data()\n");

    // bool adc_lock = core2_adc_lock();

    dprintf("[!] interrupt_read_data() core2_adc_read - ");
    float v1 = 0;
    float v2 = 0;
    float cur = 0;
    core2_adc_read(&v1, &v2, &cur);
    dprintf("OK\n");

    dprintf("[!] interrupt_read_data() read_temp_pressure - ");
    float temp = 0;
    float press = 0;
    read_temp_pressure(&temp, &press);
    dprintf("OK\n");

    /*for (int i = 0; i < 200; i++)
    {
        core2_adc_read_ex(&(Volts[i]), NULL, CORE2_ADC_CH2, false);
        Volts[i] = Volts[i] * 9.215 / 1000;

        // WORKING!
        // float V1, V2;
        // core2_adc_read(&V1, &V2);
        // Volts[i] = V2;

        vTaskDelay(pdMS_TO_TICKS(15));
    }*/

    core2_json_t *json = core2_json_create();

    core2_json_add_field_string(json, "APIKey", "OoDUEAxaDLE3L+tdG2ZWmvSNJ8A5jnzh9a4r4d4XzEw="); // TODO
    core2_json_add_field_int(json, "Action", 1);
    // core2_json_add_field(json, "Volts", &Volts, (sizeof(Volts) / sizeof(*Volts)), CORE2_JSON_FLOAT_ARRAY_DEC2);

    core2_json_add_field_float(json, "ACCvoltage1", v1);
    core2_json_add_field_float(json, "ACCvoltage2", v2);
    core2_json_add_field_float(json, "ACCcurrent", cur);
    core2_json_add_field_float(json, "Tlak", temp);
    core2_json_add_field_float(json, "Temperatura", press);

    char time_now[32] = {0};
    core2_clock_time_now(time_now);
    core2_json_add_field_string(json, "LocalTime", time_now);

    char *json_buffer;
    size_t json_len;
    core2_json_serialize(json, &json_buffer, &json_len);
    core2_json_delete(json);

    // printf("======= core2_json_test =======\n");
    // printf("%s\n", json_buffer);
    // printf("===============================\n");

    /*if (!core2_file_write_timesuffix("/sd/processing/volts_%s.json", json_buffer, json_len))
    {
        // Saving to file failed, send now?
        send_data_to_server(json_buffer, json_len);
    }*/

    send_data_to_server(json_buffer, json_len);

    core2_free(json_buffer);
    // core2_adc_unlock(adc_lock);

    /*for (int i = 0; i < 200; i++)
    {
        dprintf("%.2f  ", Volts[i]);
    }*/
}

void buzzer_task(void *a)
{
    dprintf("[BUZZER] Setting up\n");

    pinMode(GPIO_NUM_25, OUTPUT);
    digitalWrite(GPIO_NUM_25, LOW);
    // ledcAttachPin(GPIO_NUM_25, 0);

    dprintf("[BUZZER] OK\n");

    while (1)
    {
        if (buzzer_enable)
        {
            tone(GPIO_NUM_25, 3800, 0);
            vTaskDelay(pdMS_TO_TICKS(1000));
            noTone(GPIO_NUM_25);
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }

    vTaskDelete(NULL);
}

void core2_buzz(int32_t len)
{
    buzzer_enable = true;
    core2_sleep(len);
    buzzer_enable = false;
}

void core2_shellcmd_get_variables(core2_shell_func_params_t *params, int argc, char **argv)
{
    core2_json_t *json = core2_json_create();
    core2_json_begin_field(json, "vars", CORE2_JSON_BEGIN_ARRAY);

    size_t count = core2_shell_cvar_count();
    for (size_t i = 0; i < count; i++)
    {
        core2_shell_cvar_t *cvar = core2_shell_cvar_get(i);

        core2_json_add(json, CORE2_JSON_BEGIN_OBJECT);
        core2_json_add_field_string(json, "variable_name", cvar->name);

        switch (cvar->var_type)
        {
        case CORE2_CVAR_STRING:
            core2_json_add_field_string(json, "value", (const char *)cvar->var_ptr);
            break;

        case CORE2_CVAR_INT32:
            core2_json_add_field_int(json, "value", (int32_t)cvar->var_ptr);
            break;

        default:
            // TODO:
            core2_json_add_field_string(json, "value", "TODO: NOT IMPLEMENTED");
            break;
        }

        core2_json_add(json, CORE2_JSON_END_OBJECT);
    }

    core2_json_add(json, CORE2_JSON_END_ARRAY);

    char *json_buffer;
    size_t json_len;
    core2_json_serialize(json, &json_buffer, &json_len);
    core2_json_delete(json);

    httpd_resp_set_type((httpd_req_t *)params->ud1, "application/json");
    params->print(params, json_buffer);

    core2_free(json_buffer);
}

void core2_main()
{
    printf("Hello World!\n");
    xTaskCreate(buzzer_task, "buzzer_task", 1024, NULL, 0, NULL);
    led_enable(false, false, true);

    Wire.begin();
    core2_buzz(250);

    // Read pin on boot
    core2_gpio_set_input(CORE2_GPIO_SETUP_BUTTON_PIN, CORE2_GPIO_MODE_PULLUP);
    bool enter_ap_mode = false;

    for (size_t i = 0; i < 200; i++)
    {
        if (core2_gpio_hall_read() > 1000)
        {
            enter_ap_mode = true;
        }

        if (!core2_gpio_read(CORE2_GPIO_SETUP_BUTTON_PIN))
        {
            enter_ap_mode = true;
        }

        core2_sleep(10);
    }

    core2_shell_cvar_register("wifi_ssid", (void *)"Test", CORE2_CVAR_STRING);
    core2_shell_cvar_register("wifi_pass", (void *)"12345678910", CORE2_CVAR_STRING);

    core2_shell_cvar_register(CORE2_CVAR_wifi_ap_ssid, (void *)"core2_wifi_devtest", CORE2_CVAR_STRING);
    core2_shell_cvar_register(CORE2_CVAR_wifi_ap_pass, (void *)"core21234", CORE2_CVAR_STRING);

    core2_shell_register("get_variables", core2_shellcmd_get_variables);
    core2_shell_load_cvars();

    if (enter_ap_mode)
    {
        dprintf("Entering AP setup mode...\n");
        led_enable(false, true, false);

        core2_wifi_ap_start();
        core2_http_start();

        for (;;)
        {
            core2_sleep(10);
        }

        return;
    }
    else
    {
        dprintf("Skipping AP mode...\n");
        led_enable(false, false, false);
    }

    // TODO: remove defaults
    core2_wifi_add_network("Barisic", "123456789");
    // core2_wifi_add_network("Tst", "123456789");

    core2_buzz(700);

    // core2_shell_func_params_t *params = core2_shell_create_default_params();

    const char *wifi_ssid = core2_shell_cvar_get_string("wifi_ssid");
    const char *wifi_pass = core2_shell_cvar_get_string("wifi_pass");

    if (wifi_ssid != NULL && wifi_pass != NULL)
    {
        dprintf("Found wifi_ssid in cvars, \"%s\"\n", wifi_ssid);
        core2_wifi_add_network(wifi_ssid, wifi_pass);
        // core2_wifi_try_connect(wifi_ssid, wifi_pass);
    }

    core2_wifi_yield_until_connected();
    dprintf("[!] Main loop\n");

    for (;;)
    {
        interrupt_read_data();
        core2_sleep(2000);

        /*if (core2_gpio_get_interrupt0())
        {
            core2_gpio_enable_interrupt0(false);

            dprintf("[INT 0]!\n");
            interrupt_read_voltage();

            core2_gpio_clear_interrupt0();
            core2_gpio_enable_interrupt0(true);
        }*/

        // interrupt_read_voltage();

        // core2_sleep(10 * 1000); // TODO: move to variable
    }
}
