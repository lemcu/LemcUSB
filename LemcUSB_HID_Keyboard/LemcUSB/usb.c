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

#include "usb.h"
#include "em_device.h"
#include "em_cmu.h"
#include "em_int.h"
#include "em_timer.h"
#include "em_gpio.h"
#include "usb_internal_ll.h"
#include "usb_stack.h"
#include "usb_helperfunctions.h"


void usb_reset_received(void)
{  
  uint32_t cnt;
  uint32_t *ptr;
  
  GPIO->P[4].DOUTSET = (1<<13);
  
  /* zero intitialize endpoint buffers and len/status fields */
  ptr = (uint32_t *)usb_ep_buffers;
  for (cnt=0; cnt < (sizeof(usb_ep_buffers)>>2); cnt++)
  {
    *ptr++ = 0;
  }
  
  /* preinitialize DATA PIDS for IN endpoints */
  usb_ep_buffers[USB_EPBUF_OFFSET_EP0IN_BUF] = 0x4B;
  usb_ep_buffers[USB_EPBUF_OFFSET_EP1IN_BUF] = 0x4B;

  /* reset usb device address to 0 (will be changed during enumeration) */
  usbstack_init();
  GPIO->P[4].DOUTCLR = (1<<13);
}

/*
 * Pinout information:
 * PC0  = D-
 * PC1  = D+
 * PE13 = debug output (to scope)
 */

/*
 * Pre-condition:   GPIO clock enabled
 *  
 */
void usb_init(void)
{
  usb_reset_received();
  
  /* Pinout: PA0 = USB connect line (1k5 resistor to PC0)
             PC0 = D-
             PC1 = D+
  */
  /* set PA0 low (disconnected) Will enable the pulldown */
  GPIO->P[0].DOUTCLR = (1 << 0);
  /* Pin PA0 is configured to Input enabled with pull-down */
  GPIO->P[0].MODEL = (GPIO->P[0].MODEL & ~_GPIO_P_MODEL_MODE0_MASK) | GPIO_P_MODEL_MODE0_INPUTPULL;

  /* Pin PC0 is configured to Input enabled */
  GPIO->P[2].MODEL = (GPIO->P[2].MODEL & ~_GPIO_P_MODEL_MODE0_MASK) | GPIO_P_MODEL_MODE0_INPUT;
  /* Pin PC1 is configured to Input enabled */
  GPIO->P[2].MODEL = (GPIO->P[2].MODEL & ~_GPIO_P_MODEL_MODE1_MASK) | GPIO_P_MODEL_MODE1_INPUT;
  
  /* set output state of PC0 PC1 (D- D+) to 1 0 */
  GPIO->P[2].DOUTSET = (1<<0); /* D- = 1 */
  GPIO->P[2].DOUTCLR = (1<<1); /* D+ = 0 */

  /* Pin PE13 is configured to Push-pull */
  GPIO->P[4].MODEH = (GPIO->P[4].MODEH & ~_GPIO_P_MODEH_MODE13_MASK) | GPIO_P_MODEH_MODE13_PUSHPULL;

  /* setup PC1 interrupt on rising edge (D+ first SYNC bit) */  
  /* Configure PC1 interrupt on rising edge */
  GPIO_IntConfig(gpioPortC, 1, true, false, true);
  /* Set Interrupt priority to highest */
  NVIC_SetPriority(GPIO_ODD_IRQn, 0);
  /* Enable GPIO_EVEN interrupt vector in NVIC */
  NVIC_EnableIRQ(GPIO_ODD_IRQn);
}

void usb_connect(void)
{
  GPIO->P[0].DOUTSET = (1 << 0);
  GPIO->P[0].MODEL = (GPIO->P[0].MODEL & ~_GPIO_P_MODEL_MODE0_MASK) | GPIO_P_MODEL_MODE0_PUSHPULL;  
}

void usb_disconnect(void)
{
  /* Pin PA0 is configured to Input enabled with pull-down */
  GPIO->P[0].MODEL = (GPIO->P[0].MODEL & ~_GPIO_P_MODEL_MODE0_MASK) | GPIO_P_MODEL_MODE0_INPUTPULL;
  /* set PA0 low (disconnected) Will enable the pulldown */
  GPIO->P[0].DOUTCLR = (1 << 0);
}

bool usb_check_resetcondition(void)
{
  return ( (GPIO->P[2].DIN & 0x03) == 0x00 );
}

/* check if a setup packet was received */
bool usb_setup_available(void)
{
  return (usb_ep_buffers[USB_EPBUF_OFFSET_SETUP_BUF + USB_EPBUF_SUBOFFSET_STAT] & (1 << 0)) != 0x00;
}

