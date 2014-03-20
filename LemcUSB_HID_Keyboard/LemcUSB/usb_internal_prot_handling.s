                      //****************************************************************************
                      //
                      // Lemcusb - Firmware USB driver for EFM32 Microcontroller
                      // Copyright (C) 2014 http://lemcu.org
                      //
                      // This library is free software: you can redistribute it and/or modify
                      // it under the terms of the GNU General Public License version 3.0 as
                      // published by the Free Software Foundation and appearing in the file
                      // LICENSE.txt included in the packaging of this file.
                      //
                      // In addition, as a special exception, http://lemcu.org gives you certain
                      // additional rights. These rights are described in the lemcu.org GPL
                      // Exception version 1.0, included in the file GPL_EXCEPTION.txt in this
                      // package.
                      //
                      // This library is distributed in the hope that it will be useful,
                      // but WITHOUT ANY WARRANTY// without even the implied warranty of
                      // MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
                      //
                      //**************************************************************************/

                      // This code can reside in Flash
                      // It is not that timing critical and placing it in Flash
                      // saves RAM.
                      
                      // Purpose of this code:
                      // It is called for each USB packet received from Host
                      //
                      // It handles the request and calls the transmit function
                      // to return a proper response.
                      //
                      // The implementation is done in Assembler, to save
                      // some overhead to save the registers in memory, etc...
                      // => reduce latency of response

#include "usb_config.h"

#if defined ( __IAR_SYSTEMS_ASM__ )
                      NAME usb_internal_prot_handling
	#ifdef USB_PROTOCOL_HANDLING_IN_RAM
                      #define SHT_PROGBITS 0x1
                      #define SHF_WRITE 0x1
                      #define SHF_EXECINSTR 0x4
                      RSEG MYCODE:CODE:NOROOT(2)
                      SECTION_TYPE SHT_PROGBITS, SHF_WRITE | SHF_EXECINSTR
                      THUMB
	#else
                      section CODE:CODE (2)
                      CODE
	#endif

                      THUMB

                      EXTERN    usb_dev_address
                      EXTERN    USB_PROT_STATE
                      EXTERN    usb_ep_buffers
                      EXTERN    protocol_handled
                      EXTERN    usb_transmit


                      PUBLIC    usb_protocol_handle
#else
                      .syntax unified
                      .arch armv6-m
	#ifdef USB_PROTOCOL_HANDLING_IN_RAM
                      .THUMB
                      .section .functioninRAM
	#else
                      .BALIGN 4
                      .TEXT
	#endif
                      .THUMB

                      .EXTERN    usb_dev_address
                      .EXTERN    USB_PROT_STATE
                      .EXTERN    usb_ep_buffers
                      .EXTERN    protocol_handled
                      .EXTERN    usb_transmit


                      .GLOBAL    usb_protocol_handle

    				  .type    usb_protocol_handle, %function   // IMPORTANT FOR GCC to get lsb of address set to 1

