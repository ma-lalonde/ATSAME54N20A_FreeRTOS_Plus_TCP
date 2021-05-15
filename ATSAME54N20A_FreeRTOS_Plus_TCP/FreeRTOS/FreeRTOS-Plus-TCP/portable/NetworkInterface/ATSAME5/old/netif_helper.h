/*
 * phy_helper.h
 *
 * Created: 5/3/2021 5:02:14 PM
 *  Author: Marc-Antoine
 */ 


#ifndef PHY_HELPER_H_
#define PHY_HELPER_H_

#include "FreeRTOS.h"
#include "task.h"

/*/ @cond 0 */
/**INDENT-OFF**/
#ifdef __cplusplus
	extern "C" {
#endif
/**INDENT-ON**/
/*/ @endcond */

/** The buffer addresses written into the descriptors must be aligned, so the
 *  last few bits are zero.  These bits have special meaning for the GMAC
 *  peripheral and cannot be used as part of the address. 
 *
 * These values reflect the ones defined for the rx and tx descriptors in hpl_gmac.c */
    #define GMAC_RXD_ADDR_MASK         0xFFFFFFFC
    #define GMAC_RXD_WRAP              ( 1ul << 1 )  /**< Wrap bit */
    #define GMAC_RXD_OWNERSHIP         ( 1ul << 0 )  /**< Ownership bit */

    #define GMAC_RXD_BROADCAST         ( 1ul << 31 ) /**< Broadcast detected */
    #define GMAC_RXD_MULTIHASH         ( 1ul << 30 ) /**< Multicast hash match */
    #define GMAC_RXD_UNIHASH           ( 1ul << 29 ) /**< Unicast hash match */
    #define GMAC_RXD_ADDR_FOUND        ( 1ul << 27 ) /**< Specific address match found */
    #define GMAC_RXD_ADDR              ( 3ul << 25 ) /**< Address match */
    #define GMAC_RXD_RXCOEN            ( 1ul << 24 ) /**< RXCOEN related function */
    #define GMAC_RXD_TYPE              ( 3ul << 22 ) /**< Type ID match */
    #define GMAC_RXD_VLAN              ( 1ul << 21 ) /**< VLAN tag detected */
    #define GMAC_RXD_PRIORITY          ( 1ul << 20 ) /**< Priority tag detected */
    #define GMAC_RXD_PRIORITY_MASK     ( 3ul << 17 ) /**< VLAN priority */
    #define GMAC_RXD_CFI               ( 1ul << 16 ) /**< Concatenation Format Indicator only if bit 21 is set */
    #define GMAC_RXD_EOF               ( 1ul << 15 ) /**< End of frame */
    #define GMAC_RXD_SOF               ( 1ul << 14 ) /**< Start of frame */
    #define GMAC_RXD_FCS               ( 1ul << 13 ) /**< Frame check sequence */
    #define GMAC_RXD_OFFSET_MASK                     /**< Receive buffer offset */
    #define GMAC_RXD_LEN_MASK          ( 0xFFF )     /**< Length of frame including FCS (if selected) */
    #define GMAC_RXD_LENJUMBO_MASK     ( 0x3FFF )    /**< Jumbo frame length */

    #define GMAC_TXD_USED              ( 1ul << 31 ) /**< Frame is transmitted */
    #define GMAC_TXD_WRAP              ( 1ul << 30 ) /**< Last descriptor */
    #define GMAC_TXD_ERROR             ( 1ul << 29 ) /**< Retry limit exceeded, error */
    #define GMAC_TXD_UNDERRUN          ( 1ul << 28 ) /**< Transmit underrun */
    #define GMAC_TXD_EXHAUSTED         ( 1ul << 27 ) /**< Buffer exhausted */
    #define GMAC_TXD_LATE              ( 1ul << 26 ) /**< Late collision,transmit  error  */
    #define GMAC_TXD_CHECKSUM_ERROR    ( 7ul << 20 ) /**< Checksum error */
    #define GMAC_TXD_NOCRC             ( 1ul << 16 ) /**< No CRC */
    #define GMAC_TXD_LAST              ( 1ul << 15 ) /**< Last buffer in frame */
    #define GMAC_TXD_LEN_MASK          ( 0x1FFF )    /**< Length of buffer */


/**
 * \brief Check if PHY is idle.
 *
 * \param p_gmac   Pointer to the GMAC instance.
 *
 * \return  1 if PHY is idle.
 */
extern inline uint8_t gmac_is_phy_idle();

/**
 * \brief Wait PHY operation to be completed.
 *
 * \param p_gmac HW controller address.
 * \param ul_retry The retry times, 0 to wait forever until completeness.
 *
 * Return pdTRUE if the operation is completed successfully, otherwise pdFALSE.
 */
BaseType_t gmac_wait_phy( const uint32_t ul_retry );


/**
 * \brief Return transmit status.
 *
 * \param p_gmac   Pointer to the GMAC instance.
 *
 * \return  Transmit status.
 */
extern inline uint32_t gmac_get_tx_status();

/**
 * \brief Disable transfer, reset registers and descriptor lists.
 *
 * \param p_dev Pointer to GMAC driver instance.
 *
 */
void gmac_reset_tx_mem();

/**
 * \brief Set Tx Queue.
 *
 * \param p_gmac   Pointer to the GMAC instance.
 * \param ul_addr  Tx queue address.
 */
extern inline void gmac_set_tx_queue( uint32_t ul_addr );



/**
 * \brief Set base address of TX buffer.
 *
 * \param p_gmac   Pointer to the GMAC instance.
 * \param queue_idx   Index of queue, start from 1
 */
//static inline void gmac_set_tx_priority_queue( mac_async_descriptor * eth_mac_desc, uint32_t ul_addr, gmac_quelist_t queue_idx );

/**
 * \brief Enable/Disable GMAC management.
 *
 * \param p_gmac   Pointer to the GMAC instance.
 * \param uc_enable   0 to disable GMAC management, else to enable it.
 */
extern inline void gmac_enable_management( uint8_t uc_enable );

/**
 * \brief Clear transmit status.
 *
 * \param p_gmac   Pointer to the GMAC instance.
 * \param ul_status   Transmit status.
 */
extern inline void gmac_clear_tx_status( uint32_t ul_status );

/**
 * \brief Set MAC Address.
 *
 * \param p_gmac   Pointer to the GMAC instance.
 * \param uc_index  GMAC specific address register index.
 * \param p_mac_addr  GMAC address.
 */
extern inline void gmac_set_address( const uint8_t * p_mac_addr );


#endif /* PHY_HELPER_H_ */

/*/ @cond 0 */
/**INDENT-OFF**/
#ifdef __cplusplus
}
#endif
/**INDENT-ON**/
/*/ @endcond */