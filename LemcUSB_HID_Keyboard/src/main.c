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

/****************************
|                  _        |
|                 /"\       |
|                /o o\      |
|           _\/  \   / \/_  |
|            \\._/  /_.//   |
|            `--,  ,----'   |
|              /   /        |
|    ^        /    \        |
|   /|       (      )       |
|  / |     ,__\    /__,     |
|  \ \   _//---,  ,--\\_    |
|   \ \   /\  /  /   /\     |
|    \ \.___,/  /           |
|     \.______,/            |
|                           |
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/


#include "em_device.h"
#include "em_cmu.h"
#include "em_chip.h"
#include <stdbool.h>
#include <stdint.h>


#include "usb.h"
#include "usb_stack.h"
#include "usb_hid.h"
#include "uart.h"

#include "c_compat.h"



static void clock_init(void)
{
  uint16_t cmuStatus;
  uint32_t bit, val;
  
  /* set flash waitstates to 1 */
  MSC->READCTRL = (MSC->READCTRL & ~_MSC_READCTRL_MODE_MASK) | MSC_READCTRL_MODE_WS1;

  /* enable crystal oscillator*/
  CMU->OSCENCMD  = CMU_OSCENCMD_HFXOEN;
  while (!(CMU->STATUS & CMU_STATUS_HFXORDY)) /* wait until oscillator is running */
  ;

  cmuStatus = (uint16_t)(CMU->STATUS); /* Keep EMU module informed */

  /* Switch to selected oscillator */
  CMU->CMD = cmuOsc_HFXO;

  cmuStatus = (uint16_t)(CMU->STATUS); /* Keep EMU module informed */

  /* Enable peripheral clock*/
  bit = (cmuClock_GPIO >> CMU_EN_BIT_POS) & CMU_EN_BIT_MASK;
  val = 1;
  CMU->HFPERCLKEN0 = (CMU->HFPERCLKEN0 & ~(1 << bit)) | (val << bit);

}


/* 
 * Initialize Clock (= 24 MHz crystal)
 * Enable globally needed clock domains (GPIO, ...)
 */
static void sys_init(void)
{
  /* Chip errata */
  CHIP_Init();
  
  /* Use crystal oscillator for HFXO */
  CMU->CTRL |= CMU_CTRL_HFXOMODE_XTAL;
  /* HFXO setup */
  CMU->CTRL    = (CMU->CTRL & ~_CMU_CTRL_HFXOBOOST_MASK) | CMU_CTRL_HFXOBOOST_100PCENT;
  
  clock_init();
  
  /* Enable HFXO as high frequency clock, HFCLK */
//  CMU_ClockSelectSet(cmuClock_HF,cmuSelect_HFXO);
 
  /* Enable GPIO clock */
//  CMU_ClockEnable(cmuClock_GPIO, true);
}


bool check_reset(void)
{
  static bool new_resetstate = false, old_resetstate = false;

  old_resetstate = new_resetstate;
  new_resetstate = usb_check_resetcondition();
    
  if (new_resetstate && !old_resetstate) /* This checks for a reset condition on the USB BUS and resets the internal USB state machine */
  {
    int i = 0;
    
    while ( usb_check_resetcondition() && (i < 100) )
    {
      i++;
    }
    if (i >= 100)
    {
      return true;
    }
  }
  return false;
}


/* a small struct for the simulate keypress statemachine */
struct keytyper_s
{
  bool        active;       /* active == true => send the message! */
  const char *pmessage;     /* pointer to complete message string */
  const char *pmessage_cur; /* pointer to next character to be transmitted */
  uint8_t     state;        /* status (0= send make, 1 = send break) */
} message_state;

/* rudimentary conversion routine from ASCII to HID Keyboard scancode
 * return value: 
 * low-byte = HID scancode
 * hi-byte  = 0 => No shift
 * hi-byte !=0  => shift scancode (uppercase character)
 */
