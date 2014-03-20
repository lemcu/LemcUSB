/****************************************************************************
**
** Lemcusb - Firmware USB driver for EFM32 Microcontroller
** Copyright (C) 2014 http://lemcu.org
**
** This library is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License version 3.0 as
** published by the Free Software Foundation and appearing in the file
** LICENSE.txt included in the packaging of this file.
** 
** In addition, as a special exception, http://lemcu.org gives you certain
** additional rights. These rights are described in the lemcu.org GPL 
** Exception version 1.0, included in the file GPL_EXCEPTION.txt in this
** package.
**
** This library is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

/*
 * Low level internal functions.
 * These functions are internal.
 */

#include "em_device.h"
#include <stdint.h>
#include "em_chip.h"
#include "em_int.h"
#include "em_gpio.h"
#include "em_cmu.h"
   
#include "usb_config.h"
   
/* Endpoint buffers 
   Offset  0:    SETUP packets
   Offset 16:    EP0OUT
   Offset 32:    EP0IN
  (Offset 48:    EP1OUT)
  (Offset 64:    EP1IN)
   
   Offsets within one of theese buffers:
         0-11:   Raw data
   Offset  12:    _STAT  Status empty/filled (0x00/0x01)
   Offset  13:    _LEN    Length of data in buffer
   14 & 15 are free for future used - would be added anyway by codegen tools for alignment
 */
volatile uint8_t usb_ep_buffers[ (3+2*USB_ENABLE_EP1OUT_EP1IN)*16 ];
   
/* low level protocol handling state */
/* Bit  0- 7 = state machine index*/
/* Bit  8-15 = last received USB Address  (bitreversed: a0 a1 a2 a3 a4 a5 a6  0) */
/* Bit 16-23 = last received USB Endpoint (bitreversed: e0 e1 e2 e3  0  0  0  0) */
/* Bit 24-31 = not used yet */
uint32_t USB_PROT_STATE;



