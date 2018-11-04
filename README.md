# LoraCast, the Swiss Army Knife of LoraWan
LoraCast, the Swiss Army Knife of LoraWan, is a LoraWan TTN compatible firmware for the cheap ESP32/Lora boards.

```
*===================================================================================*
 :::         ...    :::::::..    :::.       .,-:::::   :::.     .::::::.::::::::::::
 ;;;      .;;;;;;;. ;;;;``;;;;   ;;`;;    ,;;;'````'   ;;`;;   ;;;`    `;;;;;;;;''''
 [[[     ,[[     [[,[[[,/[[['  ,[[ '[[,  [[[         ,[[ '[[, '[==/[[[[,    [[
 $$'     $$$,     $$$$$$$$$c   c$$$cc$$$c $$$        c$$$cc$$$c  '''    $    $$
o88oo,.__"888,_ _,88P888b "88bo,888   888,`88bo,__,o, 888   888,88b    dP    88,
""""YUMMM  "YMMMMMP" MMMM   "W" YMM   ""`   "YUMMMMMP"YMM   ""`  "YMmMY"     MMM
                   LoraCast, the Swiss Army Knife of LoraWan
*===================================================================================*
                 LoraCast v.0.0.1 build on Oct 28 2018 21:26:41
*===================================================================================*

General commands:
   h - shows this help
   v - toggle verbosity
   l - toggle led
Communication commands:
   p[hex payload]! - charge payload in the memory max 1k byte
   d - dump actual payload present in the memory
   S - start transmission
   H - halt transmission
Advanced commands:
   t - send a Test message with content 0123456789 (ASCII)
   c - shows LoraWan/TTN Configuration
   b[]! - set message dimension in byte 0-100 (actual 100)
   r - toggle retransmission (actual 0)
   w[value]! - set delay between packets in second 0-255 (actual 0)
   f[] - set spreading factor between 7,8,9,10,11,12 (actual 7)
   R - generate a new random packet ID (actual 0x53 0x12)
*===================================================================================*
```

## General commands

- h - shows help and main configurations .
- v - enable/disable verbose debug messages.
- l - toggle the status of the blue led on the board.

## Communication commands

- p[hex payload]! - load the hex payload passed in the board memory (MAX 1k byte).
- d - dump the actual payload present in the board memory.
- S - start the transmission of the payload present in memory. The payload is broken on chunks compatibles with the maximum message dimension (set with b command).
- H - set the halt flag, next transmission will be halted.

## Advanced commands:

- t - send a Test message with content 0123456789 (ASCII), this command set the halt flag in order to stop next message.
- c - shows LoraWan/TTN configuration in hexadecimal format.
- b[]! - set the message maximum dimension in byte 0-100.
- r - toggle retransmission flag (useful for transmit the same payload many times)
- w[]! - set delay between packets in second 0-255.
- f[] - set spreading factor between 7,8,9,10,11,12.
- R - generate a new random packet ID

## Communication protocol
The payload is exploded in n messages based on message maximum dimension. Every message has the following format:

- LWC - magic number (3 bytes )
- * - separator (1 byte)
- 00 - packet ID (2 bytes) same ID for the same parts of the same payload
- * - separator (1 byte)
- 0 - message number (1 byte) progressive message counter
- 0 - total messages (1 byte) number of total messages needed to have the complete message
- * - separator (1 byte)
- ... - informative content (1-100 bytes) part of the complete message to be transmitted

## Available boards
The firmware is actually tested on the following boards:

- TTGO ESP32 Lora (with or without OLED)
- HelTec WiFi Lora 32 - consider [v.2](http://www.heltec.cn/project/wifi-lora-32/?lang=en)

## Usage examples
Some examples are provided in order to use LoraCast in some real scenarios.

- File - move a file in the memory of the board, send in via Lora and receive with provided python script.
- BTC - generate a new transaction (via RPC), send it via Lora, receive the complete transaction, check and broadcast it (using the RPC available from a core node) - Require a Bitcoin core working node.
- LTC - generate a new transaction (via RPC), send it via Lora, receive the complete transaction, check and broadcast it (using the RPC available from a core node) - Require a Litecoin working node.
