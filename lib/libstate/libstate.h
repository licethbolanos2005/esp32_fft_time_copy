#ifndef LIBSTATE_H
#define LIBSTATE_H

enum SystemState {
    STATE_INIT,
    STATE_ELECTRODES_SIGNAL,
    STATE_ADC_CONVERSION,
    STATE_FILTERING,
    STATE_PROCESSING,
    STATE_VISUALIZATION
};

const char* stateToString(SystemState state);

void initStateMachine();
void setSystemState(SystemState state);

#endif
