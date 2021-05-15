/**
 * \file
 *
 * \brief KSZ8081RNA Specific MII Management Register Set
 *
 * Copyright (c) 2016-2018 Microchip Technology Inc. and its subsidiaries.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Subject to your compliance with these terms, you may use Microchip
 * software and any derivatives exclusively with Microchip products.
 * It is your responsibility to comply with third party license terms applicable
 * to your use of third party software (including open source software) that
 * may accompany Microchip software.
 *
 * THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES,
 * WHETHER EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE,
 * INCLUDING ANY IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY,
 * AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT WILL MICROCHIP BE
 * LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, INCIDENTAL OR CONSEQUENTIAL
 * LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND WHATSOEVER RELATED TO THE
 * SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS BEEN ADVISED OF THE
 * POSSIBILITY OR THE DAMAGES ARE FORESEEABLE.  TO THE FULLEST EXTENT
 * ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN ANY WAY
 * RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
 * THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
 *
 * \asf_license_stop
 *
 */

#ifndef KSZ8081RNA_H_INCLUDED
#define KSZ8081RNA_H_INCLUDED

/* KSZ8081RNA Specific Registers */
#define KSZ8081RNA_AFECR1 0x11 // AFE Control 1
#define KSZ8081RNA_RXERCR 0x15 // RXER Counter
#define KSZ8081RNA_OMSOR 0x16  // Operation Mode Strap Override
#define KSZ8081RNA_OMSSR 0x17  // Operation Mode Strap Status
#define KSZ8081RNA_ECR 0x18    // Expanded Control
#define KSZ8081RNA_ICSR 0x1B   // Interrupt Control/Status
#define KSZ8081RNA_LCSR 0x1D   // LinkMD Control/Status
#define KSZ8081RNA_PCR1 0x1E   // PHY Control 1
#define KSZ8081RNA_PCR2 0x1F   // PHY Control 2

#endif /* !KSZ8081RNA_H_INCLUDED */
