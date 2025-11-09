#include <max6675.h>

#define PWM_FREQUENCY 5000
#define PWM_RESOLUTION 9
#define GROUND 1
#define LED 21
#define THERMO_CS 23
#define THERMO_SO 19
#define THERMO_SCK 5
#define QUEUE_SIZE 5

#define MINTEMP 20
#define MAXTEMP 30

MAX6675 thermocouple(THERMO_SCK, THERMO_CS, THERMO_SO);

QueueHandle_t ThermoQueue = NULL;

void ThermoTask(void *parameter)
{
  for(;;){
    float temperature = thermocouple.readCelsius();
    xQueueSend(ThermoQueue, &temperature, portMAX_DELAY);
    Serial.printf("Temperature of %f was sent\n", temperature);
    vTaskDelay(500/portTICK_RATE_MS);
  }
}

void LEDBrightnessTask(void *parameter)
{
  for(;;)
  {
    float temperature;
    if (xQueueReceive(ThermoQueue, &temperature, portMAX_DELAY)){

      uint16_t brightness; //0-512

      if (temperature <= MINTEMP){
        
        brightness = 512;
        Serial.println("Lower limit reached");

      }
      else if (temperature >= MAXTEMP){

        brightness = 0;
        Serial.println("Upper limit reached");
      
      } else {
      
        uint16_t Max_brightness = 1 << PWM_RESOLUTION;
        brightness = 512 - (int)((temperature - MINTEMP)*Max_brightness/(MAXTEMP-MINTEMP));
        Serial.printf("Brightness is %u\n", brightness);
      
      }

      ledcWrite(LED, brightness);
    }
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

  ledcAttach(LED, PWM_FREQUENCY, PWM_RESOLUTION);

  ThermoQueue = xQueueCreate(QUEUE_SIZE, sizeof(float));
  if(ThermoQueue == NULL){
    Serial.println("Failed to create a queue");
    while (1);
  }

  xTaskCreatePinnedToCore(
    ThermoTask, 
    "ThermoTask", 
    3000, 
    NULL, 
    1, 
    NULL,
    1
  );

  xTaskCreatePinnedToCore(
    LEDBrightnessTask, 
    "LEDBrightnessTask", 
    3000, 
    NULL, 
    1, 
    NULL,
    1
  );

}

void loop() {

}
