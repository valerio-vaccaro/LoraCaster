import configparser
from bitcoin_rpc_class import RPCHost
import serial
from time import sleep

config = configparser.RawConfigParser()
config.read('caster.conf')
rpcHost = config.get('BTC', 'host')
rpcPort = config.get('BTC', 'port')
rpcUser = config.get('BTC', 'username')
rpcPassword = config.get('BTC', 'password')
address = config.get('BTC', 'address')
rpcWallet = config.get('BTC', 'wallet')
serverURL = 'http://' + rpcUser + ':' + rpcPassword + '@'+rpcHost+':' + str(rpcPort)+'/wallet/' + rpcWallet
#serverURL = 'http://' + rpcUser + ':' + rpcPassword + '@'+rpcHost+':' + str(rpcPort)

host = RPCHost(serverURL)

res = host.call('createrawtransaction', [], {address:0.001})
res = host.call('fundrawtransaction', res)
res = host.call('signrawtransaction', res['hex'])

ser = serial.Serial('/dev/cu.SLAB_USBtoUART', 115200, timeout=1)

message = 'p'+res['hex']+'!'
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
