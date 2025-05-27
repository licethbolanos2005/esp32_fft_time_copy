#include <Arduino.h>
#include "libdisplay.h"
#include "libwebsocket.h"
#include "libdsp.h"
#include "libwifi.h"
#include "libstate.h"          // ← Agrega esta línea

const char* HOSTNAME = "fft32";

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("=== Inicio de setup ===");

  listWiFiNetworks();
  startWiFi(HOSTNAME);
  setMDNS(HOSTNAME);
  setupWebSocket();

  initDisplay();
  setupAdc();
  setQueue();

  xTaskCreatePinnedToCore(taskADC, "ADC", 4096, NULL, 1, NULL, 0);
  xTaskCreatePinnedToCore(taskFFT, "FFT", 8192, NULL, 1, NULL, 1);

  initStateMachine();
  setSystemState(STATE_INIT);  // Estado inicial al arrancar

  startTimer();
}

void loop() {
  // Nada que hacer aquí.
}
