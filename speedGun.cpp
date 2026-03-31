
/*  speedGun
 *  
 * Copyright 2025, 2026 zjuergen66@gmail.com
 * This file is part of speedGun.
 * speedGun is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 * speedGun is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along with Foobar. If not, see <https://www.gnu.org/licenses/>.        
 */


// Defines constants ------------------------------------------------

#include <LibPrintf.h>

#if (ARDUINO < 10000)
#include <Wire/src/Wire.h>
#else
#include <Wire.h>
#endif
#include "LiquidCrystal_I2C.h"
#include "SoftwareSerial.h"




#define VERSION "1.4"
#define DEBUG 0
#define SOUND HIGH // HIGH==on or LOW==off

#define BUZZER_PIN 7

#define LCD_COLS 16
#define LCD_ROWS 2
#define FRAMES_POS 10,0

#define LD2415H_SENSITIVITY  0x0B // aus [0x01;0x0F], default 0x05  : the larger the sensitivity value, the lower the sensitivity, that is, so the closer the detection distance (strong anti-interference
#define LD2415H_ANGLE_COMPENSATION 0x05 // 0x0A == 10 degrees, 0x00 == 0 degrees
#define LD2415H_VIBRATION_COMPENSATION  0x20 // aus [0x00;0x70], default 0x00  :
#define LD2415H_FRAMES_PER_SECOND 0x03 // 0x00 == 22fps, 0x01 == 11fps, 0x02 == 6fps , 0x03 == 3fps
#define LD2415H_COME_AND_GO 0x00 // 0x00 == come+go, 0x01 == come, 0x02 == go
#define LD2415H_LOWER_SPEED_LIMIT 0x02 // 0x01 1km/h

#define DELAY_LOOP 100 // in ms	// delay at end of loop ==> display update freq
#define MAX_OVER_LOOPS (2000/DELAY_LOOP) // Intervalllaenge fuer Maximum-Bildung 2000ms

// ------------------ declarations ------------------------

LiquidCrystal_I2C lcd(0x27,2,1,0,4,5,6,7,3,POSITIVE); // OK fuer meine speedGun Hardware typ2 @ 0x27
//LiquidCrystal_I2C lcd(0x3F,2,1,0,4,5,6,7,3,POSITIVE);

SoftwareSerial sensorSerial(2, 3); // RX, TX

//----------------------------------------------------------------------------------
#define BUZ_ON   digitalWrite(BUZZER_PIN, SOUND);
#define BUZ_OFF  digitalWrite(BUZZER_PIN, LOW);
//#define BUZ_ON   {PORTB |= _BV(5);}
//#define BUZ_OFF  {PORTB &= ~_BV(5);}
#define BUZ_TICK 1
#define BUZ_CLICK 5

void buzz(int n) {
	BUZ_ON;
	delay(n);
	BUZ_OFF
}
void fanfare(uint8_t max) {
	for (uint8_t i=1; i < max; i++) {buzz(i); delay(5*max -i*3); }
}
//----------------------------------------------------------------------------------
void lcdSetLine(byte line) { // line = [ 0 ; 9 ]
	//lcd.setCursor(0,line); lcd.print(F("                    "));
	lcd.setCursor(0,line);
}

//----------------------------------------------------------------------------------
void resetToStandardMode() {
	uint8_t cmd1[] = {0xFA, 0x31, 0x30, 0x30, 0xFB};
	uint8_t cmd2[] = {0xFA, 0x55, 0xAA, 0xFF, 0xFB};
	sensorSerial.write(cmd1, sizeof(cmd1));
	delay(1000);
	sensorSerial.write(cmd2, sizeof(cmd2));
	delay(1000);
	Serial.println(F("Sent command to reset to standard protocol mode"));
}


void setComingAndGoing() {
	//uint8_t cmd[] = {0x43, 0x46, 0x02, 0x00, 0x003, 0x00, 0x0D, 0x0A}; // 0x03 for ~3 frames/second
	uint8_t cmd[] = {0x43, 0x46, 0x02, LD2415H_COME_AND_GO, LD2415H_FRAMES_PER_SECOND, 0x00, 0x0D, 0x0A};
	sensorSerial.write(cmd, sizeof(cmd));
	delay(100);
	Serial.println(F("Sent command to detect directions and framerate"));
}

