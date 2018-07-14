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
  
  #dat = "\x60\x06\xB1\x11\x00\x00\x00\x04"

  #dat = "\x55\x3c\x60\x06\xB1\x11\x00\x00\x00\x04\xD2"
  #dat2 = "\x55\x7D"
  
  
  #st = lin_break + lin_sync + dat
  #st =  lin_sync + dat
  #st = dat
  #st2 = dat2
  #st = "\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0A\x0B\x0C\x0E\x0F"
  #st = "\x55"
  #st = BitArray('0b10000000')
  
  #st = b"\xaa"+chr(len(st)+3).encode()+st
  
  print("Drain anything in the buffer:")
  ret = p.kline_drain(bus=send_bus)
  hexdump(ret)
  ret = p.kline_drain(bus=send_bus)
  hexdump(ret)
  ret = p.kline_drain(bus=send_bus)
  hexdump(ret)
  
  print("Sending Assign frameID, default NAD used and ID(PxReq) = 04,ID(PxResp) = 05: SB SF 3C 60 06 B1 11 00 00 00 04 D2")
  p.kline_send("\x55\x3c\x60\x06\xB1\x11\x00\x00\x00\x04\xD2", bus=send_bus, checksum=False)
  
  print("Sending Empty header for slave to respond with positive response: SB SF 7D")
  p.kline_send("\x55\x7D", bus=send_bus, checksum=False)
  
  time.sleep(0.2)

  #LIN RECV
  ret = p.kline_drain(bus=send_bus)
  print("RET Data positive response:")
  hexdump(ret)



  print("Sending Datadump1, 8 x LSE: SB SF 3C 60 06 B4 00 00 FF 00 00 E4")
  #print("Sending Datadump1, 8 x High Side Emable: SB SF 3C 60 06 B4 00 FF 00 00 00 E4")
  p.kline_send("\x55\x3c\x60\x06\xb4\x00\x00\xff\x00\x00\xe4", bus=send_bus, checksum=False)
  #p.kline_send("\x55\x3c\x60\x06\xb4\x00\xff\x00\x00\x00\xe4", bus=send_bus, checksum=False)
  
  print("Sending Empty header for slave to respond with config: SB SF 7D")
  p.kline_send("\x55\x7D", bus=send_bus, checksum=False)
  
  time.sleep(0.2)

  #LIN RECV
  ret = p.kline_drain(bus=send_bus)
  print("RET Data config:")
  hexdump(ret)
  
  
  
  print("Sending Datadump2, no capture and threshold select (optional) SB SF 3C 60 06 B4 40 00 00 00 00 A4: ")
  p.kline_send("\x55\x3c\x60\x06\xb4\x40\x00\x00\x00\x00\xa4", bus=send_bus, checksum=False)
  
  print("Sending Empty header for slave to respond with config 2: SB SF 7D")
  p.kline_send("\x55\x7D", bus=send_bus, checksum=False)
  
  time.sleep(0.2)

  #LIN RECV
  ret = p.kline_drain(bus=send_bus)
  print("RET Data config 2:")
  hexdump(ret)



  print("Sending Datadump3, LHvalue=0x55, default PWM = 0x10 (optional) SB SF 3C 60 04 B4 80 55 10 FF FF 01:")
  p.kline_send("\x55\x3c\x60\x04\xb4\x80\x55\x10\xff\xff\x01", bus=send_bus, checksum=False)
  
  print("Sending Empty header for slave to respond with config 3: SB SF 7D")
  p.kline_send("\x55\x7D", bus=send_bus, checksum=False)
  
  time.sleep(0.2)

  #LIN RECV
  ret = p.kline_drain(bus=send_bus)
  print("RET Data config 3:")
  hexdump(ret)


#   print("Sending Diagnostic Data Block 4: SB SF 3C 60 02 B4 C0 FF FF FF FF 28:")
#   p.kline_send("\x55\x3c\x60\x02\xb4\xc0\xff\xff\xff\xff\x28", bus=send_bus, checksum=False)
#   
#   print("Sending Empty header for slave to respond with diag 4: SB SF 7D")
#   p.kline_send("\x55\x7D", bus=send_bus, checksum=False)
#   
#   time.sleep(0.2)
# 
#   #LIN RECV
#   ret = p.kline_drain(bus=send_bus)
#   print("RET Data diag 4:")
#   hexdump(ret)



  print("Sending Read by identifier request (optional) SB SF 3C 60 06 B2 00 11 00 00 00 D5:")
  p.kline_send("\x55\x3c\x60\x06\xb2\x00\x11\x00\x00\x00\xd5", bus=send_bus, checksum=False)
  
  print("Sending Empty header for slave to respond with id request postive resp: SB SF 7D")
  p.kline_send("\x55\x7D", bus=send_bus, checksum=False)
  
  time.sleep(0.2)

  #LIN RECV
  ret = p.kline_drain(bus=send_bus)
  print("RET Data id request positive response:")
  hexdump(ret)
  
  print("P0 on: SB SF C4 01 80 7E")
  p.kline_send("\x55\xc4\x01\x80\x7e", bus=send_bus, checksum=False)
  time.sleep(0.2)
  print("Any return?:")
  hexdump(ret)
  print("P1 on: SB SF C4 02 80 7D")
  p.kline_send("\x55\xc4\x02\x80\x7d", bus=send_bus, checksum=False)
  time.sleep(0.2)
  print("Any return?:")
  hexdump(ret)
  print("P2 on: SB SF C4 04 80 7B")
  p.kline_send("\x55\xc4\x04\x80\x7b", bus=send_bus, checksum=False)
  time.sleep(0.2)
  print("Any return?:")
  hexdump(ret)
  print("P3 on: SB SF C4 08 80 77")
  p.kline_send("\x55\xc4\x08\x80\x77", bus=send_bus, checksum=False)
  time.sleep(0.2)
  print("Any return?:")
  hexdump(ret)
  print("P4 on: SB SF C4 10 80 6F")
  p.kline_send("\x55\xc4\x10\x80\x6f", bus=send_bus, checksum=False)
  time.sleep(0.2)
  print("Any return?:")
  hexdump(ret)
  print("P5 on: SB SF C4 20 80 5F")
  p.kline_send("\x55\xc4\x20\x80\x5f", bus=send_bus, checksum=False)
  time.sleep(0.2)
  print("Any return?:")
  hexdump(ret)
  print("P6 on: SB SF C4 40 80 3F")
  p.kline_send("\x55\xc4\x40\x80\x3f", bus=send_bus, checksum=False)
  time.sleep(0.2)
  print("Any return?:")
  hexdump(ret)
  print("P7 on: SB SF C4 80 80 FE")
  p.kline_send("\x55\xc4\x80\x80\xfe", bus=send_bus, checksum=False)
  time.sleep(0.2)
  print("Any return?:")
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