/* IDEA: The reordering / Bitreversal can be done in place to save some RAM */
void usb_setup_get_data(setupData_t *psetupdata)
{
  uint32_t i, j;
  uint8_t  dat;
  uint8_t  setupdat[8];
  volatile uint8_t *SETUP_BUF;
  
  SETUP_BUF = &usb_ep_buffers[USB_EPBUF_OFFSET_SETUP_BUF];
  for (i=0; i<8; i++) /* walk through all bytes */
  {
    j = i+1;
    j = (j & 0xfC) | (3-(j & 0x03)); /* reverse byteorder => TODO: This can be done in the .s file using the REV instruction (single cycle) */
    dat = SETUP_BUF[j];
    dat = bitreverse(dat);           /* reverse bits*/
    setupdat[i] = dat;               /* store data */
  }
  usb_ep_buffers[USB_EPBUF_OFFSET_SETUP_BUF + USB_EPBUF_SUBOFFSET_STAT] &= ~(1 << 0); /* flag setup data as empty */
  
  psetupdata->bmRequestType = setupdat[0];
  psetupdata->bRequest      = setupdat[1];
  psetupdata->wValue        = setupdat[2];
  psetupdata->wValue       |= setupdat[3] << 8;
  psetupdata->wIndex        = setupdat[4];
  psetupdata->wIndex       |= setupdat[5] << 8;
  psetupdata->wLength       = setupdat[6];
  psetupdata->wLength      |= setupdat[7] << 8;
}

bool usb_ep_in_buf_empty(uint32_t epnum)
{
  if (epnum)
  {
    return (usb_ep_buffers[USB_EPBUF_OFFSET_EP1IN_BUF + USB_EPBUF_SUBOFFSET_STAT] & (1 << 0)) == 0x00;
  }
  else
  {
    return (usb_ep_buffers[USB_EPBUF_OFFSET_EP0IN_BUF + USB_EPBUF_SUBOFFSET_STAT] & (1 << 0)) == 0x00;
  }
}

/* TODO/IDEA: check parameters like len. This can be done e.g. with ASSERTIONS, so 
 *            that checks can be disabled later to save time & space 
 */
void usb_ep_in_commit_pkt(uint32_t epnum, bool firstpkt, const uint8_t *pbuf, uint32_t len)
{
  uint32_t i;
  uint8_t  dat;
  uint16_t crc_value;
  volatile uint8_t *EP0IN_BUF;
  
  if (!len)
  {
    GPIO->P[1].DOUTSET = (1<<8);
    GPIO->P[1].DOUTCLR = (1<<8);
  }
  
  if (epnum)
  {
    EP0IN_BUF = &usb_ep_buffers[USB_EPBUF_OFFSET_EP1IN_BUF];
  }
  else
  {
    EP0IN_BUF = &usb_ep_buffers[USB_EPBUF_OFFSET_EP0IN_BUF];
  }
  
  if (firstpkt)
  {    
    EP0IN_BUF[0] = 0x4B;   /* first IN packet is always transmitted with DATA1 PID */
  }
  else
  {
    EP0IN_BUF[0] ^= 0x88; /* Following packets toggle between DATA0 and DATA1 PID */
  }
  
  crc_value = 0xffff;
  for (i=0; i<len; i++)
  {
    dat = *pbuf++;
    EP0IN_BUF[i+1] = dat;
    crc_value = crc16(crc_value, dat);
  }
  
  crc_value = ~crc_value; /* this is described in the USB spec. */
  
  /* ADD CRC */
  EP0IN_BUF[1+len+0] = crc_value & 0xff;   /* CRC low */
  EP0IN_BUF[1+len+1] = crc_value >> 8;     /* CRC high */

  /* set packet length */  
  EP0IN_BUF[USB_EPBUF_SUBOFFSET_LEN] = len + 1 +2; /* DATAx PID + CRC16 */
  
  /* Flag endpoint as "ready" for transmission. This needs to be done as the last step */
  EP0IN_BUF[USB_EPBUF_SUBOFFSET_STAT] |= (1 << 0);
}

bool usb_ep_out_data_available(uint32_t epnum)
{
  if (epnum == 0x00)
  {
    return (usb_ep_buffers[USB_EPBUF_OFFSET_EP0OUT_BUF + USB_EPBUF_SUBOFFSET_STAT] & (1 << 0)) != 0x00;
  }
  else
  {
    return (usb_ep_buffers[USB_EPBUF_OFFSET_EP1OUT_BUF + USB_EPBUF_SUBOFFSET_STAT] & (1 << 0)) != 0x00;
  }
}

