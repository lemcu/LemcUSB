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

#if defined ( __IAR_SYSTEMS_ASM__ )
                      NAME usb_internal_bitbangusb

                      ;; For a description of the purpose of the following part, look here:
                      ;; http://supp.iar.com/Support/?note=80737&from=search+result
                      ;; (TOPIC: IAR assembler ramfunction)
                      #define SHT_PROGBITS 0x1
                      #define SHF_WRITE 0x1
                      #define SHF_EXECINSTR 0x4
                      RSEG MYCODE:CODE:NOROOT(2)
                      SECTION_TYPE SHT_PROGBITS, SHF_WRITE | SHF_EXECINSTR
                      THUMB
                      EXTERN    usb_protocol_handle

                      PUBLIC    GPIO_ODD_IRQHandler
                      PUBLIC    usb_transmit
                      PUBLIC    protocol_handled
#else
                      .syntax unified
                      .arch armv6-m
                      
                      .SECTION .functioninRAM
                      
                      .thumb
					  .thumb_func

                      .extern    usb_protocol_handle



                      .global    GPIO_ODD_IRQHandler
                      .global    usb_transmit
                      .global    protocol_handled


    				  .type    GPIO_ODD_IRQHandler, %function   // IMPORTANT FOR GCC to get lsb of address set to 1
    				  .type    protocol_handled, %function      // IMPORTANT FOR GCC to get lsb of address set to 1
#endif




                      #define  USBEP_OFS_SETUP             ( 0)
                      #define  USBEP_OFS_EP0OUT            (16)
                      #define  USBEP_OFS_EP0IN             (32)
                      #define  USBEP_OFS_EP1OUT            (48)
                      #define  USBEP_OFS_EP1IN             (64)

                      #define  USBEP_SOFS_STAT             (12)
                      #define  USBEP_SOFS_LEN              (13)


                      
                      
                      
                      
        
                      
                      
        
                      // This is the Interrupt handler, triggered by a rising edge on D+ (PC1)
                      // It is implemented in assembly to minimize latency
                      // (=> avoid branch from C code to ASM and addition of veneer code)
                      
                      // This is called for each received USB data packet. received data is unstuffed.
                      // The data is decoded to R8, R9, R10
                      // Each data byte is in reversed bitorder. For data bytes, the bitorder
                      // needs to be reversed. For PIDs, etc. bitorder reversal is not mandatory
                      // The received bitcount is written to R11
                      // BYTE ORDER:
                      //    R8. 24-31   Byte  0
                      //    R8. 16-23   Byte  1
                      //    R8.  8-15   Byte  2
                      //    R8.  0- 7   Byte  3
                      //    R9. 24-31   Byte  4
                      //    R9. 16-23   Byte  5
                      //    R9.  8-15   Byte  6
                      //    R9.  0- 7   Byte  7
                      //    R10.24-31   Byte  8
                      //    R10.16-23   Byte  9
                      //    R10. 8-15   Byte 10
                      //    R10. 0- 7   Byte 11
                      //
                      // An interupt saves the following register on stack:
                      // - R0-R3, R12
                      // - LR, PC & xPSR
GPIO_ODD_IRQHandler:

                      NOP
                      NOP

					  MOV     r0, r8
                      MOV     r1, r9
                      MOV     r2, r10
                      MOV     r3, r11
                      PUSH    {r0-r7}
                      MOV     r0, LR
                      PUSH    {r0}
                  
                      
                      MOVS    r3, #3
                      MOVS    r4, #0xff                                         // preload with 0xff to avoid bitunstuffing after start
                      MOVS    r5, #0x80
                      LSLS    r5, r5, #24
                      MOVS    r0, #0                      
                      MOV     r11, r0
                      MOVS    r1, #1
                      MOV     r12, r1

                      // r1 = 1
                      // r3 = 3
                      // r5 = (1<<13)
                      // r7 = &GPIO_PE_DOUTSET
                      
                      LDR     r7, GPIO_PC_DIN                                   // r7 = address of register GPIO_PC_DIN
                      
                      // wait until a 1 is detected. Unfolded loop to be able to detect a timeout condition
