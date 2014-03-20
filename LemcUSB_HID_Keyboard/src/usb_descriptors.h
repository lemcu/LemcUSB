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

#ifndef __USB_DESCRIPTORS_H__
#define __USB_DESCRIPTORS_H__

#include <stdint.h>
#include <stdbool.h>
#include "usb.h"

/* include this file only once! */

#define WBVAL(x) (x & 0xFF),((x >> 8) & 0xFF)


/* report descriptor for HID keyboard */
const uint8_t hid_report_descriptor[] =
{
  0x05, 0x01, 0x09, 
  0x06, 0xA1, 
  0x01, 0x05, 
  0x07, 0x19, 
  0xE0, 0x29, 0xE7, 
  0x15, 0x00, 
  0x25, 0x01,
  0x75, 0x01, 0x95, 0x08, 0x81, 0x02, 0x95, 0x01,
  0x75, 0x08, 0x81, 0x01, 0x95, 0x03, 0x75, 0x01,
  0x05, 0x08, 0x19, 0x01, 0x29, 0x03, 0x91, 0x02,
  0x95, 0x05, 0x75, 0x01, 0x91, 0x01, 0x95, 0x06,
  0x75, 0x08, 0x15, 0x00, 0x26, 0xFF, 0x00, 0x05,
  0x07, 0x19, 0x00, 0x2A, 0xFF, 0x00, 0x81, 0x00,
  0xC0
};


const uint8_t device_descriptor[] = 
{
  0x12,                         /* descriptor length */
  0x01,                         /* descriptor type */
  WBVAL(0x0110),                /* USB version */
  0x00,                         /* Devices class */
  0x00,                         /* Device sub class */
  0x00,                         /* Device sub sub class */
  0x08,                         /* max packet size (for EP0IN/OUT) */
  WBVAL(0x10C4),                /* VID (low, high) */
  WBVAL(0x8944),                /* PID */
  WBVAL(0x0100),                /* DID */
  0x01,                         /* manufacturer string index */
  0x02,                         /* product string index */
  0x00,                         /* serial number string index */
  0x01                          /* number of configurations */
};

const uint8_t config_0_descriptor[] = 
{
  0x09,                         /* length of config descriptor */
  DESCRIPTOR_CONFIGURATION,     /* descriptor type */
  0x12+7+9, 0x00,               /* Config + Interface + Endpoints length (low, high) */
  0x01,                         /* number of interfaces */
  0x01,                         /* interface number */
  0x00,                         /* configuration string index */
  0xA0,                         /* attributes (buspowered = Bit7, Selfpowered = Bit6, RemoteWakeup = Bit 5) */
  50,                           /* power requirement (multiply by 2 to get current in mA) */
  
  /* interface descriptor */
  0x09,                         /* length of interface descriptor */
  DESCRIPTOR_INTERFACE,         /* descriptor type */
  0x00,                         /* Index of this interface (starts at 0) */
  0x00,                         /* Alternate setting */
  0x01,                         /* Number of Endpoints */
  0x03,                         /* Interface class */
  0x01,                         /* Interface sub class */
  0x01,                         /* Interface sub sub class */
  0x00,                         /* Interface descriptor string index */
  
  /* HID descriptor - Keyboard */
  0x09,                         /* Length of this descriptor */
  DESCRIPTOR_HID,                             /* bDescriptorType */
  WBVAL(0x0100),          /* 1.00 */          /* bcdHID */  
  0x00,                                       /* bCountryCode */
  0x01,                                       /* bNumDescriptors */
  0x22,                                       /* bDescriptorType */
  WBVAL(sizeof(hid_report_descriptor)),       /* wDescriptorLength */
  
  /* EP1IN */
  0x07,                         /* length of endpoint descriptor */
  DESCRIPTOR_ENDPOINT,          /* descriptor type */
  0x81,                         /* Endpoint number and direction (Bit 7) */
  EPTYPE_INTERRUPT,             /* Endpoint type */
  0x08, 0x00,                   /* Max packet size of this Endpoint (Low, High) */
  0x18                          /* Polling Interval */
};

const uint8_t string_0_descriptor[] = 
{
  0x04,                         /* length of string descriptor */ 
  DESCRIPTOR_STRING,            /* descriptor type */
  0x09, 
  0x04
};

const uint8_t string_1_descriptor[] = 
{
  0x14,                         /* length of string descriptor */ 
  DESCRIPTOR_STRING,            /* descriptor type */
  'L', 0,  
  'E', 0, 
  'M', 0, 
  'C', 0, 
  'U', 0, 
  '.', 0, 
  'O', 0, 
  'R', 0, 
  'G', 0, 
};

const uint8_t string_2_descriptor[] = 
{
  0x1A,                         /* length of string descriptor */ 
  DESCRIPTOR_STRING,            /* descriptor type */
  'I', 0,
  't', 0,
  ' ', 0,
  'w', 0,
  'o', 0,
  'r', 0,
  'k', 0,
  's', 0,
  ' ', 0,
  ':', 0,
  '-', 0,
  ')', 0,
};

const uint8_t hid_descriptor[] = 
{
  0x09,
  DESCRIPTOR_HID,
  0x10, 0x01,                   /* USB version */
  0x00,                         /* Localization*/
};


#endif
