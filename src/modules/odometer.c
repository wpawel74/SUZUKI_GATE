#include "os_compat.h"
#include "config.h"
#include "sensors.h"

#include <stm32_at24.h>
#include "tm_stm32_delay.h"

#include <stdio.h>

#define EEPROM_ODOMETER_ADDR				0

#define SPEEDO_SENSOR_DISTANCE_METERS		3.4

volatile u32 odometer_irqc = 0;

struct ODOMETER_F {
	volatile u32 value_1;
	volatile u32 value_2;
	volatile u32 value_3;
};

struct ODOMETER_F odometer = { .value_1 = 0, .value_2 = 0, .value_3 = 0 };

static uint32_t next_time = 0;

void odometer_init(void){
	PW_AT24xx_Init( I2C1 );
	PW_AT24xx_Read( I2C1, EEPROM_ODOMETER_ADDR,
						 (uint8_t *)&odometer,
						 sizeof(odometer) );
	_D(("I: ODOMETER module loaded ... OK\n"));
	_D(("I: odometer RD: %ld/%ld [meters]\n", odometer.value_1, odometer.value_2));
}

u32 odometer_get(void){
	PW_AT24xx_Read( I2C1, EEPROM_ODOMETER_ADDR,
						 (uint8_t *)&odometer,
						 sizeof(odometer) );
	return (u32)(((double)odometer.value_1) * SPEEDO_SENSOR_DISTANCE_METERS);
}

void odometer_push_eeprom(void){
	PW_AT24xx_Write( I2C1, EEPROM_ODOMETER_ADDR,
						 (uint8_t *)&odometer,
						 sizeof(odometer) );
}

void odometer_poll(void){
	odometer.value_1 += odometer_irqc;
	odometer_irqc = 0;
	if( HAL_GetTick() > next_time ){
		_D(("I: [%ld] odometer %ld [m]\n", HAL_GetTick(), odometer_get() ));
		next_time = HAL_GetTick();// + (3 * 1000);
	}
}
