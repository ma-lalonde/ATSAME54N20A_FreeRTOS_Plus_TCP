// Refer to https://www.freertos.org/FreeRTOS-Plus/FreeRTOS_Plus_TCP/TCP_IP_Configuration.html for more details.

#ifndef FREERTOS_IP_CONFIG_H
#define FREERTOS_IP_CONFIG_H

/* Minimize RAM usage */
// If you have a tiny CPU with less than 64KB of RAM, do not use sliding Windows.
#define ipconfigUSE_TCP_WIN       1

// If RAM is really constrained then use smaller segments:
// All peers will understand this and only send small packets.
#define ipconfigNETWORK_MTU   1500

// The window size will be fixed to 1 MSS. The buffer size can be declared as 1 or 2 MSS:
#define ipconfigTCP_TX_BUFFER_LENGTH   ( 2 * ipconfigTCP_MSS )
#define ipconfigTCP_RX_BUFFER_LENGTH   ( 2 * ipconfigTCP_MSS )

// Only allocate the minimum number of network buffer descriptors you can get away with. 
// This also has the effect of preventing high network traffic resulting in memory exhaustion
// as network buffers will not be allocated if no descriptors are available:
#define ipconfigNUM_NETWORK_BUFFER_DESCRIPTORS  8
#define ipconfigEVENT_QUEUE_LENGTH				( ipconfigNUM_NETWORK_BUFFER_DESCRIPTORS + 5 )

/*******************************/
/* TCP/IP stack constants */
#define ipconfigIP_TASK_PRIORITY			( configMAX_PRIORITIES - 1 )
#define ipconfigIP_TASK_STACK_SIZE_WORDS	1024
#define ipconfigUSE_NETWORK_EVENT_HOOK		1
#define ipconfigUSE_DHCP					1
#define ipconfigUSE_DHCP_HOOK				1
#define ipconfigDHCP_REGISTER_HOSTNAME		1
#define ipconfigDHCP_USES_UNICAST			1

/* Debug, trace and logging */
#define ipconfigHAS_DEBUG_PRINTF	0
#define ipconfigHAS_PRINTF			0

/* Hardware and driver specific */
#define ipconfigBYTE_ORDER							pdFREERTOS_LITTLE_ENDIAN
#define ipconfigDRIVER_INCLUDED_TX_IP_CHECKSUM		1
#define ipconfigDRIVER_INCLUDED_RX_IP_CHECKSUM		1
#define ipconfigETHERNET_DRIVER_FILTERS_FRAME_TYPES	1
#define ipconfigETHERNET_DRIVER_FILTERS_PACKETS		0

#define ipconfigFILTER_OUT_NON_ETHERNET_II_FRAMES	0
#define ipconfigUSE_LINKED_RX_MESSAGES				0

#define ipconfigZERO_COPY_TX_DRIVER	0
#define ipconfigZERO_COPY_RX_DRIVER	0

#define ipconfigETHERNET_AN_ENABLE			1
#define ipconfigETHERNET_AUTO_CROSS_ENABLE	1


#endif /* FREERTOS_IP_CONFIG_H */
