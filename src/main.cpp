/*===================================================================================*
  :::         ...    :::::::..    :::.       .,-:::::   :::.     .::::::.::::::::::::
  ;;;      .;;;;;;;. ;;;;``;;;;   ;;`;;    ,;;;'````'   ;;`;;   ;;;`    `;;;;;;;;''''
  [[[     ,[[     [[,[[[,/[[['  ,[[ '[[,  [[[         ,[[ '[[, '[==/[[[[,    [[
  $$'     $$$,     $$$$$$$$$c   c$$$cc$$$c $$$        c$$$cc$$$c  '''    $    $$
 o88oo,.__"888,_ _,88P888b "88bo,888   888,`88bo,__,o, 888   888,88b    dP    88,
 """"YUMMM  "YMMMMMP" MMMM   "W" YMM   ""`   "YUMMMMMP"YMM   ""`  "YMmMY"     MMM
 *===================================================================================*
  LoraCast - broadcast messages on TTN LoraWan network
   MIT License - Copyright (c) 2018 Valerio Vaccaro
   Based on https://github.com/gonzalocasas/arduino-uno-dragino-lorawan/blob/master/LICENSE
   Based on examples from https://github.com/matthijskooijman/arduino-lmic
 *===================================================================================*/
const char source_filename[] = __FILE__;
const char compile_date[] = __DATE__ " " __TIME__;
const char version[] = "LoraCast v.0.0.1";

#include <Arduino.h>
#include "lmic.h"
#include <hal/hal.h>
#include <SPI.h>
#include <SSD1306.h>
#include "soc/efuse_reg.h"
#include "ttn_config.h"

// --- Hardware configuration ---
// Led
#define LEDPIN 2
// Display
#define OLED_I2C_ADDR 0x3C
#define OLED_RESET 16
#define OLED_SDA 4
#define OLED_SCL 15
SSD1306 display (OLED_I2C_ADDR, OLED_SDA, OLED_SCL);
// Lora radio pin mapping
const lmic_pinmap lmic_pins = {
    .nss = 18,
    .rxtx = LMIC_UNUSED_PIN,
    .rst = 14,
    .dio = {26, 33, 32}  // Pins for the Heltec ESP32 Lora board/ TTGO Lora32 with 3D metal antenna
};

// --- Software config
// Internal buffer
#define MAX_BUFF_SIZE 1000
int BUFF_SIZE = 1000;
static uint8_t buff[MAX_BUFF_SIZE] = {
  0x02,0x00,0x00,0x00,0x00,0x01,0x01,0xc7,0x08,0xc8,0x6c,0xc2,0x48,0x81,0x1a,0x6b,0x4e,0xa3,0x7c,0x2c,0x31,0x20,0x34,0xd0,0x0a,
  0xc7,0x9e,0x59,0xf3,0x27,0x1a,0x57,0x7b,0x03,0xea,0xcd,0xb1,0x5c,0xde,0x01,0x00,0x00,0x00,0x17,0x16,0x00,0x14,0x65,0x05,0x70,
  0x25,0x96,0x8a,0x3f,0x2d,0x9d,0x90,0xd5,0xb3,0xdc,0x5f,0x1b,0x03,0xee,0x18,0x94,0x48,0xff,0xff,0xff,0xff,0x01,0x70,0x22,0x13,
  0x00,0x00,0x00,0x00,0x00,0x17,0xa9,0x14,0xb9,0x3b,0xa4,0xbe,0x0a,0x5e,0x96,0x20,0x50,0x8a,0xe7,0x9a,0x33,0xbb,0x7a,0x4e,0x75,
  0x19,0x00,0xb9,0x87,0x02,0x47,0x30,0x44,0x02,0x20,0x0f,0xd7,0xd1,0x80,0xef,0x41,0x16,0x72,0x03,0xe9,0x40,0x37,0xc3,0xac,0xc2,
  0x09,0xf8,0xc7,0x89,0xa6,0xe5,0x26,0x1f,0x36,0xdb,0x01,0xbe,0x3b,0xff,0x48,0x22,0xbd,0x02,0x20,0x6f,0xa7,0xcb,0x5a,0xc2,0xd2,
  0x1e,0xf8,0x20,0x8d,0x6c,0x07,0x95,0x16,0xe0,0x45,0xc3,0x8a,0xc8,0x7c,0x4d,0x88,0xc3,0x62,0x52,0x63,0x2e,0x6a,0xb0,0xff,0x0c,
  0xaf,0x01,0x21,0x03,0x88,0x24,0x97,0x95,0x3a,0x1c,0x18,0x34,0xe1,0xef,0x29,0x71,0x06,0x9e,0xe3,0xbb,0xf6,0x2f,0xe5,0xb6,0x7b,
  0xa0,0x94,0x0d,0x46,0x12,0x3b,0x56,0x22,0x0d,0x25,0x6d,0x00,0x00,0x00,0x00};
