/*
 * Copyright 2025, 2026 zjuergen66@gmail.com
 * This file is part of speedGun.
 * speedGun is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 * speedGun is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along with Foobar. If not, see <https://www.gnu.org/licenses/>.        
 *
 *
 * ========= speedGun ====================
 *
 * + buzzer digital pin 7
 *
 * + lcdisplay via i2c
 * 		arduino SDA ist A4 ( gelb )
 * 		arduino SCL ist A5 ( gruen )
 *
 * + sensor HLK-LD2415H via SoftwareSerial:
 *		arduino RX is digital pin 2 (connect to TX of sensor = white)
 * 		arduino TX is digital pin 3 (connect to RX of sensor = green)
 * 		https://docs.arduino.cc/learn/built-in-libraries/software-serial/
 *
 * the sensor:
 * 		https://github.com/dm5tt/LD2415H_ReverseEngineering
 * 		https://forum.arduino.cc/t/hlk-ld2415h-speed-sensor/1378692
 * 		https://github.com/cptskippy/esphome.ld2415h
 * 		https://github.com/cptskippy/esphome.ld2415h/blob/main/components/ld2415h/ld2415h.h
 */

