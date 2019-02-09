# revB Giraffe / Can Bus Kit

![img](https://github.com/appleguru/panda/blob/tesla_revB_giraffe/Marked-Up-revB-Giraffe.jpg)

# Table of Contents:

[Useful Part Numbers](#useful-part-numbers)

[J8 RJ45 GPIO Pinout](#j8-rj45-gpio-pinout)

[J7 RJ45 Comma Pinout](#j7-rj45-comma-pinout)

[General Purpose Outputs](#general-purpose-outputs)

[Default Output Mapping with tesla_revB_giraffe firmware](#default-output-mapping-with-tesla_revb_giraffe-firmware)

[Giraffe Can Bus Inputs/Outputs](#giraffe-can-bus-inputsoutputs)

[Firmware](#firmware)

[Warranty, Disclaimer, Assumption of Risk, and Indemnification](#warranty-disclaimer-assumption-of-risk-and-indemnification)

# Useful Part Numbers:

2pin connectors for J2 / J3 / J6 are JST 2pos 2mm PH headers, PN B2B-PH-K-S(LF)(SN)

Fuse F1 is a standard 3A blade type automotive fuse. OE fuse is a Littlefuse 0287003.PXCN

J1 (Optional) is Phoenix contact Part number 1827787 (plug) and 1827949 (soldered header) in case you misplace your plug or would like to add your own header.

J9 is Phoenix Contact plug 1827703 in case you misplace yours.

 

# J8 RJ45 GPIO Pinout:

| RJ45 Pin Number / 568B Color | Function   |
| ---------------------------- | ---------- |
| 1 / Orange/White             | OUT3       |
| 2 / Orange                   | OUT1       |
| 3 / Green / White            | OUT4       |
| 4 / Blue                     | OUT2       |
| 5 / Blue/White               | OUT5       |
| 6 / Green                    | OUT6       |
| 7 / Brown/White              | Fused +12V |
| 8 / Brown (and Shield)       | GND        |

 

# J7 RJ45 Comma Pinout:

| RJ45 Pin Number / 568B Color  | Function      |
| ----------------------------- | ------------- |
| 1 / Orange/White (and Shield) | GND           |
| 2 / Orange                    | Fused +12V    |
| 3 / Green / White             | EPAS CAN -    |
| 4 / Blue                      | No Connection |
| 5 / Blue/White                | GND           |
| 6 / Green                     | EPAS CAN+     |
| 7 / Brown/White               | CAN2+         |
| 8 / Brown                     | CAN2-         |



# General Purpose Outputs:

The general purpose outputs on the GPIO RJ45 and optional 10position terminal block are driven from the panda via one of the otherwise unused LIN bus outputs. This output, through the circuitry on the revB giraffe, drives an automotive grade 8ch smart FET (Infineon BTS4880R) that can handle loads up to 500mA per channel (2A max across all outputs).

The initial application for the GPIO is to support the "Tesla Camera switcher" project, which allows intelligent switching between the car's backup camera and a new camera added to the front bumper to help with forward parking on a Model S or Model X. The current firmware automatically switches to the backup camera feed when in reverse, and the front camera feed otherwise, allowing for the use of pressing and holding on the menu button on the steering wheel to manually select.

 Other projects that will potentially use this GPIO are the "Tesla 360 camera install", the "Tesla lighted appliqué", and a few owners that have installed light bars/want to drive relays from CAN so they come on with their high beams. The firmware is open-source, so you can configure this interface to suit your project’s needs and get a very capable GPIO from CAN! Please see the table below for the current CAN signals mapped: 

# Default Output Mapping with tesla_revB_giraffe firmware:

| Output PIN Number | Function                      |
| ----------------- | ----------------------------- |
| OUT1              | SW1 (Front, Rear cams)        |
| OUT2              | SW2 (Feed from SW1, Baby cam) |
| OUT3              | Brake Lights                  |
| OUT4              | Left Turn Signal              |
| OUT5              | Right Turn Signal             |
| OUT6              | Reverse                       |
| OUT7              | High Beams On                 |
| OUT8              | Drive                         |

 

# Giraffe Can Bus Inputs/Outputs:

Panda CAN2 is spare/unused in most standard configurations. R11 on the bottom of the board is unpopulated and available for an optional 120 Ohm 1206 termination resistor to be installed if needed. CAN2 is broken out on both the Comma RJ45 output and the J3 2pin output (see pinout).

Panda CAN1 is connected to the Model S and Model X’s Chassis CAN via the giraffe’s male OBD’s Pins 1 (CH+) and 9 (CH-). The J6 EPAS output switches between CH CAN from the Car and CAN3 from the Panda, depending on the state of CAN Select (Which is driven by the panda’s GMLAN output). When it is high (D1 Yellow), J6 is sending Panda CAN3 out. When it is low (D1 Green), the Car’s Chassis CAN is going out J6.

# Firmware:

Standard Panda firmware (For the camera switcher, etc) can be found here: <https://github.com/appleguru/panda/tree/tesla_revB_giraffe>



Openpilot-compatible firmware can be found here:

https://github.com/jeankalud/openpilot/tree/revB_giraffe

To flash new firmware, run *make recover* in the board directory with your panda connected to your PC via USB (please disconnect from the car to ensure a clean flash)

 

# Warranty, Disclaimer, Assumption of Risk, and Indemnification:

NO WARRANTIES: TO THE EXTENT PERMITTED BY APPLICABLE LAW, NO ENTITY, EITHER EXPRESSLY OR IMPLICITLY, WARRANTS ANY ASPECT OF THIS HARDWARE, SOFTWARE OR FIRMWARE, INCLUDING ANY OUTPUT OR RESULTS OF THIS HARDWARE, SOFTWARE OR FIRMWARE. THIS HARDWARE, SOFTWARE OR FIRMWARE IS BEING PROVIDED "AS IS", WITHOUT ANY WARRANTY OF ANY TYPE OR NATURE, EITHER EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.

ASSUMPTION OF RISK: THE RISK OF ANY AND ALL LOSS, DAMAGE, OR UNSATISFACTORY PERFORMANCE OF THIS HARDWARE, SOFTWARE OR FIRMWARE RESTS WITH YOU AS THE USER. TO THE EXTENT PERMITTED BY LAW, NO ENTITY EITHER EXPRESSLY OR IMPLICITLY, MAKES ANY REPRESENTATION OR WARRANTY REGARDING THE APPROPRIATENESS OF THE USE, OUTPUT, OR RESULTS OF THE USE OF THIS HARDWARE, SOFTWARE OR FIRMWARE IN TERMS OF ITS CORRECTNESS, ACCURACY, RELIABILITY, BEING CURRENT OR OTHERWISE. NOR DO THEY HAVE ANY OBLIGATION TO CORRECT ERRORS, MAKE CHANGES, SUPPORT THIS HARDWARE, SOFTWARE OR FIRMWARE, DISTRIBUTE UPDATES, OR PROVIDE NOTIFICATION OF ANY ERROR OR DEFECT, KNOWN OR UNKNOWN. IF YOU RELY UPON THIS HARDWARE, SOFTWARE OR FIRMWARE, YOU DO SO AT YOUR OWN RISK, AND YOU ASSUME THE RESPONSIBILITY FOR THE RESULTS. SHOULD THIS HARDWARE, SOFTWARE OR FIRMWARE PROVE DEFECTIVE, YOU ASSUME THE COST OF ALL LOSSES, INCLUDING, BUT NOT LIMITED TO, ANY NECESSARY SERVICING, REPAIR OR CORRECTION OF ANY PROPERTY INVOLVED.

DISCLAIMER: IN NO EVENT, UNLESS REQUIRED BY APPLICABLE LAW, SHALL ANY ENTITY EXCEPT THE USER BE LIABLE FOR ANY LOSS, EXPENSE OR DAMAGE, OF ANY TYPE OR NATURE ARISING OUT OF THE USE OF, OR INABILITY TO USE THIS HARDWARE, SOFTWARE OR FIRMWARE, INCLUDING, BUT NOT LIMITED TO, CLAIMS, SUITS OR CAUSES OF ACTION INVOLVING ALLEGED INFRINGEMENT OF COPYRIGHTS, PATENTS, TRADEMARKS, TRADE SECRETS, OR UNFAIR COMPETITION.

INDEMNIFICATION: TO THE EXTENT PERMITTED BY LAW THROUGH THIS LICENSE, YOU, THE LICENSEE, AGREE TO INDEMNIFY AND HOLD HARMLESS ANY PERSON OR ENTITY FROM AND AGAINST ALL CLAIMS, LIABILITIES, LOSSES, CAUSES OF ACTION, DAMAGES, JUDGMENTS, AND EXPENSES, INCLUDING THE REASONABLE COST OF ATTORNEYS’ FEES AND COURT COSTS, FOR INJURIES OR DAMAGES TO THE PERSON OR PROPERTY OF THIRD PARTIES, INCLUDING, WITHOUT LIMITATIONS, CONSEQUENTIAL DAMAGES AND ECONOMIC LOSSES, THAT ARISE OUT OF OR IN CONNECTION WITH YOUR USE, MODIFICATION, OR DISTRIBUTION OF THIS HARDWARE, SOFTWARE OR FIRMWARE, ITS OUTPUT, OR ANY ACCOMPANYING DOCUMENTATION.



Standard panda docs below:

Welcome to panda
======

[panda](http://github.com/commaai/panda) is the nicest universal car interface ever.

<a href="https://panda.comma.ai"><img src="https://github.com/commaai/panda/blob/master/panda.png">

<img src="https://github.com/commaai/panda/blob/master/buy.png"></a>

It supports 3x CAN, 2x LIN, and 1x GMLAN. It also charges a phone. On the computer side, it has both USB and Wi-Fi.

It uses an [STM32F413](http://www.st.com/en/microcontrollers/stm32f413-423.html?querycriteria=productId=LN2004) for low level stuff and an [ESP8266](https://en.wikipedia.org/wiki/ESP8266) for Wi-Fi. They are connected over high speed SPI, so the panda is actually capable of dumping the full contents of the busses over Wi-Fi, unlike every other dongle on amazon. ELM327 is weak, panda is strong.

It is 2nd gen hardware, reusing code and parts from the [NEO](https://github.com/commaai/neo) interface board.

[![CircleCI](https://circleci.com/gh/commaai/panda.svg?style=svg)](https://circleci.com/gh/commaai/panda)

Usage (Python)
------

To install the library:
```
# pip install pandacan
```

See [this class](https://github.com/commaai/panda/blob/master/python/__init__.py#L80) for how to interact with the panda.

For example, to receive CAN messages:
```
>>> from panda import Panda
>>> panda = Panda()
>>> panda.can_recv()
```
And to send one on bus 0:
```
>>> panda.can_send(0x1aa, "message", 0)
```
Find user made scripts on the [wiki](https://community.comma.ai/wiki/index.php/Panda_scripts)

Usage (JavaScript)
-------

See [PandaJS](https://github.com/commaai/pandajs)


Software interface support
------

As a universal car interface, it should support every reasonable software interface.

- User space ([done](https://github.com/commaai/panda/tree/master/python))
- socketcan in kernel ([alpha](https://github.com/commaai/panda/tree/master/drivers/linux))
- ELM327 ([done](https://github.com/commaai/panda/blob/master/boardesp/elm327.c))
- Windows J2534 ([done](https://github.com/commaai/panda/tree/master/drivers/windows))

Directory structure
------

- board      -- Code that runs on the STM32
- boardesp   -- Code that runs on the ESP8266
- drivers    -- Drivers (not needed for use with python)
- python     -- Python userspace library for interfacing with the panda
- tests      -- Tests and helper programs for panda

Programming (over USB)
------

[Programming the Board (STM32)](board/README.md)

[Programming the ESP](boardesp/README.md)


Debugging
------

To print out the serial console from the STM32, run tests/debug_console.py

To print out the serial console from the ESP8266, run PORT=1 tests/debug_console.py

Safety Model
------

When a panda powers up, by default it's in "SAFETY_NOOUTPUT" mode. While in no output mode, the buses are also forced to be silent. In order to send messages, you have to select a safety mode. Currently, setting safety modes is only supported over USB.

Safety modes can also optionally support "controls_allowed", which allows or blocks a subset of messages based on a piece of state in the board.

Hardware
------

Check out the hardware [guide](https://github.com/commaai/panda/blob/master/docs/guide.pdf)

Licensing
------

panda software is released under the MIT license unless otherwise specified.
