# ESP32-Web-Radio-and-Weather-Station.

## About
ESP32 Web Radio and Weather Station. This is a single station web radio. The Radio part was made by https://github.com/vincegellar . I used an NodeMCU ESP32-S, VS1003, OS-1838 IR reciver, SSD1306 display and a DHT11 sensor.

## Parts List
- NodeMCU ESP32-S.
- VS1003/VS1053 MP3 Decoder.
- DHT-11 sensor.
- OS-1838 IR reciver.
- SSD1306 display, 128x64.

## Dependencies
- VS1053 library by baldram (https://github.com/baldram/ESP_VS1053_Library)
- adafruit/Adafruit GFX Library @ 1.11.5
- adafruit/Adafruit SSD1306 @ 2.5.7
- adafruit/DHT sensor library @ 1.4.4
- arduino-libraries/Arduino_JSON @ 0.2.0
- z3t0/IRremote @ 4.1.2
- adafruit/Adafruit Unified Sensor @ 1.1.9

## Instructions:
- Build the hardware.
- In VSCode use PlatformIO and set it up. In the main.cpp you can find the .ini's content.
- Reg... on openweathermap.org to get an API key.
- Look for a station and set it up in main.cpp.
- Set your Wi-Fi SSID and Password.
- Upload the program. (You may need to hold down the boot button on the ESP and releases it when you see "Connecting...." in the terminal.)

## License:
ESP32-Web-Radio-and-Weather-Station: This is a single station radio, it uses a DHT11 sensor and gets the weather data from openweathermap.org.
Copyright (C) 2023 Krisztián Pfeifer.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
