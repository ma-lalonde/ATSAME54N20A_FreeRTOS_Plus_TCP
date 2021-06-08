/*
 * app_mqtt.h
 *
 * Created: 6/2/2021 5:22:34 PM
 *  Author: Marc-Antoine
 */ 


#ifndef APP_MQTT_H_
#define APP_MQTT_H_

#include "subscription_manager.h"

void vStartSenseAndReportTask();

void vSubscribeToTopic( char * topic, IncomingPubCallback_t callback_function );

#endif /* APP_MQTT_H_ */