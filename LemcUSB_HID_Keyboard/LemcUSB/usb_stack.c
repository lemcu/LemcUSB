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
#include "usb_descriptors.h"
#include "usb_helperfunctions.h"

#ifdef USB_ENABLE_HID          
#include "usb_hid.h"
#endif


/* Standard Request codes for Control */
#define SRQ_GET_STATUS		(0x00)	/* Standard Request : Get Status */
#define SRQ_CLEAR_FEATURE	(0x01)	/* Standard Request : Clear Feature */
#define SRQ_RESERVED1		(0x02)	/* Standard Request : Reserved for future use */
#define SRQ_SET_FEATURE		(0x03)	/* Standard Request : Set Feature */
#define SRQ_RESERVED2           (0x04)  /* Standard Request : Reserved for future use */
#define SRQ_SET_ADDRESS		(0x05)	/* Standard Request : Set Address */
#define SRQ_GET_DESCRIPTOR	(0x06)	/* Standard Request : Get Descriptor */
#define SRQ_SET_DESCRIPTOR	(0x07)	/* Standard Request : Set Descriptor */
#define SRQ_GET_CONFIGURATION	(0x08)	/* Standard Request : Get Configuration */
#define SRQ_SET_CONFIGURATION	(0x09)	/* Standard Request : Set Configuration */
#define SRQ_GET_INTERFACE	(0x0a)	/* Standard Request : Get Interface */
#define SRQ_SET_INTERFACE	(0x0b)	/* Standard Request : Set Interface */
#define SRQ_SYNCH_FRAME		(0x0c)	/* Standard Request : Synch Frame */

/* Status type codes used in SRQ_GET_STATUS request */
#define STATUS_DEVICE		(0x80)	/* Get Status: Device */
#define STATUS_INTERFACE	(0x81)	/* Get Status: Interface */
#define STATUS_ENDPOINT		(0x82)	/* Get Status: End Point */

/* Feature type codes used in SRQ_SET_FEATURE request */
#define FEATURE_DEVICE		(0x00)	/* Feature: Device */
#define FEATURE_ENDPOINT	(0x02)	/* Feature: End Point */





       uint8_t  usb_dev_address;         /* default address is 0x00 TODO: Reset to zero when a reset condition is present on BUS (SE0 long duration) */
static uint8_t  usb_dev_configuration;
static uint8_t  usb_dev_alternatesetting;
static uint8_t  usb_dev_Rwuen;
static uint8_t  usb_dev_selfpwr;




void usbstack_init(void)
{
  usb_dev_address          = 0x00;
  usb_dev_configuration    = 0;
  usb_dev_alternatesetting = 0;
  usb_dev_Rwuen            = 0;
  usb_dev_selfpwr          = 0;
}


