#include <stdint.h>
#include <stdbool.h> 

// pwm enabled structure
typedef struct {
	uint8_t pin_num;
	bool is_enabled;
} PWM_Enabled;

extern PWM_Enabled pwm_enabled_array[];

// pwm basic configuration
void PWM_init(void);

// turn led on
int led_ON(uint8_t pin_num);

// turn led off
int led_OFF(uint8_t pin_num);

// dimmer led
int led_DIMMER(uint8_t pin_num, uint8_t intensity);

// read led intensity
uint8_t get_intensity(uint8_t pin_num);

