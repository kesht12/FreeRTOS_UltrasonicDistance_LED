#include <HCSR04.h> //gamegine

#if CONFIG_FREERTOS_UNICORE
static const BaseType_t app_cpu = 0;
#else
static const BaseType_t app_cpu = 1;
#endif

HCSR04 hc(10, 46); // (trig pin, echo pin) --> can be changed

static float dist_received; //distance read by ultrasonic distance sensor

//software timer 
static const TickType_t lcd_update_time = 1000 / portTICK_PERIOD_MS; //trigger timer every 1 second
static TimerHandle_t lcd_timer = NULL; //initialize timer handle


static SemaphoreHandle_t mutex; //create mutex

static float average_distance = 0; //initialize average distance
static float distances[5]; //array of 5 distances - will be averaged and printed
static int average_dist_index = 0; //index variable for distances array

static int upper_bound = 55; //upper bound distance --> can be changed

static QueueHandle_t msg_queue_temp = NULL; //create queue
static const uint8_t msg_queue_len = 5; //length of the queue

//Initialize task handles
static TaskHandle_t task_1 = NULL;
static TaskHandle_t task_2 = NULL;
static TaskHandle_t task_3 = NULL;


//Task 1: reads new distance every 500 ms and puts that value into a FreeRTOS Queue. 
//The average distance value over the last five values is also calculated and protected using a mutex.
void readDistandQueue(void *parameters){
  while (1){
    dist_received = hc.dist();
    if (xSemaphoreTake(mutex, portMAX_DELAY) == pdTRUE){ //attempt to acquire mutex
      distances[average_dist_index] = dist_received;
      average_dist_index = (average_dist_index + 1) % 5; //increment index and use mod so it wraps around
      float sum = 0;
      for (int i = 0; i < 5; i++){
        sum += distances[i];
      }
      average_distance = sum/5.0; //store average distance in average_distance variable
      xSemaphoreGive(mutex); //release mutex
    }
    if (xQueueSend(msg_queue_temp, (void *)&dist_received, 0) != pdTRUE){ //if Queue is not full, send value
      Serial.println("QUEUE FULL");
    }
    vTaskDelay(500 / portTICK_PERIOD_MS); //delay for 500 ms
  }
}

//Software Timer callback function: Prints the average distance every second once mutex is succesfully acquired. 
void printDist(TimerHandle_t xTimer){
  float temp_buffer;
  float average_dist;
  if (xSemaphoreTake(mutex, portMAX_DELAY) == pdTRUE){ //attempt to acquire mutex
    average_dist = average_distance;
    xSemaphoreGive(mutex); //release mutex
  }
  Serial.print("Average: ");
  Serial.print(average_dist); //print average distance
  Serial.println();
}

//min distance: 2 cm
//max distance: 400 cm
//Task 2: receives distance readings from queue and uses this distance to calculate a proportional brightness. 
//The brightness of LED changes proportionally with distance. When the distance exceeds an upper bound, the LED changes from green to red with a set brightness
void pwmLED(void *parameters){
  float temp_buffer;
  while (1){
    if (xQueueReceive(msg_queue_temp, (void *)&temp_buffer, 0) == pdTRUE){
      //Serial.println(temp_buffer);
      float brightness = 255 * ((temp_buffer - 2.0) / (400.0));
      if (temp_buffer > upper_bound){
        rgbLedWrite(38, 150, 0, 0); //hard set LED to red to indicate value is above threshold
      }
      else{
        rgbLedWrite(38, 0, brightness, 0); //set to green with brightness as set in the variable
      }
    }
  }
}

void loop() {

}

void setup() {
  Serial.begin(115200);

  //initialize mutex
  mutex = xSemaphoreCreateMutex();
  //initialize distances array
  for (int i = 0; i < 5; i++) {
      distances[i] = 0;
  }
  //initialize queue
  msg_queue_temp = xQueueCreate(msg_queue_len, sizeof(float));

  //set up software timer 
  lcd_timer = xTimerCreate(
                      "LCD timer",     // timer name
                      lcd_update_time,            // timer period (5s)
                      pdTRUE,              // Auto-reload
                      (void *)0,            // Timer ID
                      printDist);  // Callback function

  xTimerStart(lcd_timer, 0); //start timer

  //Create Tasks
  //Task 1: reads new distance every 500 ms and puts that value into a FreeRTOS Queue. 
  //The average distance value over the last five values is also calculated and protected using a mutex.
  //Task 2: receives distance readings from queue and uses this distance to calculate a proportional brightness. 
  //The brightness of LED changes proportionally with distance. When the distance exceeds an upper bound, the LED changes from green to red with a set brightness
  xTaskCreatePinnedToCore(
    readDistandQueue,            // Task function
    "print from Queue",         // Task name
    4096,                 // Stack size increased to 4096
    NULL,                 // Parameter
    1,  
    &task_1,                  // Task priority                 // Task handle
    app_cpu               // Run on one core (or tskNO_AFFINITY)
  );

  xTaskCreatePinnedToCore(
    pwmLED,            // Task function
    "pwm LED",         // Task name
    4096,                 // Stack size increased to 4096
    NULL,                 // Parameter
    1,  
    &task_2,                  // Task priority                 // Task handle
    app_cpu               // Run on one core (or tskNO_AFFINITY)
  );

}

