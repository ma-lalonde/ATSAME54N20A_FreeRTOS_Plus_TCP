/*
 * app_config.c
 *
 * Created: 5/27/2021 3:02:53 PM
 *  Author: Marc-Antoine
 */ 

#include "app_config.h"

#define PORT1_PWM_POS		( 1 << 0 )

#define PORT2_PWM_POS		( 1 << 1 )

#define PORT3_PWM_POS		( 1 << 2 )

static uint32_t config_reg1 = 0;




void config_load()
{
	/* Eventually load from persistent storage */
	config_reg1 |= PORT1_PWM_POS;
	config_reg1 &= ~PORT2_PWM_POS;
	config_reg1 |= PORT3_PWM_POS;
}

void config_save()
{
	/* Eventually save to persistent storage */
}


bool config_port1_is_pwm()
{
	return ( config_reg1 & PORT1_PWM_POS );
}


bool config_port2_is_pwm()
{
	return ( config_reg1 & PORT2_PWM_POS );
}

bool config_port3_is_pwm()
{
	return ( config_reg1 & PORT3_PWM_POS );
}