#endif


                      

                      /* Endpoint buffer offsets and suboffsets*/
                      #define  USBEP_OFS_SETUP             ( 0)
                      #define  USBEP_OFS_EP0OUT            (16)
                      #define  USBEP_OFS_EP0IN             (32)
                      #define  USBEP_OFS_EP1OUT            (48)
                      #define  USBEP_OFS_EP1IN             (64)
                      #define  USBEP_SOFS_STAT             (12)
                      #define  USBEP_SOFS_LEN              (13)
                      
                      
                      /* some internal C-Style MACROs */
                      #define REV2(x)  ((((x)&1)<<1) | (((x)>>1)&1))
                      #define REV4(x)  ((REV2(x)<<2) | (REV2((x)>>2)))
                      #define REV8(x)  ((REV4(x)<<4) | (REV4((x)>>4)))
                      #define CREATE_PID(x) (x & 0x0F) | (((~x) & 0x0f)<<4)
                      
                      /* define Bitreversed PIDs */
                      #define USB_TOKEN_BR_OUT              REV8(CREATE_PID( 0x01 ))
                      #define USB_TOKEN_BR_IN               REV8(CREATE_PID( 0x09 ))
                      #define USB_TOKEN_BR_SETUP            REV8(CREATE_PID( 0x0d ))
                      #define USB_TOKEN_BR_DATA0            REV8(CREATE_PID( 0x03 ))
                      #define USB_TOKEN_BR_DATA1            REV8(CREATE_PID( 0x0b ))
                      #define USB_TOKEN_BR_ACK              REV8(CREATE_PID( 0x02 ))
                      #define USB_TOKEN_BR_NAK              REV8(CREATE_PID( 0x0a ))
                      
                      /* define PIDs used for transmit (not bitreversed) */
                      #define USB_TOKEN_ACK                 CREATE_PID( 0x02 )
                      #define USB_TOKEN_NAK                 CREATE_PID( 0x0a )
                      #define USB_TOKEN_STALL               CREATE_PID( 0x0e )
                      

                      
                      // The following registers are saved externally on stack:
                      // r0-r11
                      // Registers that are not allowed to be modified:
                      // r12, LR
                      
                      // Input data:
                      // -----------
                      // r11 = received bitcount
                      // r8...r10 = received data
                      //

                      // Output data:
                      // ------------
                      // none

                      // A complete packet was received.
                      // Now do some protocol handling
                      // 1.) Identify PID R8.24-31
