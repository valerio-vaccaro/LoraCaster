# LoraCast, the Swiss Army Knife of LoraWan
LoraCast, the Swiss Army Knife of LoraWan, is a LoraWan TTN compatible firmware for a cheap ESP32/Lora board.

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
   t - sent a Test message with content 0123456789 (ASCII)
   c - shows LoraWan/TTN Configuration
   b[]! - set message dimension in byte 0-100 (actual 100)
   r - toggle retransmission (actual 0)
   w[value]! - set delay between packets in second 0-255 (actual 0)
   fnn - set spreading factor (nn) between 7,8,9,10,11,12 (actual 7)
   R - generate a new random packet identificator (actual 0x53 0x12)
*===================================================================================*
```

## General commands

### h
Shows help and main configurations .

### v - toggle verbosity
Enable/disable verbose debug messages.

### l - toggle led
Toggle the status of the blue led on the board.

## Communication commands

### p[hex payload]!
Load the hex payload passed in the board memory (MAX 1k byte).

### d
Dump the actual payload present in the board memory.

### S
Start the transmission of the payload present in memory. The payload is broken on
chunks compatibles with the maximum message dimension (set with b command).

### H
set the halt flag, next transmission will be halted.

## Advanced commands:

### t
Sent a Test message with content 0123456789 (ASCII), this command set the halt
flag in order to stop next message.

### c
Shows LoraWan/TTN configuration in hexadecimal format.

### b[]!
Set the message maximum dimension in byte 0-100.

### r
Toggle retransmission flag (useful for transmit the same payload many times)

### w[]!
Set delay between packets in second 0-255.

### f[sf]
Set spreading factor between 7,8,9,10,11,12.

### R
Generate a new random packet ID