usb_sync_wait_while_0:
                      LDR     r0, [r7]                                          // read port state
                      TST     r0, r1
                      BEQ     usb_sync_found
                      LDR     r0, [r7]                                          // read port state
                      TST     r0, r1
                      BEQ     usb_sync_found
                      LDR     r0, [r7]                                          // read port state
                      TST     r0, r1
                      BEQ     usb_sync_found
                      LDR     r0, [r7]                                          // read port state
                      TST     r0, r1
                      BEQ     usb_sync_found
                      LDR     r0, [r7]                                          // read port state
                      TST     r0, r1
                      BEQ     usb_sync_found
                      LDR     r0, [r7]                                          // read port state
                      TST     r0, r1
                      BEQ     usb_sync_found
                      LDR     r0, [r7]                                          // read port state
                      TST     r0, r1
                      BEQ     usb_sync_found
                      
                      B       protocol_handled                                  // State was zero for more than 1 bittime => exit (timeout condition)
                      
                      
usb_sync_found:
                      // delay 1 bittime
                      LDR     r0, [r7]                                          // read port state
                      TST     r0, r1
                      BNE     usb_sync_wait_while_0
                      LDR     r0, [r7]                                          // read port state
                      TST     r0, r1
                      BNE     usb_sync_wait_while_0
                      LDR     r0, [r7]                                          // read port state
                      TST     r0, r1
                      BNE     usb_sync_wait_while_0
                      LDR     r0, [r7]                                          // read port state
                      TST     r0, r1
                      BNE     usb_sync_wait_while_0
                      LDR     r0, [r7]                                          // read port state
                      TST     r0, r1
                      BNE     usb_sync_wait_while_0
                        NOP
                      
                      // check, if bitstate is still 1 (=> if 2 consecutive 1 bits received)
                      LDR     r2, [r7]                                          // read port state
                      TST     r2, r1
                      BNE     usb_sync_wait_while_0

                      BL    _delay_13

                      // here: We are in the middle of the very first bit of this packet
                      // r0 = port state current
                      // r1 = 1
                      // r2 = port state old
                      // r3 = 3
                      // r4/5/6 = received data (parallel)
                      // r7 = address of GPIO_PC_DIN
                      // r11 = bitcounter
                      MOVS r1, #0
                      
rcv_bits:             LDR     r0, [r7]                    // read port state
                      ANDS    r0, r3                      // check for SE0 and mask out unneeded bits
                      BEQ     se0_detected                // end of receive loop!
					  EORS    r2, r0                      // NRZ decoding (XOR bitstate with old bitstate) => inverts data, USB uses NRZI!
                      LSRS    r2, r2, #1
                      ADCS    r4, r4, r4                  // Shift carry into receive chain
                      ADCS    r5, r5, r5
                      ADCS    r6, r6, r6
                      ADD     r11, r11, r12               // increment bitcounter
                      MOV     r2, r0                      // Store old bitstate
                        NOP
                      LSLS    r1, r1, #1                  // Shift bitunstuffing EOR mask
                      LSLS    r0, r4, #26                 // Mask out last 6 received bits
                      EORS    r0, r0, r1                  // check if 6 consecutive 0s have been received
                      BNE     rcv_bits
                      
                      // here we get when bitunstuffing needs to be done
                      // 15/16 cycles over
                      NOP                 //16
                      LDR     r2, [r7]    // 1             // read port state to save it as "old" bitstate
                            BL    _delay_11   //12
                      MOVS    r1, #1      //13             // save bitunstuffing EOR mask
                      LSLS    r1, r1, #26 //14
                      B       rcv_bits    //16
                      
                      


se0_detected:         
                      // left align data (rough: 32 Bit steps)
                      MOV     r0, r11
                      CMP     r0, #64
                      BHI     se0_detected_align_0
                      
                      MOVS    r6, r5
                      MOVS    r5, r4
                      SUBS    r4, r4, r4 // not mandatory
                      
                      CMP     r0, #32
                      BHI     se0_detected_align_0
                      
                      MOVS    r6, r5
                      MOVS    r5, r4                      
                      SUBS    r4, r4, r4 // not mandatory
                      
                      // left align data (fine alignment: 0..31 bits)
