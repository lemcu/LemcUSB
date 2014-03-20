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

#ifndef __USB_STACK_H__
#define __USB_STACK_H__

#include <stdint.h>
#include <stdbool.h>
#include "usb.h"


/* These defines can be used to mask out the bmRequestType bits */
#define   USB_SETUP_BM_DIRECTION        (0x80)
#define   USB_SETUP_BM_TYPE             (0x60)
#define   USB_SETUP_BM_RECIPIENT        (0x1f)

#define   USB_SETUP_DIR_DEV_TO_HOST     (0x80)

#define   USB_SETUP_TYPE_STANDARD       (0 << 5)
#define   USB_SETUP_TYPE_CLASS          (1 << 5)
#define   USB_SETUP_TYPE_VENDOR         (2 << 5)

#define   USB_SETUP_DEVICE              (0x00)
#define   USB_SETUP_INTERFACE           (0x01)
#define   USB_SETUP_ENDPOINT            (0x02)
#define   USB_SETUP_OTHER               (0x03)

void usbstack_init(void);


/* call this function when setup data was received 
   TODO: allow user to call this also, when no setup data was received.
         I think the best way is to let the user only call functions from this file and not from usb.h */
bool usbstack_got_setup_cmd(const setupData_t *psetupdata);




#endif