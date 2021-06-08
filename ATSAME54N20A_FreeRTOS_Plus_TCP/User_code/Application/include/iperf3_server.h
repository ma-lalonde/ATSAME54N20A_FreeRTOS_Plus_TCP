/*
 * iperf3_server.h
 *
 * Created: 6/4/2021 10:03:26 PM
 *  Author: Marc-Antoine
 */ 


#ifndef IPERF3_SERVER_H_
#define IPERF3_SERVER_H_

/* This function will start a task that will handl all IPERF requests from outside.
Call it after the IP-stack is up and running. */

void vIPerfInstall( void );




#endif /* IPERF3_SERVER_H_ */