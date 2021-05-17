// Refer to https://www.freertos.org/FreeRTOS-Plus/FreeRTOS_Plus_TCP/TCP_IP_Configuration.html for more details.

#ifndef FREERTOS_IP_CONFIG_H
#define FREERTOS_IP_CONFIG_H

/* Minimize RAM usage */
// If you have a tiny CPU with less than 64KB of RAM, do not use sliding Windows.
#define ipconfigUSE_TCP		1
#define ipconfigUSE_TCP_WIN 1

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
#define ipconfigUSE_NETWORK_EVENT_HOOK		0

/* Support PING requests */
#define ipconfigSUPPORT_OUTGOING_PINGS		1
#define ipconfigREPLY_TO_INCOMING_PINGS		1
#define ipconfigPING_QUEUE_SIZE				2

/* Include support for LLMNR: Link-local Multicast Name Resolution
(non-Microsoft) */
#define ipconfigUSE_LLMNR	1

/* Include support for NBNS: NetBIOS Name Service (Microsoft) */
#define ipconfigUSE_NBNS	0


/* Set ipconfigUSE_DNS to 1 to include a basic DNS client/resolver.  DNS is used
through the FreeRTOS_gethostbyname() API function. */
#define ipconfigUSE_DNS			1

/* Include support for DNS caching.  For TCP, having a small DNS cache is very
useful.  When a cache is present, ipconfigDNS_REQUEST_ATTEMPTS can be kept low
and also DNS may use small timeouts.  If a DNS reply comes in after the DNS
socket has been destroyed, the result will be stored into the cache.  The next
call to FreeRTOS_gethostbyname() will return immediately, without even creating
a socket. */
#define ipconfigUSE_DNS_CACHE					1
#define ipconfigDNS_CACHE_NAME_LENGTH			64
#define ipconfigDNS_CACHE_ENTRIES				4
#define ipconfigDNS_REQUEST_ATTEMPTS			4
#define ipconfigDNS_CACHE_ADDRESSES_PER_ENTRY	4

/* If ipconfigUSE_DHCP is 1 then FreeRTOS+TCP will attempt to retrieve an IP
address, netmask, DNS server address and gateway address from a DHCP server.  If
ipconfigUSE_DHCP is 0 then FreeRTOS+TCP will use a static IP address.  The
stack will revert to using the static IP address even when ipconfigUSE_DHCP is
set to 1 if a valid configuration cannot be obtained from a DHCP server for any
reason.  The static configuration used is that passed into the stack by the
FreeRTOS_IPInit() function call. */
#define ipconfigUSE_DHCP				1
#define ipconfigDHCP_REGISTER_HOSTNAME	1
#define ipconfigUSE_DHCP_HOOK			0
#define ipconfigDHCP_USES_UNICAST       1

/* The ARP cache is a table that maps IP addresses to MAC addresses.  The IP
stack can only send a UDP message to a remove IP address if it knowns the MAC
address associated with the IP address, or the MAC address of the router used to
contact the remote IP address.  When a UDP message is received from a remote IP
address the MAC address and IP address are added to the ARP cache.  When a UDP
message is sent to a remote IP address that does not already appear in the ARP
cache then the UDP message is replaced by a ARP message that solicits the
required MAC address information.  ipconfigARP_CACHE_ENTRIES defines the maximum
number of entries that can exist in the ARP table at any one time. */
#define ipconfigARP_CACHE_ENTRIES		4

/* ARP requests that do not result in an ARP response will be re-transmitted a
maximum of ipconfigMAX_ARP_RETRANSMISSIONS times before the ARP request is
aborted. */
#define ipconfigMAX_ARP_RETRANSMISSIONS	5

/* ipconfigMAX_ARP_AGE defines the maximum time between an entry in the ARP
table being created or refreshed and the entry being removed because it is stale.
New ARP requests are sent for ARP cache entries that are nearing their maximum
age.  ipconfigMAX_ARP_AGE is specified in tens of seconds, so a value of 150 is
equal to 1500 seconds (or 25 minutes). */
#define ipconfigMAX_ARP_AGE			150

/* Implementing FreeRTOS_inet_addr() necessitates the use of string handling
routines, which are relatively large.  To save code space the full
FreeRTOS_inet_addr() implementation is made optional, and a smaller and faster
alternative called FreeRTOS_inet_addr_quick() is provided.  FreeRTOS_inet_addr()
takes an IP in decimal dot format (for example, "192.168.0.1") as its parameter.
FreeRTOS_inet_addr_quick() takes an IP address as four separate numerical octets
(for example, 192, 168, 0, 1) as its parameters.  If
ipconfigINCLUDE_FULL_INET_ADDR is set to 1 then both FreeRTOS_inet_addr() and
FreeRTOS_indet_addr_quick() are available.  If ipconfigINCLUDE_FULL_INET_ADDR is
not set to 1 then only FreeRTOS_indet_addr_quick() is available. */
// Full INET address is only needed for sending pings in this project.
#define ipconfigINCLUDE_FULL_INET_ADDR	0


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