uint32_t usb_ep_out_get_data(uint32_t epnum, uint8_t *pbuf)
{
  uint32_t i, j, len;
  uint8_t  dat, bytecount;
  volatile uint8_t *EP0OUT_BUF;
    
  if (epnum == 0)
  {
    EP0OUT_BUF = &usb_ep_buffers[USB_EPBUF_OFFSET_EP0OUT_BUF];
  }
  else
  {    
    EP0OUT_BUF = &usb_ep_buffers[USB_EPBUF_OFFSET_EP1OUT_BUF];
  }

  len = EP0OUT_BUF[USB_EPBUF_SUBOFFSET_LEN];
  bytecount = ((len + 7) >> 3) - 1 - 2; /* -PID -CRC16 */
  
  for (i=0; i<bytecount; i++) /* walk through all bytes */
  {
    j = i+1;
    j = (j & 0xfC) | (3-(j & 0x03)); /* reverse byteorder => TODO: This can be done in the .s file using the REV instruction (single cycle) */
    dat = EP0OUT_BUF[j];
    dat = bitreverse(dat);        /* reverse bits*/
    *pbuf++ = dat;                /* store data */
  }

  //bytecount = 0;
  
  if (bytecount > 0)
  {
    volatile int i;
    i=0;
  }

  /* flag endpoint as empty. Needs to be done as last step */
  EP0OUT_BUF[USB_EPBUF_SUBOFFSET_STAT]  &= ~(1 << 0);
  //usb_ep_buffers[USB_EPBUF_OFFSET_EP0OUT_BUF + USB_EPBUF_SUBOFFSET_STAT] &= ~(1 << 0); 

  return bytecount;
}

void usb_ep_stall(uint32_t epnum)
{
  usb_ep_buffers[epnum + USB_EPBUF_SUBOFFSET_STAT] |= (1 << 1); 
}

void usb_ep_unstall(uint32_t epnum)
{
  usb_ep_buffers[epnum + USB_EPBUF_SUBOFFSET_STAT] &= ~(1 << 1); 
}

bool usb_ep_is_stalled(uint32_t epnum)
{
  return (usb_ep_buffers[epnum + USB_EPBUF_SUBOFFSET_STAT] & (1 << 1)); 
}

/* send a package via EP0IN which is longer as 7 bytes */
void usb_ep_in_commit_pkt_long(uint32_t epnum, const uint8_t *pbuf, uint8_t len, bool sendshortpkt)
{
  bool    first;
  uint8_t curlen;
  
  first = (epnum == 0x00); /* EP0IN packets always start with DATA1 PID, while EP1IN PID toggles between DATA0/1 */

  curlen = 0;
  
  while (len)
  {
    curlen = len;
    if (curlen > 8)
    {
      curlen = 8;
    }    
    while (!usb_ep_in_buf_empty(epnum)); /* wait until endpoint buffer is free */
    usb_ep_in_commit_pkt(epnum, first, pbuf, curlen);
    len  -= curlen;
    pbuf += curlen;
    first = false;
  }
  if (!sendshortpkt)
  {
    return;
  }
  if ((curlen == 8) || (curlen == 0)) /* in case the last packet has 8 bytes a short packet needs to be sent to signalize the host, that this was the last packet*/
  { 
    while (!usb_ep_in_buf_empty(epnum));  /* wait until endpoint buffer is free */
    usb_ep_in_commit_pkt(epnum, false, pbuf, 0);
  }
}

/* receive a package via EP0OUT. It is allowed to be longer than 8 bytes */
/* important: ensure, that pbuf has a buffer size with a multiple of 8 Bytes */
/* returns received length */
uint8_t usb_ep_out_get_data_long(uint8_t *pbuf, uint8_t maxlen)
{
  uint8_t curlen, len;

  curlen = 8;
  len = 0;
  do
  {
    if (usb_ep_out_data_available(0))
    {
      curlen = usb_ep_out_get_data(0, pbuf);    /* TODO: Handle this within usb_stack.c. It is part of control transfers. setup */
      pbuf += curlen;
      if (len >= maxlen) /* at least a little bit of protection against buffer overruns */
      {
        pbuf -= curlen;
      }
      len += curlen;
    }    
  } while ( (curlen == 8) && (len < maxlen) ); /* a short packet indicates the last packet */
  return len;
}

/* Handle a Control transfer without data stage => transmits a ACK in status stage */
void usb_control_acknowledge(void)
{
  uint8_t response[8];
  while (!usb_ep_in_buf_empty(0x00))
    ;
  usb_ep_in_commit_pkt(0, true, response, 0);  
}

/* Handle a Control IN transfer */
bool usb_control_dataIn(const uint8_t *pbuf, uint8_t len)
{
  usb_ep_in_commit_pkt_long(0, pbuf, len, false);//true);
  /* wait for status stage - acknowledge from HOST*/
  while ( !usb_ep_out_data_available(0) )
  {    
  }
  usb_ep_buffers[USB_EPBUF_OFFSET_EP0OUT_BUF + USB_EPBUF_SUBOFFSET_STAT] &= ~(1 << 0); /* ignore ACK (OUT packet) */
  return true;
}

/* Handle a Control OUT transfer */
uint8_t usb_control_dataOut(uint8_t *pbuf, uint8_t maxlen)
{
  maxlen = usb_ep_out_get_data_long(pbuf, maxlen);
  usb_control_acknowledge();
  return maxlen;
}



/* TODO: Combine the functions */
