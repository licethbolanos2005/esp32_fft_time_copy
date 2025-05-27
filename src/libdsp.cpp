#include <Arduino.h>
#include <arduinoFFT.h>
#include "libdisplay.h"
#include "libwebsocket.h"
#include "libdsp.h"
#include "libstate.h"

#define SR_HZ       10000            // Frecuencia de muestreo
#define N_SAMPLES   1024             // Tamaño de bloque
#define ADC_PIN     7               // Cambiar si usas otro pin ADC del ESP32-S3

static uint16_t rawBuf[N_SAMPLES];
static QueueHandle_t qFFT;

static double vReal[N_SAMPLES];
static double vImag[N_SAMPLES];

ArduinoFFT<double> FFT(vReal, vImag, N_SAMPLES, SR_HZ);

hw_timer_t *timer = NULL;
volatile uint16_t idx = 0;

void IRAM_ATTR onTimer() {
    if (idx == 0) setSystemState(STATE_ADC_CONVERSION); // Al comenzar nueva adquisición
    if (idx < N_SAMPLES) {
        rawBuf[idx++] = analogRead(ADC_PIN);
    }
}


void setupAdc() {
    analogReadResolution(12);  // Resolución a 12 bits
    pinMode(ADC_PIN, INPUT);
}

void setQueue() {
    qFFT = xQueueCreate(2, N_SAMPLES * sizeof(uint16_t));
}

void taskADC(void *) {
    while (true) {
        if (idx >= N_SAMPLES) {
            setSystemState(STATE_ELECTRODES_SIGNAL);
            xQueueSend(qFFT, rawBuf, 0);
            idx = 0;
        }
        vTaskDelay(pdMS_TO_TICKS(1));  // Pequeño retardo para evitar watchdog
    }
}

void taskFFT(void *) {
    uint16_t block[N_SAMPLES];
    for (;;) {
        if (xQueueReceive(qFFT, block, portMAX_DELAY) == pdTRUE) {

            // 1. Filtro (centrado de la señal)
            setSystemState(STATE_FILTERING);
            double mean = 0;
            for (uint16_t i = 0; i < N_SAMPLES; i++) mean += block[i];
            mean /= N_SAMPLES;

            for (uint16_t i = 0; i < N_SAMPLES; i++) {
                vReal[i] = block[i] - mean;
                vImag[i] = 0;
            }

            // 2. FFT
            setSystemState(STATE_PROCESSING);
            FFT.windowing(FFT_WIN_TYP_HAMMING, FFT_FORWARD);
            FFT.compute(FFT_FORWARD);
            FFT.complexToMagnitude();

            // 3. Envío por WebSocket + visualización
            setSystemState(STATE_VISUALIZATION);
            String json = "{\"time\":[";
            for (uint16_t i = 0; i < N_SAMPLES; i++) {
                json += String(block[i]);
                if (i < N_SAMPLES - 1) json += ",";
            }
            json += "],\"freq\":[";
            uint16_t limit = N_SAMPLES / 2;
            for (uint16_t i = 0; i < limit; i++) {
                json += String(vReal[i], 1);
                if (i < limit - 1) json += ",";
            }
            json += "]}";

            ws.textAll(json);
            drawFFT(vReal, vImag);
        }
    }
}


void startTimer() {
    timer = timerBegin(0, 80, true);  // 80 prescaler → 1 µs tick (80 MHz / 80 = 1 MHz)
    timerAttachInterrupt(timer, &onTimer, true);
    timerAlarmWrite(timer, 100, true);  // 100 µs = 10 kHz
    timerAlarmEnable(timer);
}