
LemcUSB
=======

A purely software based USB peripheral for ARM Cortex M0+ devices.

This software enables Cortex M0+ devices to act as Low speed USB device. 
It includes a lightweight USB & HID Stack and easily be optimized to 
consume only 4KB of flash.

The development too a lot of time, especially testing. Alltough I created
a lot of USB devices and stacks before, this was very challeging, because 
each time it looked to work OK, a new issues was found which needed fixing. 
E.g. special cases in bitdestuffig, timing related issues, and so on.

A Cortex M0+ at 24 MHz can do the whole USB decoding within a single loop.
With loop unrolling and some other techniques the clock frequency can be 
decreased more. To keep the memory footprint low, we decided to start with
24 MHz only for the first version.
A synchronization method using an internal RC oscillator is in evaluation 
phase. perspectively it can be possible to use e.g. the 21 MHz HFRCO of a
EFM32ZG as clock source. Compared to other solutions, the synchronization 
will most likely not retune the HFRCO, but act like a DPLL.

Timing critical code executes from RAM. This is to avoid waitstates, 
which can be undeterministic. From RAM, it will execute always with 
0 waitstates.

Hardware requirements
=====================

This software was up to now only tested on EFM32ZG110F32. It should work 
on any other EFM32ZG, too.

Currently PA0 needs to connect to PC0 via a 1K5 resistor
PC0 to D-
PC1 to D+ of the USB port.

Apart from an external 24 MHz crystal nothing more is needed.

License:
========

This library is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License version 3.0 as
published by the Free Software Foundation and appearing in the file
LICENSE.txt included in the packaging of this file.

In addition, as a special exception, http://lemcu.org gives you certain
additional rights. These rights are described in the lemcu.org GPL
Exception version 1.0, included in the file GPL_EXCEPTION.txt in this
package.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
