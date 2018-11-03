import configparser
from bitcoin_rpc_class import RPCHost
import sys
import configparser
import time
import ttn
import base64
import threading

configFile="caster.conf"

config = configparser.RawConfigParser()
config.read(configFile)
app_id = config.get('TTN', 'app_id')
access_key = config.get('TTN', 'access_key')
rpcHost = config.get('LTC', 'host')
rpcPort = config.get('LTC', 'port')
rpcUser = config.get('LTC', 'username')
rpcPassword = config.get('LTC', 'password')
address = config.get('LTC', 'address')
serverURL = 'http://' + rpcUser + ':' + rpcPassword + '@'+rpcHost+':' + str(rpcPort)

host = RPCHost(serverURL)

messages = {}

lock = threading.Lock()

def uplink_callback(msg, client):
  lock.acquire()
  try:
      payload_plain = base64.b64decode(msg.payload_raw)

      dev_id = msg.dev_id
      prefix = payload_plain[0:3]
      message_id = payload_plain[4:6].hex()
      message_tot = payload_plain[7:8].hex()
      message_no = payload_plain[8:9].hex()
      payload = payload_plain[10:].hex()

      if dev_id in messages.keys():
          if message_id in messages[dev_id].keys():
              if message_no in messages[dev_id][message_id].keys():
                  #duplicate
                  print("duplicate")
              else:
                  messages[dev_id][message_id][message_no] = {}
                  messages[dev_id][message_id][message_no]['payload'] = payload
          else:
              messages[dev_id][message_id] = {}
              messages[dev_id][message_id]['message_tot'] = message_tot
              messages[dev_id][message_id][message_no] = {}
              messages[dev_id][message_id][message_no]['payload'] = payload
      else:
          messages[dev_id] = {}
          messages[dev_id][message_id] = {}
          messages[dev_id][message_id]['message_tot'] = message_tot
          messages[dev_id][message_id][message_no] = {}
          messages[dev_id][message_id][message_no]['payload'] = payload
  finally:
      lock.release()

handler = ttn.HandlerClient(app_id, access_key)

mqtt_client = handler.data()
mqtt_client.set_uplink_callback(uplink_callback)
mqtt_client.connect()

while(True):
    lock.acquire()
    try:
        for dev_id in list(messages):
            for message_id in list(messages[dev_id]):
                buff=""
                message_complete=True
                for i in range(int(messages[dev_id][message_id]['message_tot'])+1):
                    try:
                        buff += messages[dev_id][message_id][format(i, '02x')]['payload']
                    except:
                        message_complete=False
                if (message_complete):
                    #print(messages[dev_id])
                    messages[dev_id].pop(message_id)
                    #print(messages[dev_id])
                    print(buff)
                    tx = buff.strip()
                    print(tx)
                    res = host.call('decoderawtransaction', tx, True)
                    print(res)
                    res = host.call('sendrawtransaction', tx)
                    print(res)
    finally:
        lock.release()
    time.sleep(10)

mqtt_client.close()
