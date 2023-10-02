#include "../../include/capacitivo_tool.h"
#include "../../include/utils.h"
#include "freertos/ringbuf.h"
#include "soc/rtc_wdt.h"
#include "driver/gpio.h"

#define TAG "capacitivo_tool"


#define SENSOR1_TOOL_PIN 12
#define SENSOR2_TOOL_PIN 13

SemaphoreHandle_t sens;
SemaphoreHandle_t capacitivo_tool_sem;

gpio_config_t capacitivo_tool_gpio_config;

bool sensor1 = false;
bool sensor2 = false;


static void verificar_sens(void *Parameter, RingbufHandle_t *ring_buf)
{
    xSemaphoreTake(sens, portMAX_DELAY);


    if (sensor1 && sensor2)
    {
        ESP_LOGI(TAG, "Combustivel baixo");
        gpio_set_level(SENSOR1_TOOL_PIN, 1);
        gpio_set_level(SENSOR2_TOOL_PIN, 1);


     
        BaseType_t rc1 = xRingbufferSend(ring_buf, &sensor1, sizeof(sensor1), pdMS_TO_TICKS(1));
        BaseType_t rc2 = xRingbufferSend(ring_buf, &sensor2, sizeof(sensor2), pdMS_TO_TICKS(1));
       
        if (rc1 != pdTRUE || rc2 != pdTRUE)
        {
            ESP_LOGE(TAG, "Falha ao enviar dados para o sbuffer");
        }
    }
    else
    {
        ESP_LOGI(TAG, "Combustivel Normal");
        gpio_set_level(SENSOR1_TOOL_PIN, 0);
        gpio_set_level(SENSOR2_TOOL_PIN, 0);
    }


    xSemaphoreGive(sens);
}


void capacitivo_tool_init(void *p1)
{
   
    sens = xSemaphoreCreateBinary();
   
    gpio_config_t sensor1_config = {
        .pin_bit_mask = (1ULL << SENSOR1_TOOL_PIN),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_ENABLE,
        .intr_type = GPIO_INTR_POSEDGE,
    };


    gpio_config_t sensor2_config = {
        .pin_bit_mask = (1ULL << SENSOR2_TOOL_PIN),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_ENABLE,
        .intr_type = GPIO_INTR_POSEDGE,
    };


    gpio_config(&sensor1_config);
    gpio_config(&sensor2_config);
   
    RingbufHandle_t *ring_buf = (RingbufHandle_t *) p1;

       
    while (true){
        sensor1 = gpio_get_level(SENSOR1_TOOL_PIN);
        sensor2 = gpio_get_level(SENSOR2_TOOL_PIN);

        BaseType_t rc1 = xRingbufferSend(ring_buf, &sensor1, sizeof(sensor1), pdMS_TO_TICKS(1));
        BaseType_t rc2 = xRingbufferSend(ring_buf, &sensor2, sizeof(sensor2), pdMS_TO_TICKS(1));
       
        if (rc1 != pdTRUE || rc2 != pdTRUE)
        {
            ESP_LOGE(TAG, "Falha ao enviar dados para o sbuffer");
        }
            if (sensor1 && sensor2)
             {
             ESP_LOGI(TAG, "Combustivel Normal");
              
            }
                else
                {
                ESP_LOGI(TAG, "Combustivel Baixo");
                }
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}
