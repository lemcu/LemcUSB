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

#include "usb_stack.h"
#include "usb.h"
#include "usb_helperfunctions.h"
#include "usb_config.h"
#include "usb_hid.h"

#include "em_device.h" /* removeme. Only needed for debugging */



#ifdef USB_ENABLE_HID

/* HID requests */
#define HID_GET_REPORT		(0x01)	             /* mandatory */
#define HID_GET_IDLE		(0x02)	             /* optional */
#define HID_GET_PROTOCOL	(0x03)	             /* mandatory for BOOT devices */
#define HID_SET_REPORT		(0x09)	             /* mandatory if USB device does not use EP1OUT */
#define HID_SET_IDLE		(0x0a)	             /* optional */
#define HID_SET_PROTOCOL	(0x0b)	             /* mandatory for BOOT devices */





bool usbhid_got_setup_cmd(const setupData_t *psetupdata)
{
  uint8_t response[8];
  bool    doStall;
  
  doStall = false;

  switch(psetupdata->bRequest)
  {
    case HID_GET_REPORT:
      doStall = !HID_GetReport(psetupdata->wValue >> 8);
      break;
    case HID_SET_REPORT:      
      doStall = !HID_SetReport(psetupdata->wValue >> 8);
      if (!doStall)
      {
        //usb_control_acknowledge(); //TODO: REMOVE UNCOMMENT!!!
      }
      break;    
    case HID_GET_PROTOCOL:
      doStall = !HID_GetProtocol(response);
      if (!doStall)
      {
        usb_control_dataIn(response, 1);
      }
      break;      
    case HID_SET_PROTOCOL:
      doStall = !HID_SetProtocol(psetupdata->wValue >> 8); /* low byte of value is interface number */
      if (!doStall)
      {
        usb_control_acknowledge();
      }
      break;
    case HID_GET_IDLE:
      doStall = !HID_GetIdle(response);
      if (!doStall)
      {
        usb_control_dataIn(response, 1);      
      }
      break;
    case HID_SET_IDLE:
      doStall = !HID_SetIdle(psetupdata->wValue >> 8);  /* low byte of value is interface number */
      if (!doStall)
      {
        usb_control_acknowledge(); 
      }
      break;
  }
  
  if (doStall)
  {
    usb_ep_stall(USB_EP0OUT);
    usb_ep_stall(USB_EP0IN);
  }
  return true;
}


#endif