int buff_size = 215;

uint8_t counter = 0;
static osjob_t sendjob;
uint8_t TX_INTERVAL = 0;
bool TX_RETRASMISSION = false;
bool VERBOSE = false;
bool HALT = false;
char TTN_response[30];
uint8_t PAYLOAD_SIZE = 100;
bool LAST_TX=false;
uint8_t SF=7;

uint8_t r1 = esp_random();
uint8_t r2 = esp_random();

void mydisplay(char* function, char* message, char* error, int counter){
  char tmp[32];
  display.clear();
  display.drawString (0, 0, version);
  display.drawString (0, 10, function);
  display.drawString (0, 20, message);
  display.drawString (0, 30, error);
  sprintf(tmp, "ID 0x%.2X 0x%.2X - SF %d",r1,r2,SF);
  display.drawString (0, 40, tmp);
  sprintf(tmp, "Message counter: %d/%d",counter);
  display.drawString (0, 50, tmp);
  display.display ();
}

void help(){
  static char * banner =
  "*===================================================================================*\n"
  " :::         ...    :::::::..    :::.       .,-:::::   :::.     .::::::.::::::::::::\n"
  " ;;;      .;;;;;;;. ;;;;``;;;;   ;;`;;    ,;;;'````'   ;;`;;   ;;;`    `;;;;;;;;''''\n"
  " [[[     ,[[     \[[,[[[,/[[['  ,[[ '[[,  [[[         ,[[ '[[, '[==/[[[[,    [[     \n"
  " $$'     $$$,     $$$$$$$$$c   c$$$cc$$$c $$$        c$$$cc$$$c  '''    $    $$     \n"
  "o88oo,.__\"888,_ _,88P888b \"88bo,888   888,`88bo,__,o, 888   888,88b    dP    88,    \n"
  "\"\"\"\"YUMMM  \"YMMMMMP\" MMMM   \"W\" YMM   \"\"`   \"YUMMMMMP\"YMM   \"\"`  \"YMmMY\"     MMM    \n"
  "                   LoraCast, the Swiss Army Knife of LoraWan\n"
  "*===================================================================================*\n";
  Serial.print(banner);
  Serial.print(version);Serial.print(F(" build on "));Serial.println(compile_date);
  Serial.println(F("*===================================================================================*"));
  Serial.println(F("General commands:"));
  Serial.println(F("   h - shows this help"));
  Serial.println(F("   v - toggle verbosity"));
  Serial.println(F("   l - toggle led"));
  Serial.println(F("Communication commands:"));
  Serial.println(F("   p[hex payload]! - charge payload in the memory max 1k byte"));
  Serial.println(F("   d - dump actual payload present in the memory"));
  Serial.println(F("   S - start transmission"));
  Serial.println(F("   H - halt transmission"));
  Serial.println(F("Advanced commands:"));
  Serial.println(F("   t - sent a Test message with content 0123456789 (ASCII)"));
  Serial.println(F("   c - shows LoraWan/TTN configuration"));
  Serial.print(  F("   b[]! - set message dimension in byte 0-"));Serial.print(MAX_BUFF_SIZE);Serial.print(F(" (actual "));Serial.print(BUFF_SIZE);Serial.println(F(") "));
  Serial.print(  F("   r - toggle retransmission (actual "));Serial.print(TX_RETRASMISSION);Serial.println(F(")"));
  Serial.print(  F("   w[]! - set delay betweet packets in second 0-255 (actual "));Serial.print(TX_INTERVAL);Serial.println(F(")"));
  Serial.print(  F("   f[] - set spreading factor (nn) between 7,8,9,10,11,12 (actual "));Serial.print(SF);Serial.println(F(")"));
  char tmp[16];
  sprintf(tmp, "0x%.2X 0x%.2X",r1,r2);
  Serial.print(  F("   R - generate a new random packet ID (actual "));Serial.print(tmp);Serial.println(F(")"));
  Serial.println(F("*===================================================================================*"));
}