se0_detected_align_0: MOV     r0, r11
                      MOVS    r1, #31
                      ANDS    r1, r1, r0  // r1 = bitcount & 31
                      
                    CMP r1, #0
                    BEQ se0_alginment_done

                      MOVS    r0, #0
                      MVNS    r0, r0      // r0 = 0xffffffff
                      LSRS    r0, r0, r1  // 8 => 0x00ffffff, 24 => 0x000000ff r0 = Mask of bytes to copy from lower word to upper one

                      RORS	r6, r6, r1  //
                      BICS	r6, r6, r0  // r6 = r6 and not r0 (reset lower bytes to 0)

                      RORS	r5, r5, r1  //
                      MOVS	r2, r5
                      ANDS    r2, r2, r0
                      ORRS    r6, r6, r2  //
                      BICS	r5, r5, r0  // r6 = r6 and not r0 (reset lower bytes to 0)

                      RORS	r4, r4, r1  //
                      MOVS    r2, r4
                      ANDS    r2, r2, r0
                      ORRS    r5, r5, r2
                      BICS	r4, r4, r0  // not mandatory
                      


                      // Invert data (NRZ => NRZI)
se0_alginment_done:   MVNS    r0, r4
                      MOV     r10, r0
                      MVNS    r0, r5
                      MOV     r9, r0
                      MVNS    r0, r6
                      MOV     r8, r0
                      
                      LDR     r0, LABEL_USB_PROT_HDL
                      BX      r0                           // Jump to protocol state machine code located in Flash

                      
protocol_handled:
// This was for debugging. enable it, if you want to debug host packets only
//                      MOVS    r5, #255
//delayloop:
//                      SUBS    r5, r5, #1
//                      BNE     delayloop
                      
                      // acknowledge GPIO interrupt
                      LDR     r6, GPIO_IFC
                      MOVS    r5, #2
                      STR     r5, [r6]

                      POP     {r0}
                      MOV     LR, r0

                      POP     {r0-r7}
                      MOV     r8, r0
                      MOV     r9, r1
                      MOV     r10, r2
                      MOV     r11, r3          // r0-r3 are restored by ISR return
                      BX      LR               // return
                      

////////////////////////////////////////


#if defined ( __IAR_SYSTEMS_ASM__ )
                      DATA
                      ALIGNROM 2
GPIO_PC_DIN:          DC32    0x40006064   ; GPIO_PC_DIN
GPIO_PE_DOUTSET       DC32    0x400060a0   ; GPIO_PE_DOUTSET.DOUTSET
GPIO_IFC:             DC32    0x4000611c   ; GPIO.IFC
LABEL_USB_PROT_HDL:   DC32 usb_protocol_handle
//GPIO_PB_TOGGLE:       DC32    0x4000603C   ; REMOVEME

#else
                      .BALIGN 4
GPIO_PC_DIN:          .INT    0x40006064   // GPIO_PC_DIN
GPIO_PE_DOUTSET:      .INT    0x400060a0   // GPIO_PE_DOUTSET.DOUTSET
GPIO_IFC:             .INT    0x4000611c   // GPIO.IFC
LABEL_USB_PROT_HDL:   .INT usb_protocol_handle


#endif






#if defined ( __IAR_SYSTEMS_ASM__ )
                      CODE
                      THUMB
#else
                      .SECTION .functioninRAM
                      .THUMB
#endif
                      // This function transmits a USB packet as response to the host
                      // Used for : ACK, NAK, STALL, DATA0 & DATA1 PIDs
                      // Parameters:
                      // r0/r1/r2 = Data to transmit
                      // r5 = Bitcount
                      // The Bitorder of the transmission is the right one (no bitreversal as in receive function)
                      // transmit order:
                      // r0.0-7 => r0.8-15 => r0.16-23 => r0.24-31 =>
                      // r1.0-7 => r1.8-15 => r1.16-23 => r1.24-31 =>
                      // r2.0-7 => r2.8-15 => r2.16-23 => r2.24-31