bool usbstack_got_setup_cmd(const setupData_t *psetupdata)
{
  uint8_t response[8];
  uint16_t clen;
  bool     handled;
  
  handled = true;

  switch(psetupdata->bRequest)
  {
    case SRQ_SET_ADDRESS:                         /* This needs to be done within 50ms after receiving the setup packet! */
      usb_control_acknowledge();
      while (!usb_ep_in_buf_empty(0));            /* Wait until the complete setup transfer is finished, before setting the usb device address */
      usb_dev_address = bitreverse(psetupdata->wValue);    
      /* set Address does not have an OUT stage */
      break;
    case SRQ_GET_DESCRIPTOR:						// *** Get Descriptor
      switch (psetupdata->wValue >> 8)			
      {
        case DESCRIPTOR_DEVICE:				// Device
          clen = psetupdata->wLength;
          if (clen > sizeof(device_descriptor)) clen = sizeof(device_descriptor);
          usb_control_dataIn(device_descriptor, clen);
          break;
        case DESCRIPTOR_CONFIGURATION:			// Configuration setupdat[2] contains configuration number
          clen = psetupdata->wLength;
          if (clen > sizeof(config_0_descriptor)) 
          {
            clen = sizeof(config_0_descriptor);
          }
          usb_control_dataIn(config_0_descriptor, clen); /* setupdat[2] specifies configurationnumber, currently fixed to 0 */
          
          if (clen == 0x09)
          {
            volatile int i;
            i++;
          }
          
          break;
        case DESCRIPTOR_STRING:				// String setupdat[2] contains string index
          switch (psetupdata->wValue & 0xff)
          {
            case 0:
              clen = psetupdata->wLength;
              if (clen > sizeof(string_0_descriptor)) clen = sizeof(string_0_descriptor);
              usb_control_dataIn(string_0_descriptor, clen);
              break;
            case 1:
              clen = psetupdata->wLength;
              if (clen > sizeof(string_1_descriptor)) clen = sizeof(string_1_descriptor);
              usb_control_dataIn(string_1_descriptor, clen);
              break;
            case 2:
              clen = psetupdata->wLength;
              if (clen > sizeof(string_2_descriptor)) clen = sizeof(string_2_descriptor);
              usb_control_dataIn(string_2_descriptor, clen);
              break;
            default:
              usb_control_dataIn(response, 0);
              break;
              /* INFO: more/less string descriptors can be used. String descriptors are not mandatory! */
          }
          break;
#ifdef USB_ENABLE_HID          
        case DESCRIPTOR_REPORT:
          clen = psetupdata->wLength;
          if (clen > sizeof(hid_report_descriptor)) clen = sizeof(hid_report_descriptor);
          usb_control_dataIn(hid_report_descriptor, clen);
          break;          
#endif
        default:
          usb_ep_stall(USB_EP0OUT);
          usb_ep_stall(USB_EP0IN);
          handled = false;
      }
      break;
    case SRQ_GET_INTERFACE:				
      response[0] = usb_dev_alternatesetting;
      usb_control_dataIn(response, 1);
      break;
    case SRQ_SET_INTERFACE:				
      usb_dev_alternatesetting = psetupdata->wValue;
      usb_control_acknowledge();
      break;
    case SRQ_SET_CONFIGURATION:				
      usb_dev_configuration = psetupdata->wValue;
      usb_control_acknowledge();
      break;
    case SRQ_GET_CONFIGURATION:				
      response[0] = usb_dev_configuration;
      usb_control_dataIn(response, 1);
      break;
    case SRQ_GET_STATUS:				
      switch(psetupdata->bmRequestType)
      {
        case STATUS_DEVICE:				
          response[0] = (usb_dev_Rwuen << 1) | usb_dev_selfpwr;
          response[1] = 0;
          usb_control_dataIn(response, 2);
          break;
        case STATUS_INTERFACE:			
          response[0] = 0;
          response[1] = 0;
          usb_control_dataIn(response, 2);
          break;
        case STATUS_ENDPOINT:			
          response[0] = 0; /* TODO: return if endpoint is stalled */
          response[1] = 0;
          usb_control_dataIn(response, 2);
          break;
        default:
          usb_ep_stall(USB_EP0OUT);
          usb_ep_stall(USB_EP0IN);
          handled = false;          
      }
      break;
    case SRQ_CLEAR_FEATURE:			
      switch(psetupdata->bmRequestType)
      {
        case FEATURE_DEVICE:			
          if(psetupdata->wValue == 1)
            usb_dev_Rwuen = 0; 		/* Disable Remote Wakeup */ 
          else
          {
            usb_ep_stall(USB_EP0OUT);
            usb_ep_stall(USB_EP0IN);
          }
          usb_control_acknowledge();
          break;
        case FEATURE_ENDPOINT:		
          if ( (psetupdata->wValue & 0x0f) == 0 )
          {
            usb_ep_unstall(USB_EP0OUT);
            usb_ep_unstall(USB_EP0IN);
          }
          else
          {
            usb_ep_unstall(USB_EP1OUT);
            usb_ep_unstall(USB_EP1IN);
          }
          usb_control_acknowledge();
          break;
      }
      break;
    case SRQ_SET_FEATURE:			
      switch(psetupdata->bmRequestType)
      {
        case FEATURE_DEVICE:			
          if (psetupdata->wValue == 1)
            usb_dev_Rwuen = 1; /* enable remote wakeup */
          else
          {
            usb_ep_stall(USB_EP0OUT);
            usb_ep_stall(USB_EP0IN);
          }
          break;
        case FEATURE_ENDPOINT:			
          if ( (psetupdata->wValue & 0x0f) == 0 )
          {
            usb_ep_stall(USB_EP0OUT);
            usb_ep_stall(USB_EP0IN);
          }
          else
          {
            usb_ep_stall(USB_EP1OUT);
            usb_ep_stall(USB_EP1IN);
          }
          break;
      }
      break;
    default:
      usb_ep_stall(USB_EP0OUT);
      usb_ep_stall(USB_EP0IN);
      handled = false;
  }
  
  
  return handled;  
}
