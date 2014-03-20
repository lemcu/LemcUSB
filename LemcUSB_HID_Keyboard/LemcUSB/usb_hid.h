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

#ifndef __USB_HID_H__
#define __USB_HID_H__

#include <stdint.h>
#include <stdbool.h>
#include "usb.h"
#include "usb_config.h"

#ifdef USB_ENABLE_HID


/* HID Report Types */
#define HID_RPRT_INPUT               (0x01)
#define HID_RPRT_OUTPUT              (0x02)
#define HID_RPRT_FEATURE             (0x03)


/* HID Requests functions, need to be defined by the user! */
/* They are not implemented as callback functions to get */
/* a more efficient code.*/
/* The functions should false, if they were not handled */
/* return TRUE if function was handled */
extern bool HID_GetReport   (uint8_t   reportType);
extern bool HID_SetReport   (uint8_t   reportType);
extern bool HID_GetIdle     (uint8_t *pIdleTime);
extern bool HID_SetIdle     (uint8_t   idleTime);
extern bool HID_GetProtocol (uint8_t *pProtocol);
extern bool HID_SetProtocol (uint8_t   protocol);



bool usbhid_got_setup_cmd(const setupData_t *psetupdata);



#else
#warning "HID disabled, but hid includes compiled"
#endif

#endif
