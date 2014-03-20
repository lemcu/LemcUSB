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

#ifndef __USB_INTERNAL_H__
#define __USB_INTERNAL_H__

#include <stdint.h>
#include <stdbool.h>
#include "usb_config.h"

#define  USB_EPBUF_OFFSET_SETUP_BUF             ( 0)
#define  USB_EPBUF_OFFSET_EP0OUT_BUF            (16)
#define  USB_EPBUF_OFFSET_EP0IN_BUF             (32)
#define  USB_EPBUF_OFFSET_EP1OUT_BUF            (48)
#define  USB_EPBUF_OFFSET_EP1IN_BUF             (64)

#define  USB_EPBUF_SUBOFFSET_STAT               (12)
#define  USB_EPBUF_SUBOFFSET_LEN                (13)


/* Endpoint buffers 
   Offset  0:    SETUP packets
   Offset 16:    EP0OUT
   Offset 32:    EP0IN
  (Offset 48:    EP1OUT)
  (Offset 64:    EP1IN)
   
   Sub-offsets within one of these buffers:
         0-11:   Raw data
   Offset  12:    _STAT  Status empty/filled (0x00/0x01)
   Offset  13:    _LEN    Length of data in buffer
   14 & 15 are free for future used - would be added anyway by codegen tools for alignment
 */


/* Endpoint buffers */
extern volatile uint8_t usb_ep_buffers[ (3+2*USB_ENABLE_EP1OUT_EP1IN)*16 ];

extern uint32_t USB_PROT_STATE;


#endif
