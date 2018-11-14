# lora_gps_tracker
Simple LoRa/GPS tracker demonstrator. Arduino based, with two sketches: tracker and hq.

## Tracker
- Feather 32u4 proto
- NEO uBlox GPS
- Ra-02 (Ai Thinker) Lora Radio 433MHz
- OLED (adafruit I2C .96")

## HQ
- Feather 32u4 proto
- Ra-02 (Ai Thinker) Lora Radio 433MHz

## Installation / Setup
A symbolic link for 'lg_shared' should be created in arduino libraries folder.

For example (OSx command line example):
- ln -s /Users/aduvenhage/wok/lora_gps_tracker/lg_shared/ /Users/aduvenhage/Documents/Arduino/libraries/



