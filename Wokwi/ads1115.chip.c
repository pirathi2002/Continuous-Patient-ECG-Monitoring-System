#include "wokwi-api.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define I2C_ADDRESS 0x48

typedef struct {
  uint16_t conversion_register; // Holds the ADC conversion result
  uint8_t config_register;      // Configuration register
  i2c_dev_t i2c_dev;            // I2C device
  i2c_config_t i2c_config;      // I2C configuration
} chip_state_t;

// Function prototypes
void chip_init();
bool on_i2c_connect(void *user_data, uint32_t address, bool read);
uint8_t on_i2c_read(void *user_data);
bool on_i2c_write(void *user_data, uint8_t data);
void simulate_adc_conversion(chip_state_t *chip);

// Initializes the chip
void chip_init() {
  chip_state_t *chip = malloc(sizeof(chip_state_t));
  memset(chip, 0, sizeof(chip_state_t)); // Initialize all values to 0

  // Set up the I2C configuration
  chip->i2c_config.address = I2C_ADDRESS;
  chip->i2c_config.scl = pin_init("SCL", INPUT);
  chip->i2c_config.sda = pin_init("SDA", INPUT);
  chip->i2c_config.connect = on_i2c_connect;
  chip->i2c_config.read = on_i2c_read;
  chip->i2c_config.write = on_i2c_write;
  chip->i2c_config.user_data = chip;
  chip->i2c_dev = i2c_init(&(chip->i2c_config));

  // Initialize the ADC registers
  chip->conversion_register = 0; // Default conversion value
  chip->config_register = 0;     // Default config
}

// Handle I2C connect event
bool on_i2c_connect(void *user_data, uint32_t address, bool read) {
  chip_state_t *chip = user_data;
  return address == I2C_ADDRESS;
}

// Handle I2C read event
uint8_t on_i2c_read(void *user_data) {
  chip_state_t *chip = user_data;
  simulate_adc_conversion(chip); // Simulate ADC conversion
  return chip->conversion_register & 0xFF; // Return the lower byte of the conversion register
}

// Handle I2C write event
bool on_i2c_write(void *user_data, uint8_t data) {
  chip_state_t *chip = user_data;
  chip->config_register = data; // Store the configuration
  return true; // Acknowledge the write
}

// Simulates an ADC conversion
void simulate_adc_conversion(chip_state_t *chip) {
  // Simple ADC conversion simulation
  // This should be adapted to simulate analog-to-digital conversion based on the config register
  // For now, let's just assign a fixed value
  chip->conversion_register = 1234; // Example fixed ADC value
}
