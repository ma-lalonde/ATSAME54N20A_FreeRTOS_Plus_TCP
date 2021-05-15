/*
 * phy_helper.c
 *
 * Created: 5/3/2021 5:02:03 PM
 *  Author: Marc-Antoine
 */ 
#include "netif_helper.h"

#include "hal_mac_async.h"

/*/ @cond 0 */
/**INDENT-OFF**/
#ifdef __cplusplus
	extern "C" {
#endif
/**INDENT-ON**/
/*/ @endcond */

/** Clear circular buffer */
//#define CIRC_CLEAR( head, tail )          do { ( head ) = 0; ( tail ) = 0; } while( 0 )

///* Transmit and Receive buffer descriptor array defined in hpl_mac.c */
//extern struct _mac_txbuf_descriptor _txbuf_descrs[CONF_GMAC_TXDESCR_NUM];
//extern struct _mac_rxbuf_descriptor _rxbuf_descrs[CONF_GMAC_RXDESCR_NUM];
//
///* Transmit buffer data array defined in hpl_mac.c */
//extern uint8_t _txbuf[CONF_GMAC_TXDESCR_NUM][CONF_GMAC_TXBUF_SIZE];
//extern uint8_t _rxbuf[CONF_GMAC_RXDESCR_NUM][CONF_GMAC_RXBUF_SIZE];


inline uint8_t gmac_is_phy_idle()
{
	return( ( GMAC->GMAC_NSR_OFFSET & GMAC_NSR_IDLE ) > 0 );
}

BaseType_t gmac_wait_phy( const uint32_t ul_retry )
{
    volatile uint32_t ul_retry_count = 0;
    const uint32_t xPHYPollDelay = pdMS_TO_TICKS( 1ul );

    while( !gmac_is_phy_idle() )
    {
        if( ul_retry == 0 )
        {
            continue;
        }

        ul_retry_count++;

        if( ul_retry_count >= ul_retry )
        {
            return pdFALSE;
        }

        /* Block the task to allow other tasks to execute while the PHY
         * is not connected. */
        vTaskDelay( xPHYPollDelay );
    }

    return pdTRUE;
}

inline uint32_t gmac_get_tx_status()
{
	return GMAC->GMAC_TSR_OFFSET;
}

/**
 * \brief Disable transfer, reset registers and descriptor lists.
 *
 * \param p_dev Pointer to GMAC driver instance.
 *
 */
//void gmac_reset_tx_mem( mac_async_descriptor * eth_mac_desc )
//{
    //uint32_t ul_index;
    //uint32_t ul_address;
//
    ///* Disable TX */
    //mac_async_disable( eth_mac_desc );
//
    //{
        //for( ul_index = 0; ul_index < ARRAY_SIZE( _txbuf_descrs ); ul_index++ )
        //{
            //uint32_t ulAddr = _txbuf_descrs[ ul_index ].address;
//
            //if( ulAddr )
            //{
                //returnTxBuffer( ( uint8_t * ) ulAddr );
            //}
        //}
    //}
    ///* Set up the TX descriptors */
    //CIRC_CLEAR( p_dev->l_tx_head, p_dev->l_tx_tail );
//
    //for( ul_index = 0; ul_index < CONF_GMAC_TXDESCR_NUM; ul_index++ )
    //{
        //#if ( ipconfigZERO_COPY_TX_DRIVER != 0 )
            //{
                //ul_address = ( uint32_t ) 0u;
            //}
        //#else
            //{
                //ul_address = ( uint32_t ) ( &( _txbuf[ ul_index ][ CONF_GMAC_TXBUF_SIZE ] ) );
            //}
        //#endif /* ipconfigZERO_COPY_TX_DRIVER */
        //_txbuf_descrs[ ul_index ].address = ul_address;
        //_txbuf_descrs[ ul_index ].status.val = GMAC_TXD_USED;
    //}
//
    ///* Set the WRAP bit in the last descriptor. */
    //_txbuf_descrs[ CONF_GMAC_TXDESCR_NUM - 1 ].status.val = GMAC_TXD_USED | GMAC_TXD_WRAP;
//
    ///* Set transmit buffer queue */
    //gmac_set_tx_queue( ( uint32_t ) _txbuf_descrs );
    ////#if ( (defined( SAME70) && ( SAME70 != 0 ) )
        ////{
            ////gmac_set_tx_priority_queue( p_hw, ( uint32_t ) &_txbuf_descrs_null, GMAC_QUE_1 );
            ////gmac_set_tx_priority_queue( p_hw, ( uint32_t ) &_txbuf_descrs_null, GMAC_QUE_2 );
            /////* Note that SAME70 REV B had 6 priority queues. */
            ////gmac_set_tx_priority_queue( p_hw, ( uint32_t ) &_txbuf_descrs_null, GMAC_QUE_3 );
            ////gmac_set_tx_priority_queue( p_hw, ( uint32_t ) &_txbuf_descrs_null, GMAC_QUE_4 );
            ////gmac_set_tx_priority_queue( p_hw, ( uint32_t ) &_txbuf_descrs_null, GMAC_QUE_5 );
        ////}
    ////#endif
//}


inline void gmac_set_tx_queue( uint32_t ul_addr )
{
	GMAC->GMAC_TBQB_OFFSET = GMAC_TBQB_ADDR_Msk & ul_addr;
}


//static inline void gmac_set_tx_priority_queue( mac_async_descriptor * eth_mac_desc, uint32_t ul_addr, gmac_quelist_t queue_idx )
//{
	//eth_mac_desc->dev->hw->GMAC_TBQBAPQ[ queue_idx - 1 ] = GMAC_TBQB_ADDR_Msk & ul_addr;
//}

inline void gmac_enable_management( uint8_t uc_enable )
{
	if( uc_enable )
	{
		GMAC->GMAC_NCR_OFFSET |= GMAC_NCR_MPE;
	}
	else
	{
		GMAC->GMAC_NCR_OFFSET &= ~GMAC_NCR_MPE;
	}
}


inline void gmac_clear_tx_status( uint32_t ul_status )
{
	GMAC->GMAC_TSR_OFFSET = ul_status;
}

inline void gmac_set_address( const uint8_t * p_mac_addr )
{
	GMAC->GMAC_SAB_OFFSET = ( p_mac_addr[ 3 ] << 24 )
	| ( p_mac_addr[ 2 ] << 16 )
	| ( p_mac_addr[ 1 ] << 8 )
	| ( p_mac_addr[ 0 ] );
	GMAC->GMAC_SAT_OFFSET = ( p_mac_addr[ 5 ] << 8 )
	| ( p_mac_addr[ 4 ] );
}



/*/ @cond 0 */
/**INDENT-OFF**/
#ifdef __cplusplus
}
#endif
/**INDENT-ON**/
/*/ @endcond */