void PrintHex8(uint8_t *data, uint8_t length){
  char tmp[16];
  for (int i=0; i<length; i++) {
      sprintf(tmp, "0x%.2X",data[i]);
      Serial.print(tmp); Serial.print(" ");
  }
}

void do_send(osjob_t* j){
    uint8_t message[11+PAYLOAD_SIZE] = "LWC*00*00*0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789";
    const uint8_t max_counter = round(buff_size/PAYLOAD_SIZE);
    if (counter > max_counter){
      counter = 0;
    }
    message[4] = r1;
    message[5] = r2;
    message[7] = max_counter;
    message[8] = counter;
    if(message[7]==message[8]) LAST_TX = true;
    else LAST_TX = false;

    Serial.println(F("-------------"));
    Serial.print(F("Counter ")); Serial.print(counter);
    int remaining = buff_size - (counter*PAYLOAD_SIZE);
    Serial.print(F(" remaining ")); Serial.println(remaining);
    if (remaining > 100) remaining = 100;
    for(uint8_t x=0; x<remaining; x++){
        message[10+x] = buff[(counter*PAYLOAD_SIZE)+x];
    }

    if (VERBOSE) {
      PrintHex8((uint8_t *)message, 10+remaining);
      Serial.println();
    }

    // Check if there is not a current TX/RX job running
    if (LMIC.opmode & OP_TXRXPEND) {
      Serial.println(F("OP_TXRXPEND, not sending"));
    } else {
      // Prepare upstream data transmission at the next possible time.
      LMIC_setTxData2(1, message, 11+remaining-1, 0);
      Serial.println(F("Sending uplink packet..."));
      digitalWrite(LEDPIN, HIGH);
      ++counter;
      mydisplay("Send","Sending uplink packet...","",counter);
    }
    // Next TX is scheduled after TX_COMPLETE event.
}

void onEvent (ev_t ev) {
    Serial.print(os_getTime());
    Serial.print(": ");
    switch(ev) {
        case EV_SCAN_TIMEOUT:
            Serial.println(F("EV_SCAN_TIMEOUT"));
            break;
        case EV_BEACON_FOUND:
            Serial.println(F("EV_BEACON_FOUND"));
            break;
        case EV_BEACON_MISSED:
            Serial.println(F("EV_BEACON_MISSED"));
            break;
        case EV_BEACON_TRACKED:
            Serial.println(F("EV_BEACON_TRACKED"));
            break;
        case EV_JOINING:
            Serial.println(F("EV_JOINING"));
            break;
        case EV_JOINED:
            Serial.println(F("EV_JOINED"));
            break;
        case EV_RFU1:
            Serial.println(F("EV_RFU1"));
            break;
        case EV_JOIN_FAILED:
            Serial.println(F("EV_JOIN_FAILED"));
            break;
        case EV_REJOIN_FAILED:
            Serial.println(F("EV_REJOIN_FAILED"));
            break;
        case EV_TXCOMPLETE:
            Serial.println(F("EV_TXCOMPLETE (includes waiting for RX windows)"));
            digitalWrite(LEDPIN, LOW);
            if (LMIC.txrxFlags & TXRX_ACK)
              Serial.println(F("Received ack"));
            if (LMIC.dataLen) {
              Serial.println(F("Received "));
              Serial.println(LMIC.dataLen);
              Serial.println(F(" bytes of payload"));
            }
            if (LAST_TX){
              if (TX_RETRASMISSION){ // Schedule next transmission
                if (!HALT){
                  os_setTimedCallback(&sendjob, os_getTime()+sec2osticks(TX_INTERVAL), do_send);
                  mydisplay("Send","Sending uplink packet...","",counter);
                } else {
                  Serial.println(F("HALT!"));
                  mydisplay("Send","HALT","",counter);
                  HALT = false;
                }
              }
            } else {
              if (!HALT){
                os_setTimedCallback(&sendjob, os_getTime()+sec2osticks(TX_INTERVAL), do_send);
                mydisplay("Send","Sending uplink packet...","",counter);
              } else {
                Serial.println(F("HALT!"));
                HALT = false;
                mydisplay("Send","HALT","",counter);
              }
            }

            break;
        case EV_LOST_TSYNC:
            Serial.println(F("EV_LOST_TSYNC"));
            break;
        case EV_RESET:
            Serial.println(F("EV_RESET"));
            break;
        case EV_RXCOMPLETE:
            // data received in ping slot
            Serial.println(F("EV_RXCOMPLETE"));
            break;
        case EV_LINK_DEAD:
            Serial.println(F("EV_LINK_DEAD"));
            break;
        case EV_LINK_ALIVE:
            Serial.println(F("EV_LINK_ALIVE"));
            break;
         default:
            Serial.println(F("Unknown event"));
            break;
    }
}

