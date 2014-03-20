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

#ifndef __USB_H__
#define __USB_H__

#include <stdint.h>
#include <stdbool.h>
#include "usb_config.h"

/* public interface to virtual USB peripheral & Stack */

/* some definitions for USB descriptors */
#define DESCRIPTOR_DEVICE		(0x01)	/* Get device descriptor: Device */
#define DESCRIPTOR_CONFIGURATION	(0x02)	/* Get device descriptor: Configuration */
#define DESCRIPTOR_STRING		(0x03)	/* Get device descriptor: String */
#define DESCRIPTOR_INTERFACE            (0x04)
#define DESCRIPTOR_ENDPOINT             (0x05)

#ifdef  USB_ENABLE_HID
#define DESCRIPTOR_HID	                (0x21)	/* Get descriptor: HID */
#define DESCRIPTOR_REPORT	        (0x22)	/* Get descriptor: Report */
#endif

#define EPTYPE_CONTROL                  (0x00)
#define EPTYPE_INTERRUPT                (0x03)


/* Some defines for endpoint function parameters => TODO, will be used in a later stage */
#define USB_EPSETUP     (0)
#define USB_EP0OUT      (16)
#define USB_EP0IN       (32)
#if USB_ENABLE_EP1OUT_EP1IN == 1
  #define USB_EP1OUT      (48)
  #define USB_EP1IN       (64)
#endif


/* struct, which is used for control transfers */
typedef struct  
{
  uint8_t  bmRequestType;
  uint8_t  bRequest;
  uint16_t wLength;
  uint16_t wValue;
  uint16_t wIndex;
} setupData_t;



/* Initialize USB and all used peripherals
   It startes with the USB peripheral disconnected from PC.
   Call usb_connect after initialization.
*/
void usb_init(void);

void usb_connect(void);

void usb_disconnect(void);

bool usb_check_resetcondition(void);

void usb_reset_received(void);



/* check, if a setup packet was received */
bool usb_setup_available(void);

/* read setup data. Setup data does have a fixed length of 8 Bytes */
void usb_setup_get_data(setupData_t *psetupdata);



/* check, if data can be written to ep0in buffer (== buffer is empty) */
bool usb_ep_in_buf_empty(uint32_t epnum);


/* write data to ep0in buffer */
void usb_ep_in_commit_pkt(uint32_t epnum, bool firstpkt, const uint8_t *pbuf, uint32_t len);



/* check, if received data is in OUT endpoint */
bool usb_ep_out_data_available(uint32_t epnum);

/* get data from EPxOUT. Length [bytes] is returned. */
uint32_t usb_ep_out_get_data(uint32_t epnum, uint8_t *pbuf);

/* stall endpoint */
void usb_ep_stall(uint32_t epnum);

/* remove stall condition from endpoint */
void usb_ep_unstall(uint32_t epnum);

/* returns true if endpoint is stalled */
bool usb_ep_is_stalled(uint32_t epnum);

/* receive a package via EP0OUT. It is allowed to be longer than 8 bytes */
/* important: ensure, that pbuf has a buffer size with a multiple of 8 Bytes */
/* returns received length */
uint8_t usb_ep_out_get_data_long(uint8_t *pbuf, uint8_t maxlen);

/* send a package via EP0IN which is longer as 7 bytes */
//void usb_ep_in_commit_pkt_long(uint32_t epnum, const uint8_t *pbuf, uint8_t len);
void usb_ep_in_commit_pkt_long(uint32_t epnum, const uint8_t *pbuf, uint8_t len, bool sendshortpkt);



/* Handle a Control transfer without data stage => transmits a ACK in status stage */
void usb_control_acknowledge(void);

/* Handle a Control IN transfer */
bool usb_control_dataIn(const uint8_t *pbuf, uint8_t len);

/* Handle a Control OUT transfer */
uint8_t usb_control_dataOut(uint8_t *pbuf, uint8_t maxlen);

#endif