static uint16_t hid_get_scancode(char ch)
{
  if ( (ch >= 'A') && (ch <= 'Z') )
  {
    return (ch-'A'+4) | (0x02 << 8);
  } 
  else if ( (ch >= 'a') && (ch <= 'z') )
  {
    return (ch-'a'+4);
  }
  else if ( (ch >= '1') && (ch <= '0') )
  {
    return (ch-'1'+0x1e);
  }
  else if (ch == '.')
    return 0x37 | (0x02 << 8);
  else if (ch == '!')
    return 0x1e | (0x02 << 8);
  else if (ch == '@')
    return 0x1f | (0x02 << 8);
  else 
    return 0x2c; /* => SPACE */
}

static void handle_keyboard(void)
{
  static uint8_t buf[8] = {0, 0, 0, 0, 0, 0, 0, 0};
  
  if (message_state.active && usb_ep_in_buf_empty(1))
  {
    switch (message_state.state)
    {
    case 0:
      buf[0] = hid_get_scancode(*(message_state.pmessage_cur)) >> 8;
      buf[2] = hid_get_scancode(*(message_state.pmessage_cur));
      usb_ep_in_commit_pkt(1, false, buf, 8); /* make code */        
      message_state.state = 1;
      /* send make code */
      break;
    case 1:
    default:
      /* send break code */
      buf[0] = 0;
      buf[2] = 0;
      usb_ep_in_commit_pkt(1, false, buf, 8); /* break code */
      
      message_state.pmessage_cur++;
      if (*(message_state.pmessage_cur) == '\0')
      {
        message_state.active = false;
      }
      message_state.state = 0;        
      break;
    }
  }

}


bool HID_SetReport(uint8_t reportType)
{
  uint8_t buf[8];
  switch (reportType)
  {
    case HID_RPRT_INPUT:
      return false;
      break;
    case HID_RPRT_OUTPUT:
      usb_control_dataOut(buf, 8); /* read report from PC */
      return true;
      break;
    case HID_RPRT_FEATURE:
      return true;
      break;
  }
  
  return false;
}
bool HID_GetReport(uint8_t reportType)
{
  switch (reportType)
  {
    case HID_RPRT_INPUT:
      return false;
      break;
    case HID_RPRT_OUTPUT:
      //usb_control_dataOut(buf, 8); /* read report from PC */
      return true;
      break;
    case HID_RPRT_FEATURE:
      //usb_control_dataIn(hid_responsebuffer, 8);
      return true;
      break;
  }
  
  return false;
}

bool HID_GetIdle(uint8_t *pIdleTime)
{
  return false;
}

bool HID_SetIdle(uint8_t   idleTime)
{
  return true;
}

bool HID_GetProtocol(uint8_t *pProtocol)
{
  return false;
}

bool HID_SetProtocol(uint8_t   protocol)
{
  return false;
}



void main(void)
{ 
  setupData_t setupdata;
  uint32_t timeoutcounter = 0;
  
  const char *msg = "This is a message typed by LemcUSB! Hello World!\0";
  
  sys_init();
  usb_init();    
  
  
  
  message_state.active = false;
  message_state.pmessage = msg;
  
  
  
  /* let all the magic happen within the ISRs... */
  usb_connect();
  
  while(true)
  {

    if (usb_setup_available())
    {
      usb_setup_get_data(&setupdata);
      switch( setupdata.bmRequestType & USB_SETUP_BM_TYPE )
      {
        case USB_SETUP_TYPE_STANDARD:
          usbstack_got_setup_cmd(&setupdata); 
          timeoutcounter = 0; /* don't send keystrokes during enumeration */
          break;
        case USB_SETUP_TYPE_CLASS:
          usbhid_got_setup_cmd(&setupdata);
          break;
        case USB_SETUP_TYPE_VENDOR:
        default:
        break;
      }
    }
    
    if (check_reset())
    {
      usb_reset_received();
    }
    
    if (timeoutcounter++ > 1000000ul)
    {
      message_state.active = true;
      message_state.pmessage_cur = message_state.pmessage;
      message_state.state = 0;
      timeoutcounter = 0;
    }
    
    /* simulate keypresses */
    handle_keyboard();
    
  }
}