usb_transmit:         
                      MOV     r7, LR
                      PUSH    {r7}
                      // Switch PC0 & PC1 to output
                      LDR     r7, GPIO_PC_BASE
                      LDR     r3, [r7, #4]  // read pin mode low register
                      ADDS    r3, r3, #0x33  // mode Input = 0x11, Output = 0x44
                      STR     r3, [r7, #4]  // writepin mode low register
                      // IO state = push pull & IDLE (D-=1 D+=0)
                      
                      // 1.) Send SYNC Byte
                      MOVS    r3, #0x03       // bitmask for toggling IO pin
                      MOVS    r4, #7
usb_tx_syncloop:      STR     r3, [r7, #24]   // Toggle IOs to transmit a 0
                        BL    _delay_12
                      SUBS    r4, r4, #1
                      BNE     usb_tx_syncloop
//15
                      // transmit a "1" - last bit of SYNC sequence
                        BL    _delay_12

                      // 2.) Send data given in registers r0/r1/r2 and bitcount in r5
                      // Here all bits get transmitted. Bitstuffing is also applied
                      // The minimal version of this would only need 6 cycles
                      MOVS    r4, #5          // consecutive 1-Bit counter (used for bitstuffing). Last transmitted Bit was 1, so initialize it with 5 rather than 6
                      MOVS    r6, #32         // Needed to check if a new 32 Bit word should be transmitted
usb_tx_bitloop:       LSRS    r0, r0, #1      // shift out bit to carry flag for transmitting
                      BCS     usb_txbitloop_send1 //do nothing if bit is 1 (only a zero causes toggling
usb_txbitloop_send0:  STR     r3, [r7, #24]   // Toggle IOs to transmit a 0
                      MOVS    r4, #6          // Reset 1-Bit counter
                      B       usb_txbitloop_sent1
usb_txbitloop_send1:  SUBS    r4, #1
                      BEQ     usb_tx_sendstuffbit
usb_tx_bitstutt_ret:    NOP                   // equalize execution time for 2 branch paths
usb_txbitloop_sent1:  SUBS    r5, r5, #1
                      BEQ     usb_tx_send_eop // final word transmitted
                      SUBS    r6, r6, #1
                      BEQ     usb_txbit_nextword
                        NOP
                        NOP
                        NOP  
                        NOP
                      B       usb_tx_bitloop
                      
                      // select next word for transmission
usb_txbit_nextword:   MOVS    r0, r1
                      MOVS    r1, r2
                      MOVS    r6, #32
                      B       usb_tx_bitloop                       
                      
                      // Send stuffbit
                      // 4 cycles over since last bit
usb_tx_sendstuffbit:    BL    _delay_11
                      MOVS    r4, #6          // Reset 1-Bit counter
                      STR     r3, [r7, #24]   // Toggle IOs to transmit a 0
                      B       usb_tx_bitstutt_ret
                      
                      // send EOP (2*SE0)
                      // 7 cycles over since last bit
usb_tx_send_eop:      
                      BL    _delay_9
                      STR     r3, [r7, #20]   // write to GPIO_PC_DOUTCLR to set D+ and D- to LOW
                        BL    _delay_10
                        BL    _delay_10
                        BL    _delay_10
                         
                      MOVS    r1, #1
                      STR     r1, [r7, #16]   // Set D- back to HIGH
                      
                      // Switch PC0 & PC1 to input
                      LDR     r7, GPIO_PC_BASE
                      LDR     r0, [r7, #4]    // read pin mode low register
                      SUBS    r0, r0, #0x33   // set from 0x44 to 0x11 (push pull to input)
                      STR     r0, [r7, #4]    // writepin mode low register
                      
//                      // delay some time for easier debugging
//                      MOVS    r5, #255
//delayloop_x:          MOVS    r4, #6        // reset 1-bit counter
//                      SUBS    r5, r5, #1
//                      BNE     delayloop_x
                      POP    {r0}
                      MOV    LR, r0
                      BX     LR
         
                      // some delay functions:
_delay_14:            NOP
_delay_13:            NOP
_delay_12:            NOP
_delay_11:            NOP
_delay_10:            NOP
_delay_9:             NOP
_delay_8:             NOP
_delay_7:             NOP
_delay_6:             NOP
_delay_5:             BX     LR                      
                      

#if defined ( __IAR_SYSTEMS_ASM__ )

                      DATA
                      ALIGNROM 2
GPIO_PC_BASE:         DC32    0x40006048   ; GPIO_PC_DIN - This is redundant, has already been defined in the RX function
GPIO_PE_DOUTSET_      DC32    0x400060a0   ; GPIO_PE_DOUTSET.DOUTSET

					  END

#else

                      .BALIGN 4
GPIO_PC_BASE:         .INT    0x40006048   // GPIO_PC_DIN - This is redundant, has already been defined in the RX function
GPIO_PE_DOUTSET_:     .INT    0x400060a0   // GPIO_PE_DOUTSET.DOUTSET

                      .END
#endif
                      





/*
TODO:
- Flag endpoint status based on Host reply
- interpret CRC5 ??
- detect RESET condition on bus (very long SE0 phase, check USB spec. for duration). This should reset the usb address
- RESET = SE0 for >= 10ms

- codesize optimization: 
  * use branches to identical code secitions
*/

