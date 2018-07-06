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
  #LIN send
  #st ="test"
  #st = b"\x60"+chr(len(st)+3).encode()+st
  #lin_break = "\x00\x00"
  #lin_sync = "\x55"
  #dat = "\x3c\x60\x06\xB1\x11\x00\x00\x00\x04\xd2"
  
  dat = "\x06\xB1\x11\x00\x00\x00\x04\xd2"
  
  #st = lin_break + lin_sync + dat
  #st =  lin_sync + dat
  st = dat
  #st = "\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0A\x0B\x0C\x0E\x0F"
  #st = "\x55"
  #st = BitArray('0b10000000')
  
  #st = b"\xaa"+chr(len(st)+3).encode()+st
  
  print("ST Data:")
  print(st)
  p.kline_send(st, bus=send_bus, checksum=False)

  #LIN RECV
  ret = p.kline_drain(bus=receive_bus)
  print("RET Data:")
  hexdump(ret)
  
  exit(0)


# this sets bus 2 to actually be GMLAN
# p2.set_gmlan(bus=2)
# 
# send w bitbang then without
# iden = 123
# iden = 18000
# dat = "\x01\x02"
# dat = "\x01\x02\x03\x04\x05\x06\x07\x08"
# while 1:
#   iden += 1
#   p1.set_gmlan(bus=None)
#   p1.can_send(iden, dat, bus=3)
#   p1.set_gmlan(bus=2)
#   p1.can_send(iden, dat, bus=3)
#   time.sleep(0.01)
#   print p2.can_recv()
#   exit(0)