usb_protocol_handle:

                      LDR     r7, ASM_USB_PROT_STATE      // r7 = pointer to protocol state variable
                      LDRB    r6, [r7]                    // r6 = state machine state
                      LDRB    r5, [r7, #1]                // r5 = Address from state variable
                      LDRB    r4, [r7, #2]                // r4 = Endpoint from state variable
                      LDR     r3, ASM_USB_DEV_ADDRESS
                      LDRB    r3, [r3]                    // r3 = USB Addresss of this device
                      MOV     r0, r8
                      LSRS    r0, r0, #24                 // r0 now contains the PID only
                      CMP     r0, #USB_TOKEN_BR_SETUP     // sends address and endpoint as parameter
                      BEQ     pid_setup
                      CMP     r0, #USB_TOKEN_BR_DATA0
                      BEQ     pid_data0
                      CMP     r0, #USB_TOKEN_BR_DATA1
                      BEQ     pid_data1
                      CMP     r0, #USB_TOKEN_BR_IN        // sends address and endpoint as parameter
                      BEQ     pid_in
                      CMP     r0, #USB_TOKEN_BR_OUT       // sends address and endpoint as parameter
                      BEQ     pid_out
      
jump_exit:            LDR     r0, LBL_PROT_HANDLED
                      BX      r0
                      


////////////// Protocol handling statemachine
                      // r7 = pointer to protocol state variable
                      // r6 = state machine state (Byte 0)
                      // r5 = Address & endpoint from state variable (Byte 1)
pid_setup:            MOVS    r0, #0
                      STRB    r0, [r7]                   // Flag, that the next data package is a setup packet
                      MOV     r0, r8
                      // extract address
                      LSRS    r1, r0, #16
                      MOVS    r2, #0xfe
                      ANDS    r1, r1, r2                 // Mask out address bits a0-a6. Order: a0 a1 a2 a3 a4 a5 a6 0
                      STRB    r1, [r7, #1]               // Store address in protocol state

                      // extract endpoint number
                      LSRS    r1, r0, #9
                      MOVS    r2, #0xf0
                      ANDS    r1, r1, r2
                      STRB    r1, [r7, #2]               // Store endpoint in protocol state => TODO: APPLY MASK! Bitorder e0 e1 e2 e3 0 0 0 0
                      
                      B       jump_exit

pid_data0:            
pid_data1:            
                      // Check the usb address
                      CMP     r5, r3
                      BNE     jump_exit                  // This device is not addressed => Don't react on DATA PID

                      CMP     r6, #0                     // check if this is data is a setup phase or out phase
                      BEQ     pid_data_setup
                      
                      // Here: data PID is for OUT phase
                      
                      // Create pointer to addressed endpoint buffer
                      LDR     r6, ASM_USB_EP_BUFFERS
                      
                      LDRB    r1, [r7, #2]               // Store endpoint in protocol state => TODO: APPLY MASK! Bitorder e0 e1 e2 e3 0 0 0 0
                      CMP     r1, #0
                      BEQ     pid_da_ep0_addressed
                        ADDS    r6, r6, #USBEP_OFS_EP1OUT 
                      B       pid_da_ep1_addressed
pid_da_ep0_addressed:   ADDS    r6, r6, #USBEP_OFS_EP0OUT 
pid_da_ep1_addressed: 
                      
                      LDRB    r0, [r6, #USBEP_SOFS_STAT]
                      // check if endpoint is stalled
                      MOVS    r2, #2
                      TST     r0, r2
                      BNE     send_STALL
                      
                      // check if endpoint contains data
                      MOVS    r1, #1
                      TST     r0, r1                    // if equal => Endpoint buffer is free
                      BNE     send_NAK
                      
                      // here: buffer available and endpoint not stalled, so copy data to buffer...
                      MOV     r0, r8
                      MOV     r1, r9
                      MOV     r2, r10
                      MOV     r5, r6                     // generate address to EP0buffer data
                      STM     r5!, {r0, r1, r2}          // copy data to EP0OUT buffer
                      MOV     r0, r11                    // EP0OUT_LEN is filled with the received bitcount (includes PID & CRC)
                      STRB    r0, [r6, #USBEP_SOFS_LEN]
                      
                      LDRB    r1, [r6, #USBEP_SOFS_STAT]                      
                      MOVS    r0, #1    
                      ORRS    r1, r1, r0
                      STRB    r1, [r6, #USBEP_SOFS_STAT] // set "data available" flag for EP0OUT Buffer
                      B       send_ACK
                      
pid_data_setup:       LDR     r6, ASM_USB_EP_BUFFERS          // copy setup data to buffer
                      MOV     r0, r8
                      MOV     r1, r9
                      MOV     r2, r10
                      MOV     r5, r6                     // generate address to EP0buffer data
                      ADDS    r5, r5, #USBEP_OFS_SETUP
                      STM     r5!, {r0, r1, r2}          // copy data to SETUP buffer
                      LDRB    r1, [r6, #(USBEP_OFS_SETUP + USBEP_SOFS_STAT)] 
                      MOVS    r0, #1          
                      ORRS    r1, r1, r0
                      STRB    r1, [r6, #(USBEP_OFS_SETUP + USBEP_SOFS_STAT)] // set "data available" flag for SETUP BUFFER
                      
                      // SEND ACK... This is the only option. NAK is not allowed in a setup phase
                      B       send_ACK


pid_in:               MOVS    r0, #2
                      STRB    r0, [r7]                   // Flag, that an IN PID was received
                      
                      // extract address
                      MOV     r0, r8                      
                      LSRS    r1, r0, #16
                      MOVS    r2, #0xfe
                      ANDS    r1, r1, r2                 // Mask out address bits a0-a6. Order: a0 a1 a2 a3 a4 a5 a6 0
                      STRB    r1, [r7, #1]               // Store address in protocol state

                      // check if USB Address is for this device
                      CMP     r1, r3
                      BNE     jump_exit                  // This device is not addressed => Don't react on DATA PID
                      
                      // extract endpoint number
                      LSRS    r1, r0, #9
                      MOVS    r2, #0xf0
                      ANDS    r1, r1, r2
                      STRB    r1, [r7, #2]               // Store endpoint in protocol state => TODO: APPLY MASK! Bitorder e0 e1 e2 e3 0 0 0 0
                      
                      // build pointer to correct endpoint buffer
                      LDR     r5, ASM_USB_EP_BUFFERS
                      CMP     r1, #0
                      BEQ     pid_in_ep0_addressed
                        ADDS    r5, r5, #USBEP_OFS_EP1IN 
                      B       pid_in_ep1_addressed
pid_in_ep0_addressed:   ADDS    r5, r5, #USBEP_OFS_EP0IN 
pid_in_ep1_addressed: 
                      
                      // check stall condition
                      LDRB    r0, [r5, #USBEP_SOFS_STAT]    // Bit0 = 0 = no data available => send NAK, Bit 0 = 1 = data availabe => send data
                      MOVS    r2, #2
                      TST     r0, r2
                      BNE     send_STALL                 // Check and handle Endpoint STALL condition
                      
                      // check if data is available in addressed endpoint
                      MOVS    r1, #1                      
                      TST     r0, r1
                      BEQ     send_NAK
                      // send Data. Important: First byte contains correct DATA PID (DATA0/DATA1)
                      
                      PUSH    {r5}
                      LDM     r5!, {r0, r1, r2}
                      LDRB    r5, [r5, #1]               // => USBEP_SOFS_LEN
                      LSLS    r5, r5, #3                 //Count of bits to transmit
                      BL      usb_transmit
                      POP     {r5}
                      
                      LDRB    r1, [r5, #USBEP_SOFS_STAT]
                      MOVS    r0, #0xfe
                      ANDS    r1, r1, r0
                      STRB    r1, [r5, #USBEP_SOFS_STAT]  // Flag EP0IN as empty
                      B       jump_exit

                      
pid_out:              MOVS    r0, #1
                      STRB    r0, [r7]                   // Flag, that the next data package is for an endpoint
                      MOV     r0, r8
                      // extract address
                      LSRS    r1, r0, #16
                      MOVS    r2, #0xfe
                      ANDS    r1, r1, r2                 // Mask out address bits a0-a6. Order: a0 a1 a2 a3 a4 a5 a6 0
                      STRB    r1, [r7, #1]               // Store address in protocol state

                      // extract endpoint number
                      LSRS    r1, r0, #9
                      MOVS    r2, #0xf0
                      ANDS    r1, r1, r2
                      STRB    r1, [r7, #2]               // Store endpoint in protocol state => TODO: APPLY MASK! Bitorder e0 e1 e2 e3 0 0 0 0
                      B       jump_exit
                      


                      // Status return functions (function => host)
send_STALL:           MOVS    r0, #USB_TOKEN_STALL
                      MOVS    r5, #8                     //Count of bits to transmit
                      BL      usb_transmit
                      B       jump_exit
                      
send_NAK:             MOVS    r0, #USB_TOKEN_NAK
                      MOVS    r5, #8                     //Count of bits to transmit
                      BL      usb_transmit
                      B       jump_exit
                  
send_ACK:             MOVS    r0, #USB_TOKEN_ACK
                      MOVS    r5, #8                     //Count of bits to transmit
                      BL      usb_transmit
                      B       jump_exit
                  
                      

#if defined ( __IAR_SYSTEMS_ASM__ )

                      DATA
                      ALIGNROM 2
ASM_USB_PROT_STATE:   DC32    USB_PROT_STATE
ASM_USB_EP_BUFFERS:   DC32    usb_ep_buffers
ASM_USB_DEV_ADDRESS:  DC32    usb_dev_address
LBL_PROT_HANDLED:     DC32    protocol_handled

                      END
#else

                      .BALIGN 4
ASM_USB_PROT_STATE:   .long    USB_PROT_STATE
ASM_USB_EP_BUFFERS:   .long    usb_ep_buffers
ASM_USB_DEV_ADDRESS:  .long    usb_dev_address
LBL_PROT_HANDLED:     .long    protocol_handled
                      .END

#endif
                      


