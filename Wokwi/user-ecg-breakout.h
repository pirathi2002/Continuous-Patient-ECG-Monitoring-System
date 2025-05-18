#ifndef USER_ECG_BREAKOUT1_H
#define USER_ECG_BREAKOUT1_H

// Function prototypes for initializing and generating the ECG signal
void user_ecg_breakout_init();
float user_ecg_breakout_generate_ecg_signal();

// Define pins for your chip
#define VCC 1    // Power supply pin
#define GND 2    // Ground pin
#define OUT 3    // Output pin for ECG signal

#endif // USER_ECG_BREAKOUT1_H