void forceTxSingleChannelDr(_dr_eu868_t sf) {
    for(int i=0; i<9; i++) { // For EU; for US use i<71
        if(i != 0) {
            LMIC_disableChannel(i);
        }
    }
    // Set data rate (SF) and transmit power for uplink
    LMIC_setDrTxpow(sf, 14); //DR_SF7
}

void setup() {
   Serial.begin(115200);
   delay(1500);   // Give time for the seral monitor to start up
   //help();
   Serial.println(F("Starting..."));
   // Use the Blue pin to signal transmission.
   pinMode(LEDPIN,OUTPUT);

   // reset the OLED
   pinMode(OLED_RESET,OUTPUT);
   digitalWrite(OLED_RESET, LOW);
   delay(50);
   digitalWrite(OLED_RESET, HIGH);

   display.init ();
   display.flipScreenVertically ();
   display.setFont (ArialMT_Plain_10);
   display.setTextAlignment (TEXT_ALIGN_LEFT);
   display.display ();

    // LMIC init
    os_init();

    // Reset the MAC state. Session and pending data transfers will be discarded.
    LMIC_reset();

    // Set up the channels used by the Things Network, which corresponds
    // to the defaults of most gateways. Without this, only three base
    // channels from the LoRaWAN specification are used, which certainly
    // works, so it is good for debugging, but can overload those
    // frequencies, so be sure to configure the full frequency range of
    // your network here (unless your network autoconfigures them).
    // Setting up channels should happen after LMIC_setSession, as that
    // configures the minimal channel set.

    LMIC_setupChannel(0, 868100000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
    LMIC_setupChannel(1, 868300000, DR_RANGE_MAP(DR_SF12, DR_SF7B), BAND_CENTI);      // g-band
    LMIC_setupChannel(2, 868500000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
    LMIC_setupChannel(3, 867100000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
    LMIC_setupChannel(4, 867300000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
    LMIC_setupChannel(5, 867500000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
    LMIC_setupChannel(6, 867700000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
    LMIC_setupChannel(7, 867900000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
    LMIC_setupChannel(8, 868800000, DR_RANGE_MAP(DR_FSK,  DR_FSK),  BAND_MILLI);      // g2-band
    // TTN defines an additional channel at 869.525Mhz using SF9 for class B
    // devices' ping slots. LMIC does not have an easy way to define set this
    // frequency and support for class B is spotty and untested, so this
    // frequency is not configured here.

    // Set static session parameters.
    LMIC_setSession (0x1, DEVADDR, NWKSKEY, APPSKEY);

    // Disable link check validation
    LMIC_setLinkCheckMode(0);

    // TTN uses SF9 for its RX2 window.
    LMIC.dn2Dr = DR_SF9;

    // Set data rate and transmit power for uplink (note: txpow seems to be ignored by the library)
    //LMIC_setDrTxpow(DR_SF7,14);
    forceTxSingleChannelDr(DR_SF7);

    mydisplay("Start","Waiting commands","",0);
}

void loop() {
    os_runloop_once();
    char v;
    char c = Serial.read();
    switch(c) {
      case 'h'  :
        help();
      break;

      case 'v'  :
        VERBOSE = !VERBOSE;
        Serial.println(F("OK"));
        Serial.println(F("*===================================================================================*"));
      break;

      case 'l' :
        digitalWrite(LEDPIN, !digitalRead(LEDPIN));
        Serial.println(F("OK"));
        Serial.println(F("*===================================================================================*"));
      break;

      case 'p'  :
        // payload
        uint8_t my_buff[MAX_BUFF_SIZE];
        r1 = esp_random();
        r2 = esp_random();
        /*int num = 0;
        char digit[2];
        int elements = 0;
        bool valid = false;
        c = Serial.read();
      /*  while(true){
          if (c == '!') {
            valid = true;
            break;
          }
          if ( (c >= '0' && c <= '9') || (hex >= 'A' && hex <= 'F') || (hex >= 'a' && hex <= 'f') ){
            c

          } else {
            valid = false;
            break;
          }
          c = Serial.read();
        }*/
      /*  if (valid) {
          // copy my_buff in buff
          // copy my_buff_size in buff_size
          Serial.println(F("OK"));
        }
        else Serial.println(F("Not OK"));
        Serial.println(F("*===================================================================================*"));*/
      break;

      case 'd'  :
        PrintHex8(buff, buff_size);Serial.println("");
        Serial.println(F("OK"));
        Serial.println(F("*===================================================================================*"));
      break;

      case 'S'  :
        HALT = false;
        counter = -1;
        do_send(&sendjob);
        Serial.println(F("OK"));
        Serial.println(F("*===================================================================================*"));
      break;

      case 'H'  :
        HALT = true;
        Serial.println(F("OK"));
        Serial.println(F("*===================================================================================*"));
      break;

      case 't'  :
        HALT = true;
        // Check if there is not a current TX/RX job running
        if (LMIC.opmode & OP_TXRXPEND) {
          Serial.println(F("OP_TXRXPEND, not sending"));
          Serial.println(F("*===================================================================================*"));
        } else {
          // Prepare upstream data transmission at the next possible time.
          LMIC_setTxData2(1, (uint8_t*)"0123456789", 10, 0);
          digitalWrite(LEDPIN, HIGH);
          ++counter;
          mydisplay("Start","Sending uplink packet...","",counter);
          Serial.println(F("OK"));
          Serial.println(F("*===================================================================================*"));
        }
      break;

      case 'c'  :
        Serial.print(F("NWKSKEY: "));PrintHex8(NWKSKEY, 16);Serial.println("");
        Serial.print(F("APPSKEY: "));PrintHex8(APPSKEY, 16);Serial.println("");
        Serial.print(F("DEVADDR: "));PrintHex8((uint8_t *)DEVADDR, 4);Serial.println("");
        Serial.println(F("OK"));
        Serial.println(F("*===================================================================================*"));
      break;

      case 'b'  :
        Serial.println(F("OK"));
        Serial.println(F("*===================================================================================*"));
      break;

      case 'r'  :
        TX_RETRASMISSION = !TX_RETRASMISSION;
        Serial.println(F("OK"));
        Serial.println(F("*===================================================================================*"));
      break;

      case 'w'  :
        v = Serial.read();
        while (v==0xff)
          v = Serial.read();
        switch(v){
          case '!':
            break;
        }
        Serial.println(F("OK"));
        Serial.println(F("*===================================================================================*"));
      break;

      case 'R' :
        r1 = esp_random();
        r2 = esp_random();
        Serial.println(F("OK"));
        Serial.println(F("*===================================================================================*"));
      break;

      case 'f'  :
        v = Serial.read();
        while (v==0xff)
          v = Serial.read();
        switch (v){
          case '7':
              Serial.println(F("OK"));
              forceTxSingleChannelDr(DR_SF7);
              SF=7;
          break;
          case '8':
              Serial.println(F("OK"));
              forceTxSingleChannelDr(DR_SF8);
              SF=8;
          break;
          case '9':
              Serial.println(F("OK"));
              forceTxSingleChannelDr(DR_SF9);
              SF=9;
          break;
          case '1':
              char v2 = Serial.read();
              while (v2==0xff)
                v2 = Serial.read();
              switch (v2) {
                case '0':
                  forceTxSingleChannelDr(DR_SF10);
                  SF=10;
                  Serial.println(F("OK"));
                break;
                case '1':
                  forceTxSingleChannelDr(DR_SF11);
                  SF=11;
                  Serial.println(F("OK"));
                break;
                case '2':
                  forceTxSingleChannelDr(DR_SF12);
                  SF=12;
                  Serial.println(F("OK"));
                break;
                default:
                  Serial.println(F("ERROR"));
              }
          break;
        }
        Serial.println(F("*===================================================================================*"));
      break;

    }
    delay(100);
}
