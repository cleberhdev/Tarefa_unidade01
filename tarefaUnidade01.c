#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#define BUTTON_PIN 5
#define LED_PIN_RED 11
#define LED_PIN_BLUE 12

QueueHandle_t button_queue;
SemaphoreHandle_t led_semaphore;

void setup() {
    stdio_init_all();
    gpio_init(BUTTON_PIN);
    gpio_set_dir(BUTTON_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_PIN);  
    
    gpio_init(LED_PIN_RED);
    gpio_set_dir(LED_PIN_RED, GPIO_OUT);
    gpio_put(LED_PIN_RED, 0);  
    
    gpio_init(LED_PIN_BLUE);
    gpio_set_dir(LED_PIN_BLUE, GPIO_OUT);
    gpio_put(LED_PIN_BLUE, 0);
}

void vButtonTask(void *pvParameters) {
    while(1) {
        uint8_t estado = !gpio_get(BUTTON_PIN); 
        xQueueSend(button_queue, &estado, portMAX_DELAY);
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void vProcessTask(void *pvParameters) {
    uint8_t estado;
    while(1) {
        if(xQueueReceive(button_queue, &estado, portMAX_DELAY) == pdTRUE) {
            if(estado == 1) {  
                xSemaphoreGive(led_semaphore);
            }
        }
    }
}

void vLedTask(void *pvParameters) {
    uint8_t led_state = 0;  
    while(1) {
        if(xSemaphoreTake(led_semaphore, portMAX_DELAY) == pdTRUE) {
            led_state = !led_state;  
            
            gpio_put(LED_PIN_RED, !led_state);   
            gpio_put(LED_PIN_BLUE, led_state);    
        }
    }
}

int main() {
    setup();
    
    button_queue = xQueueCreate(1, sizeof(uint8_t));
    led_semaphore = xSemaphoreCreateBinary();
    
    xTaskCreate(vButtonTask, "Button", 256, NULL, 1, NULL);
    xTaskCreate(vProcessTask, "Process", 256, NULL, 2, NULL);
    xTaskCreate(vLedTask, "LED", 256, NULL, 3, NULL);
    
    vTaskStartScheduler();
    
    while(1);
}