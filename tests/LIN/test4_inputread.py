#!/usr/bin/env python
import time
from hexdump import hexdump
from panda import Panda
from bitstring import BitArray, BitStream

p = Panda()

# this is a test, no safety
p.set_safety_mode(Panda.SAFETY_ALLOUTPUT)

# get version
print(p.get_version())

# **** test K/L line loopback ****

send_bus = 3
receive_bus = 2

p.set_uart_baud(send_bus, 10400)
p.set_uart_baud(receive_bus, 10400)
p.set_uart_parity(send_bus, 0) # parity, 0=off, 1=even, 2=odd
p.set_uart_parity(receive_bus, 0)


while 1:
  print("Drain anything in the buffer:")
  ret = p.kline_drain(bus=send_bus)
  hexdump(ret)
  ret = p.kline_drain(bus=send_bus)
  hexdump(ret)
  ret = p.kline_drain(bus=send_bus)
  hexdump(ret)
  
  print("IO configuration enabled and read inputs: SB SF 85 00 00 FF:")
  p.kline_send("\x55\x85\x00\x00\xff", bus=send_bus, checksum=False)
  
  print("Sending Empty header for slave to respond...")
  p.kline_send("\x55\x80", bus=send_bus, checksum=False)
  
  time.sleep(0.1)

  #LIN RECV
  ret = p.kline_drain(bus=send_bus)
  print("RET Data response?:")
  hexdump(ret)
  
  print("Sending Empty header for slave to respond...")
  p.kline_send("\x55\x85", bus=send_bus, checksum=False)
  
  time.sleep(0.1)

  #LIN RECV
  ret = p.kline_drain(bus=send_bus)
  print("RET Data response?:")
  hexdump(ret)


  exit(0)

