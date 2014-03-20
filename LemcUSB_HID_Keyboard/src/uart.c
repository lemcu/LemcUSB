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

#include "uart.h"
#include "em_device.h"
#include "em_usart.h"
#include "em_cmu.h"


  
void uart_init(void)
{
  
  
  /* To avoid false start, configure output US1_TX as high on PD7 */
  GPIO->P[3].DOUT |= (1 << 7);
  /* Pin PD7 is configured to Push-pull */
  GPIO->P[3].MODEL = (GPIO->P[3].MODEL & ~_GPIO_P_MODEL_MODE7_MASK) | GPIO_P_MODEL_MODE7_PUSHPULL;
  
  /* Enable clock for USART1 */
  CMU_ClockEnable(cmuClock_USART1, true);
  
  
  
  USART_InitAsync_TypeDef init = USART_INITASYNC_DEFAULT;

  init.baudrate     = 2000000;
  init.oversampling = usartOVS4;
  init.databits     = usartDatabits8;
  init.parity       = usartNoParity;
  init.stopbits     = usartStopbits1;
  init.mvdis        = 0;
  init.prsRxEnable  = 0;

  USART_InitAsync(USART1, &init);
  
  /* Module USART1 is configured to location 2 */
  USART1->ROUTE = (USART1->ROUTE & ~_USART_ROUTE_LOCATION_MASK) | USART_ROUTE_LOCATION_LOC2;
  /* Enable signal TX */
  USART1->ROUTE |= USART_ROUTE_TXPEN;

  
}