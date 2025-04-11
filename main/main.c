/*
inmp441和max98357a的I2S音频采集和播放实例
时间:11/4/2025
作者:***
基于ESP32-S3-DevKitC-1开发板
*/

#include <stdio.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_system.h"
#include "driver/gpio.h"
#include "driver/i2s.h"

//引脚定义,详见audio_codes.readme.md
#define I2S_INMP441_WS_GPIO     6 // INMP441 WS pin
#define I2S_INMP441_SCK_GPIO    7 // INMP441 SCK pin
#define I2S_INMP441_SD_GPIO     4 // INMP441 SD pin     采集数据
#define I2S_MAX98357A_DIN_GPIO  5// MAX98357A WS pin

#define I2S_SAMPLE_RATE 44100 // 采样率
#define I2S_BUFFER_SIZE 1024  // DMA 缓冲区大小

int32_t sample = 0;
size_t bytes_read = 0;
size_t bytes_written = 0;

void I2S_Init(void)
{
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_TX), // 主模式，接收和发送
        .sample_rate = I2S_SAMPLE_RATE,                                    // 采样率
        .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,                      // 32 位数据
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,                       // 单声道（左声道）
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,                 // 标准 I2S 格式
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,                          // 中断优先级
        .dma_buf_count = 8,                                                // DMA 缓冲区数量
        .dma_buf_len = I2S_BUFFER_SIZE,                                    // DMA 缓冲区长度
        .use_apll = false                                                  // 不使用 APLL
    };

    i2s_pin_config_t pin_config = {
        .bck_io_num = I2S_INMP441_SCK_GPIO,    // 位时钟
        .ws_io_num = I2S_INMP441_WS_GPIO,      // 左右声道时钟
        .data_out_num = I2S_MAX98357A_DIN_GPIO, // 数据输出（连接到 MAX98357 的 DIN）
        .data_in_num = I2S_INMP441_SD_GPIO     // 数据输入（连接到 INMP441 的 DOUT）
    };

    // 初始化 I2S
    i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
    i2s_set_pin(I2S_NUM_0, &pin_config);
}

void app_main(void)
{
    I2S_Init(); // 初始化 I2S
    while (1)
    {
        // 从 INMP441 读取音频数据
        i2s_read(I2S_NUM_0, &sample, sizeof(sample), &bytes_read, portMAX_DELAY);
        
        //仅调试使用,正常使用时请勿打开
        // vTaskDelay(10 / portTICK_PERIOD_MS); // 延时 10 毫秒，避免过快读取
        // printf("Read sample: %" PRId32 "\n", sample); // 打印读取的音频数据
        
        // 将音频数据发送到 MAX98357
        if (bytes_read > 0)
        {
            i2s_write(I2S_NUM_0, &sample, sizeof(sample), &bytes_written, portMAX_DELAY);
        }
    }
}