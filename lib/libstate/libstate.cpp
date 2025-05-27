#include "libstate.h"
#include <Arduino.h>
#include <TelnetStream.h>  // Asegúrate de tener instalada esta biblioteca

static const char* stateNames[] = {
    "Inicio",
    "Senal desde electrodos",
    "Conversion A/D",
    "Filtrado",
    "Procesamiento",
    "Visualizacion"
};

static SystemState currentState = STATE_INIT;

void initStateMachine() {
    TelnetStream.begin();
    TelnetStream.println("Máquina de estados iniciada.");
}

void setSystemState(SystemState newState) {
    static unsigned long lastPrintTime = 0;
    unsigned long now = millis();

    if (newState != currentState) {
        currentState = newState;
        if (now - lastPrintTime >= 5000) {  // 5 segundos
            TelnetStream.print("");
            TelnetStream.println(stateToString(currentState));
            lastPrintTime = now;
        }
    }
}

const char* stateToString(SystemState state) {
    if (state >= STATE_INIT && state <= STATE_VISUALIZATION) {
        return stateNames[state];
    } else {
        return "Desconocido";
    }
}