void setSensitivityAndAngle(uint8_t sensitivity, uint8_t angle) {
	uint8_t cmd[] = {0x43, 0x46, 0x01, LD2415H_LOWER_SPEED_LIMIT, angle, sensitivity, 0x0D, 0x0A};
	sensorSerial.write(cmd, sizeof(cmd));
	delay(100);
	Serial.println(F("Sent command to set sensitivity and angle compensation"));
}

void setAntiVibration(uint8_t value) {
	uint8_t cmd[] = {0x43, 0x46, 0x03, value, 0x00, 0x00, 0x00, 0x00, 0x0D, 0x0A};
	sensorSerial.write(cmd, sizeof(cmd));
	delay(100);
	Serial.println(F("Sent command to set anti-vibration"));
}

void flushSensorSerial() {
	while (sensorSerial.available()) {
		sensorSerial.read();
	}
	Serial.println(F("Flushed sensorSerial buffer"));
}

void printParameterSettings() {
	Serial.println(F("--- LD2415H Parameter Settings (Assumed/Reset) ---"));
	Serial.println(F("Protocol Mode: Standard (V+ddd.d or V-ddd.d)"));
	Serial.print(F("Speed Range Lower Limit: "));
		Serial.print(LD2415H_LOWER_SPEED_LIMIT);
		Serial.println(F("km/h"));
	Serial.print(F("Angle Compensation: "));
		Serial.print(LD2415H_ANGLE_COMPENSATION);
		Serial.println(F(" degrees"));
	Serial.print(F("Sensitivity: 0x"));
		Serial.print(LD2415H_SENSITIVITY, HEX);
		Serial.println(F(" aus [0x01;0x0F], default 0x05"));
	Serial.print(F("Target Detection: "));
		switch(LD2415H_COME_AND_GO) {
			case 0x00:
				Serial.println(F(" coming + and going -"));
				break;
			case 0x01:
				Serial.println(F("coming + "));
				break;
			case 0x02:
				Serial.println(F("going -"));
				break;
		}
	Serial.print(F("Output Rate: 0x"));
		Serial.print(LD2415H_FRAMES_PER_SECOND, HEX);
		Serial.println(F(" 0x00 == 22fps, 0x01 == 11fps, 0x02 == 6fps , 0x03 == 3fps"));
	Serial.println(F("Speed Unit: km/h"));
	Serial.print(F("Anti-Vibration: 0x"));
		Serial.print(LD2415H_VIBRATION_COMPENSATION, HEX);
		Serial.println(F(" aus [0x00;0x70], default 0x00, 0x20 (moderate)"));
	Serial.println(F("-----------------------------------------------"));
}

//----------------------------------------------------------------------------------
void paramsScreen() {
	lcd.clear();
	lcd.setCursor(0,0);
	//           0123456789.123456789
	lcd.print(F("SpeedLow="));
		lcd.print(LD2415H_LOWER_SPEED_LIMIT);
		lcd.print(F("km/h"));
	lcd.setCursor(0,1);
	lcd.print(F("AngleComp="));
		lcd.print(LD2415H_ANGLE_COMPENSATION);
		lcd.print(F("deg"));
	delay(2500); lcd.clear();

	lcd.setCursor(0,0);
	lcd.print(F("Sensitivity=0x"));
		lcd.print(LD2415H_SENSITIVITY, HEX);
	lcd.setCursor(0,1);
	lcd.print(F("Dir="));
			switch(LD2415H_COME_AND_GO) {
				case 0x00:
					lcd.print(F("come+ & go-"));
					//			   456789.12345
					break;
				case 0x01:
					lcd.print(F("come+"));
					break;
				case 0x02:
					lcd.print(F("go-"));
					break;
			}
	delay(2500); lcd.clear();

	lcd.setCursor(0,0);
	lcd.print(F("OutpRate="));
		switch(LD2415H_FRAMES_PER_SECOND) {
			case 0x00:
				lcd.print(F("22pfs"));
				//			   456789.12345
				break;
			case 0x01:
				lcd.print(F("11fps"));
				break;
			case 0x02:
				lcd.print(F("6fps"));
				break;
			case 0x03:
				lcd.print(F("3pfs"));
				break;
			default:
				lcd.print(F("0x"));
				lcd.print(LD2415H_FRAMES_PER_SECOND, HEX);

		}
	lcd.setCursor(0,1);
	lcd.print(F("AntiVib=0x"));
			lcd.print(LD2415H_VIBRATION_COMPENSATION, HEX);
	delay(2500);
	return ;
}

