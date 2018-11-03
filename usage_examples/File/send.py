import configparser
import serial
from time import sleep

config = configparser.RawConfigParser()
config.read('caster.conf')

file = open('sending.txt', 'r')
buff = file.read()
file.close()

ser = serial.Serial('/dev/cu.SLAB_USBtoUART', 115200, timeout=1)

message = 'p'+buff.encode('utf-8').hex()+'!'
for c in message:
    print(c.encode('utf8'), end='')
    ser.write(c.encode('utf8'))
    sleep(0.001)

message = 'S'
print(message.encode('utf8'))
ser.write(message.encode('utf8'))

while(True):
    print(ser.read().decode('utf-8'), end='')

ser.close()
