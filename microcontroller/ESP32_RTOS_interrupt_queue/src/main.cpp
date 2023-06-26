
#include <Arduino.h>
#include <Mcp320x.h>
#include <SPI.h>

#define PIN_SWITCH (gpio_num_t)15  // !!!

QueueHandle_t interruptQueue;

#define ADC_VREF 3311   // 3.3V Vref
#define ADC_CLK 1600000 // 1600000  // SPI clock 1.6MHz

MCP3201 adc(ADC_VREF, 4);
MCP3201 adc1(ADC_VREF, 5);

float voltage_drops;
float naponi[200];  // marica za zapis padova napona u interruptu

static void IRAM_ATTR gpio_isr_handler(void *args)  // definicija ISR
{
  int pinNumber = (int)args;
  xQueueSendFromISR(interruptQueue, &pinNumber,NULL);

}

void buttonPushedTask (void *params)   // task koji se pokreće nakon aktivacije ISR
{
  int pinNumber,count = 0;
  while(true)
  {
    if(xQueueReceive(interruptQueue, &pinNumber,portMAX_DELAY))
    {
      gpio_num_t gpinNumber = (gpio_num_t)pinNumber;

      //disable the interrupt
      gpio_isr_handler_remove(gpinNumber);

      //wait some time while we check for the pin to be released
      do
      {
        vTaskDelay(5 / portTICK_PERIOD_MS);
      } while (gpio_get_level(gpinNumber) == (gpio_num_t)0);



      printf ("GPIO %d was pressed %d times. The state is %d \n", pinNumber,count++,gpio_get_level(gpinNumber));

       digitalWrite(13,HIGH);

       for(int i = 0; i < 200; i++){
        float minnapon = 35.0;
          for (int j = 0; j<20;j++){
            uint16_t raw1 = adc1.read(MCP3201::Channel::SINGLE_0); 
            uint16_t val1 = adc1.toAnalog(raw1);
            float naponj = val1 * 9.215 / 1000 ;
            delay(1);
            if (naponj < minnapon) minnapon = naponj;
          }
          naponi[i] =int (minnapon*100);
       }
        
       // vTaskDelay(1000 / portTICK_PERIOD_MS);

        for (int i = 0 ; i < 200; i++){
        // printf(" %f , ", naponi[i]);
        Serial.print(int(naponi[i]));
        Serial.print(",");
         }
        printf("\n");
      
       digitalWrite(13,LOW);

      gpio_isr_handler_add(gpinNumber,gpio_isr_handler,(void *)pinNumber);
    }

  }

}

void setup() {
   // configure PIN mode
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(13,OUTPUT);

  // set initial PIN state
  digitalWrite(4, HIGH);
  digitalWrite(5, HIGH);


   // initialize SPI interface for MCP3208 & LCD
  SPISettings settings(ADC_CLK, MSBFIRST, SPI_MODE0);
  SPI.begin();
  SPI.beginTransaction(settings);

   

  Serial.begin(115200);
  // definicija GPIO kao ISR
  gpio_set_direction(PIN_SWITCH, GPIO_MODE_INPUT);
  gpio_pulldown_en(PIN_SWITCH);
  gpio_pullup_dis(PIN_SWITCH);
  gpio_set_intr_type(PIN_SWITCH, GPIO_INTR_HIGH_LEVEL);      //GPIO_INTR_NEGEDGE);

 

  interruptQueue = xQueueCreate(1,sizeof(int));  // postavljen queue za interrupt
  xTaskCreate(buttonPushedTask,"button pushed",2048,NULL,1,NULL);  // task koji se izvršava po interruptu 

  gpio_install_isr_service(0);  // instalacija ISR-a
  gpio_isr_handler_add(PIN_SWITCH,gpio_isr_handler,(void *)PIN_SWITCH);  // dodjela ISR
}

void loop() {
          //uint16_t raw = adc.read(MCP3201::Channel::SINGLE_0);
          uint16_t raw1 = adc1.read(MCP3201::Channel::SINGLE_0); 

          //uint16_t val = adc.toAnalog(raw);
          uint16_t val1 = adc1.toAnalog(raw1);

          voltage_drops = val1 * 9.215 / 1000 ;
        //float voltage_drops1 = val1 * 9.215 / 1000 ;
          
         printf("Voltage drops= %.2f \n ", voltage_drops); //,voltage_drops1);
         vTaskDelay(5000 / portTICK_PERIOD_MS);
}