//----------------------------------------------------------------------------------

void splashScreen() {
	lcd.clear();
	Serial.println(F("\n=== HiHo ===================================================\n"));
	Serial.print(F("Version: "));
	Serial.println(VERSION);

	lcd.clear();
	lcd.setCursor(0,0);
	//           0123456789.123456789 // last 4 chars reserved for version
	lcd.print(F("speedGun v"));
	lcd.print(VERSION);
	lcd.setCursor(0,1);
	//         0123456789.12345.... 
	lcd.print(F("mit HLK-LD2415H"));

	return ;
}
//----------------------------------------------------------------------------------
void maskScreen() {
	lcd.clear();
	lcd.setCursor(6,0);
	//           0123456789.123456789
	lcd.print(F(      "kmh"));
	return ;
}



//----------------------------------------------------------------------------------

void setup(){
	Serial.begin(38400);
	lcd.begin(LCD_COLS,LCD_ROWS);
	splashScreen();

	pinMode(BUZZER_PIN, OUTPUT);

	sensorSerial.begin(9600);

	// Reset sensor settings
	resetToStandardMode();
	setComingAndGoing();
	setSensitivityAndAngle(LD2415H_SENSITIVITY, LD2415H_ANGLE_COMPENSATION); // Sensitivity 0x0B, angle 10°
	setAntiVibration(0x20); // Moderate anti-vibration

	Serial.println(F("-setup done"));
	fanfare(11);
	delay(1000);

	printParameterSettings();
	paramsScreen(); // the sensor params
	delay(1000);

	maskScreen(); // the static parts

	Serial.println(F("-setup end."));
}

bool isValidSpeedStr(const char* str) {
	return (isdigit(str[0]) && isdigit(str[1]) && isdigit(str[2]) && str[3] == '.' && isdigit(str[4]));
}

