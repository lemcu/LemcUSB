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

#ifndef __USB_CONFIG_H__
#define __USB_CONFIG_H__

/* Set to 1 to enable 2 additional endpoints.
   If set to 0, only control transfers will be supported, because only EP0OUT and EP0IN are present
*/
#define USB_ENABLE_EP1OUT_EP1IN    (1)

#define USB_ENABLE_HID

/*
 Enable this define, if the USB low level protocol handling should be done in RAM.
 This is e.g. mandatory if Flash programming is used, otherwise USB will not be
 responsive anymore when flash is erased or programmed, because Flash accesses will
 be stalled.
 */
#define USB_PROTOCOL_HANDLING_IN_RAM

#endif
