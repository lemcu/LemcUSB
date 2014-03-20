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

#ifndef __USB_HELPERFUNCTIONS_H__
#define __USB_HELPERFUNCTIONS_H__

#include <stdint.h>

/* from bit twiddling hacks. Did not check efficiency.
   My guess is, that the standard assembly way using 8 consecutive ROR/ROL instructions is faster!
so: TODO check and optimize
*/
static inline uint8_t bitreverse(uint8_t b)
{
  return ((b * 0x0802LU & 0x22110LU) | (b * 0x8020LU & 0x88440LU)) * 0x10101LU >> 16; 
}


/* TODO: Make this nice and efficient! Maybe implement this directly in CM0+ assembly 
         using uint32_t instead of uint8_t can also speed things up
         See: http%3A%2F%2Fwww.usb.org%2Fdevelopers%2Fwhitepapers%2Fcrcdes.pdf&ei=
              AKrAUtuBD4WMtQakyoCwAQ&usg=AFQjCNE71tY6EAUjJKXWbjtbNvDTiOKKlg&bvm=bv.
              58187178,d.Yms&cad=rja
 */
static uint16_t crc16(uint16_t crc, uint8_t c)
{
  uint8_t  i;
  
  for (i=0; i<8; i++)
  {
    if ((crc ^ c) & 1) 
    { 
      crc = (crc >> 1) ^ 0xa001; /* 0xa001*/
    } 
    else 
    {
      crc >>= 1;
    }
    c >>= 1;
  }
  return crc;
}


#endif