void loop(){
	static uint32_t validFrames = 0;
	static uint32_t lastValidFrameTime = 0;
	static uint8_t invalidFrameCount = 0;
	static String lastSpeed = "";
	static String lastDirection = "";
	static String lastFrame = "";
	char buffer[9];
	static String NN = F("NN  ");
	static String blanks5 = F("     ");

	static float maxSpeed = 0; // das Betrags-Max der gemessenen Speeds aus dem vorhergehenden Intervall
	static String maxSpeedString = "";
	static String maxDirection = "";
	static uint16_t maxOverLoopsCounter = 0; // counter fuer das Intervall (wird hochgezaehlt)

	static uint8_t error = 0; // Fehlernummer , auch index in errorText Array
	static uint8_t lastError = 0;
	static uint8_t errorDisplayCounter = 0; // Zaehler fuer Loop-Durchgaenge - fuer so viele wird der Fehlertext angezeigt
	static String errorText[5] = {
				F("                "), // 0
				F("Invalid speed   "), // 1
				F("Invalid frame   "), // 2
				F("Find frame start"), // 3
				 };
	//			   0123456789.12345

#if DEBUG
	lcd.setCursor(0,1);
	lcd.print( millis() );
	Serial.println( millis() );
#endif
	buzz(BUZ_TICK);

	if (sensorSerial.available()) {
		if (sensorSerial.peek() == 'V') {
			// ein 'V' kommt, das ist der Frame-Anfang
			if (sensorSerial.available() >= 9) {
				// ein Datensatz mit Frame-Laenge
				for (int i = 0; i < 9; i++) {
					buffer[i] = sensorSerial.read();
				}
#if DEBUG
				Serial.print("Raw frame: ");
				for (int i = 0; i < 9; i++) {
					Serial.print(buffer[i], HEX);
					Serial.print(" ");
				}
				Serial.println();
#endif
				if (buffer[0] == 'V' && buffer[7] == 0x0D && buffer[8] == 0x0A && (buffer[1] == '+' || buffer[1] == '-')) {
					// ein valid frame erhalten, aufbereiten & darstellen :

					String direction = (buffer[1] == '+') ? "+":"-"; //"Coming" : "Going";

					char speedStr[6];
					for (int i = 0; i < 5; i++) {
						speedStr[i] = buffer[i + 2];
					}
					speedStr[5] = '\0';

					if (isValidSpeedStr(speedStr)) {
						float speed = atof(speedStr);
						validFrames++;
						lastValidFrameTime = millis();
						invalidFrameCount = 0;

#if DEBUG
						// Serial output
						Serial.print("Frame #"); Serial.print(validFrames);
						Serial.print(" | Direction: "); Serial.print(direction);
						Serial.print(" | Speed: "); Serial.print(speed, 1);
						Serial.println(" km/h");
#endif
						// lcd-data update
						String newSpeed = String(speed, 1);
						String newFrame = String(validFrames % 1000);

						// output direction, speed, frames to lcd
						if (direction != lastDirection) {
							lcd.setCursor(0,0);
							lcd.print(direction);
							lastDirection = direction;
						}
						if (newSpeed != lastSpeed) {
							lcd.setCursor(1,0);
							lcd.print(blanks5);
							lcd.setCursor(1,0);
							lcd.print(newSpeed);
							lastSpeed = newSpeed;
						}
//						if (newFrame != lastFrame) {
//							lcd.setCursor(FRAMES_POS);
//							lcd.print(newFrame);
//							lastFrame = newFrame;
//						}

						if ( abs(speed) > abs(maxSpeed) ) {
							maxSpeed = speed;
							maxSpeedString = newSpeed;
							maxDirection = direction;
						}

					} else {
						// Invalid speed format
						error = 1;
						Serial.println( errorText[error] );
						invalidFrameCount++;
					}
				} else {
					// Invalid frame received
					error = 2;
					Serial.println( errorText[error] );
					invalidFrameCount++;
				}

				// Flush buffer if too many invalid frames
				if (invalidFrameCount > 5) {
					flushSensorSerial();
					invalidFrameCount = 0;
				}
			} // end ein Datensatz mit Frame-Laenge
		} else {
			// es kommt nicht das 'V', also kein gueltiger Frame-Anfang. Deshalb wegschmeissen :
			sensorSerial.read();
			// Discarding byte to find frame start
			error = 3;
			Serial.println( errorText[error] );
		}
	}

	// Show "NN" for speed and direction and frames if no valid frames for 5 seconds

	if (millis() - lastValidFrameTime > 5000) {
		if (lastDirection != NN) {
			lcd.setCursor(0,0);
			lcd.print('.');
			lastDirection = NN;
		}
		if (lastSpeed != NN) {
			lastSpeed = NN;
			lcd.setCursor(1,0);
			lcd.print(lastSpeed);
		}
//		if (lastFrame != NN) {
//			lastFrame = NN;
//			lcd.setCursor(FRAMES_POS);
//			lcd.print(lastFrame);
//		}
	}

	if (maxOverLoopsCounter++ >= MAX_OVER_LOOPS) {
		lcd.setCursor(FRAMES_POS);
		//			 .12345
		lcd.print(F("      "));
		lcd.setCursor(FRAMES_POS);
		lcd.print(maxDirection);
		lcd.print(maxSpeedString);
		// und neu initialisieren
		maxSpeed = 0;
		maxSpeedString = "";
		maxDirection = "";
		maxOverLoopsCounter = 0;
	}

	if (error > 0) {
		// Fehler aufgetreten
		buzz(BUZ_CLICK);
		if (error != lastError) { // der Fehler ist anders (entweder ein anderer Fehler als der bisherige Fehler oder neu )
/*			nicht noetig - alle Texte haben volle Zeilenlanege
			if (lastError != 0) { // den vorherigen jetzt veralteten Text loeschen
				lcd.setCursor(0,1); // 2.te Zeile
				lcd.print(errorText[0]); // Zeile loeschen
			}
*/
			lcd.setCursor(0,1); // 2.te Zeile
			lcd.print(errorText[error]); // aktuellen Fehlertext schreiben
		}
		lastError = error;
		error = 0;
		errorDisplayCounter = 1000 /DELAY_LOOP; // effektiv den neuen Fehler nach einer Sekunde wieder loeschen
	}
	if (errorDisplayCounter > 0) {
		errorDisplayCounter--;
		if (errorDisplayCounter == 0) { // lange genug angezeigt, jetzt wieder loeschen
			lcd.setCursor(0,1); // 2.te Zeile
			lcd.print(errorText[0]); // Zeile loeschen
			lastError = 0;
		}
	}

	delay(DELAY_LOOP);
}
