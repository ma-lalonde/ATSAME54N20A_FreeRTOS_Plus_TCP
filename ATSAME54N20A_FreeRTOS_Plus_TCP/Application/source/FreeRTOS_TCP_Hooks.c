/*
 * FreeRTOS_TCP_Hooks.c
 *
 * Created: 5/1/2021 1:05:31 AM
 *  Author: Marc-Antoine
 */ 

#include "FreeRTOS.h"
#include "task.h"

#include "FreeRTOS_IP.h"
#include "FreeRTOS_Sockets.h"
#include "FreeRTOS_DHCP.h"

#include "platform.h"


uint32_t ulApplicationGetNextSequenceNumber(
uint32_t ulSourceAddress,
uint16_t usSourcePort,
uint32_t ulDestinationAddress,
uint16_t usDestinationPort )
{
	uint32_t ulReturn;
	( void ) ulSourceAddress;
	( void ) usSourcePort;
	( void ) ulDestinationAddress;
	( void ) usDestinationPort;
	genRand32( &ulReturn );

	return ulReturn;
}

/* Defined by the application code, but called by FreeRTOS+TCP when the network
connects/disconnects (if ipconfigUSE_NETWORK_EVENT_HOOK is set to 1 in
FreeRTOSIPConfig.h). */
void vApplicationIPNetworkEventHook( eIPCallbackEvent_t eNetworkEvent )
{
uint32_t ulIPAddress, ulNetMask, ulGatewayAddress, ulDNSServerAddress;
static BaseType_t xTasksAlreadyCreated = pdFALSE;
char cBuffer[ 16 ];

    /* Check this was a network up event, as opposed to a network down event. */
    if( eNetworkEvent == eNetworkUp )
    {
        /* Create the tasks that use the TCP/IP stack if they have not already been
        created. */
        if( xTasksAlreadyCreated == pdFALSE )
        {
            /*
             * Create the tasks here.
             */

            xTasksAlreadyCreated = pdTRUE;
        }

        /* The network is up and configured.  Print out the configuration,
        which may have been obtained from a DHCP server. */
        FreeRTOS_GetAddressConfiguration( &ulIPAddress,
                                          &ulNetMask,
                                          &ulGatewayAddress,
                                          &ulDNSServerAddress );

        /* Convert the IP address to a string then print it out. */
        //FreeRTOS_inet_ntoa( ulIPAddress, cBuffer );
        //printf( "IP Address: %srn", cBuffer );

        /* Convert the net mask to a string then print it out. */
        //FreeRTOS_inet_ntoa( ulNetMask, cBuffer );
        //printf( "Subnet Mask: %srn", cBuffer );

        /* Convert the IP address of the gateway to a string then print it out. */
        //FreeRTOS_inet_ntoa( ulGatewayAddress, cBuffer );
        //printf( "Gateway IP Address: %srn", cBuffer );

        /* Convert the IP address of the DNS server to a string then print it out. */
        //FreeRTOS_inet_ntoa( ulDNSServerAddress, cBuffer );
        //printf( "DNS server IP Address: %srn", cBuffer );

        //networkStatus = NETWORK_CONNECTED;
    }
    else if( eNetworkEvent == eNetworkDown )
    {
    	//networkStatus = NETWORK_DISCONNECTED;
    }
}


const char *pcApplicationHostnameHook( void )
{
	/* Assign the name "STM32H7" to this network node.  This function will be
	called during the DHCP: the machine will be registered with an IP address
	plus this name. */
	return "ATSAME5X";
}


eDHCPCallbackAnswer_t xApplicationDHCPHook( eDHCPCallbackPhase_t eDHCPPhase, uint32_t ulIPAddress )
{
  eDHCPCallbackAnswer_t eReturn;

  /* This hook is called in a couple of places during the DHCP process, as
  identified by the eDHCPPhase parameter. */
  switch( eDHCPPhase )
  {
    case eDHCPPhasePreDiscover  :
      /* A DHCP discovery is about to be sent out.  eDHCPContinue is
      returned to allow the discovery to go out.

      If eDHCPUseDefaults had been returned instead then the DHCP process
      would be stopped and the statically configured IP address would be
      used.

      If eDHCPStopNoChanges had been returned instead then the DHCP
      process would be stopped and whatever the current network
      configuration was would continue to be used. */
      eReturn = eDHCPContinue;
      break;

    case eDHCPPhasePreRequest  :
      /* Accept offered DHCP address no matter what */
      eReturn = eDHCPContinue;
      break;

    default :
      /* Cannot be reached, but set eReturn to prevent compiler warnings
      where compilers are disposed to generating one. */
      eReturn = eDHCPContinue;
      break;
  }

  return eReturn;
}


BaseType_t xApplicationDNSQueryHook( const char *pcName )
{
	BaseType_t xReturn = pdFAIL;

	/* Determine if a name lookup is for this node.  Two names are given
	to this node: that returned by pcApplicationHostnameHook() and that set
	by mainDEVICE_NICK_NAME. */
	if( strcasecmp( pcName, pcApplicationHostnameHook() ) == 0 )
	{
		xReturn = pdPASS;
	}
	return xReturn;
}

void vApplicationPingReplyHook( ePingReplyStatus_t eStatus, uint16_t usIdentifier )
{
		FreeRTOS_printf( ( "Received ping ID %04X\n", usIdentifier ) );
}