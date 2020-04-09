/*
 Name:		SFM3000.ino
 Created:	2020/4/5 下午 12:49:18
 Author:	Liu
*/

//-- Defines  for SFM3000-------------------------------------------------------------------
// I2C Address
#include <Wire.h>
#define SFM3000_I2C		0x40 // 64
// CRC
#define CRC_POLYNOMIAL 0x131 // P(x) = x^8 + x^5 + x^4 + 1 = 100110001

//-- Defines -------------------------------------------------------------------
#define FAN_pin 2
unsigned int FAN_setting = 255;

byte CRC = 0;

uint8_t DATA[2];   // 2bit 資料 1bit CRC8：CRC另外放在全域變數當中

void setup() {
	// init serial
	Serial.begin(115200);

	pinMode(FAN_pin, OUTPUT);
	analogWrite(FAN_pin, FAN_setting);

	
	int _reset = 0x2000;
	byte _cmd_H = _reset >> 8;
	byte _cmd_L = _reset | _cmd_H << 8;
	
	if (_cmd_H < 16)	Serial.print("0");
	Serial.print(_cmd_H, HEX);
	
	Serial.print(" ");

	if(_cmd_L<16)	Serial.print("0");
	Serial.println(_cmd_L, HEX);

	int ret;
	Wire.begin();

	do {
		delay(1000);
		// Soft reset the sensor

		Wire.beginTransmission(SFM3000_I2C);
		Wire.write(_cmd_H);
		Wire.write(_cmd_L);
		ret = Wire.endTransmission();
		if (ret != 0) {
			Serial.println("Error while sending soft reset command, retrying..."); \
		}

		delay(1000);
		// Switch to measurement mode
		Wire.beginTransmission(SFM3000_I2C);
		Wire.write(byte(0x10));
		Wire.write(byte(0x00));
		ret = Wire.endTransmission();
		if (ret != 0) {
			Serial.println("Error during write measurement mode command");
		}
	} while (ret != 0);

	Serial.println("Setup Done");
}

void loop() {
	//Wire.beginTransmission(SFM3000_I2C);
	Wire.requestFrom(SFM3000_I2C, 3);
	DATA[0] = Wire.read(); // first received byte stored here
	DATA[1] = Wire.read(); // second received byte stored here
	CRC= Wire.read(); // third received byte stored here
	Wire.endTransmission();

	/*
	Serial.print(DATA[0], HEX);
	Serial.print(" ");
	Serial.println(DATA[1], HEX);
	*/


	crc8(DATA,CRC);
	int value = getValue(DATA);
	float flowRate = (float(value) - 32000) / 140.0;


	Serial.print("Value: " + String(value));
	Serial.println("\tFlow: " + String(flowRate));

	FAN_setting -= 10;
	if (FAN_setting < 60) FAN_setting = 255;
//	analogWrite(FAN_pin, FAN_setting);

	delay(100);
}

uint8_t crc8(uint8_t* _data, uint8_t _checksum)
{
	uint8_t _bit;					// bit mask
	uint8_t _byteCtr;			// byte counter
	uint8_t _calcCrc = 0;		// calculated checksum
	uint8_t _result = false;

	for (uint8_t _byteCtr = 0; _byteCtr < 2; _byteCtr++)
	{
		_calcCrc ^= (_data[_byteCtr]);


		for (_bit = 8; _bit > 0; --_bit)
		{
			if (_calcCrc & 0x80)  _calcCrc = (_calcCrc << 1) ^ CRC_POLYNOMIAL;
			else					      _calcCrc = (_calcCrc << 1);
		}
	}

	if (_calcCrc != _checksum) 		Serial.println("[ERROR] CRC faile: C( " + String(_calcCrc) + " )-O( " + String(_checksum) + " )");
	else {
		//Serial.println("[CRC] Checked: C( " + String(_calcCrc) + " )-O( " + String(_checksum) + " )");
		_result = true;
	}
	return _result;
}

int getValue(uint8_t* _data)
{
	int _value = _data[0] << 8 | _data[1];
	return _value;
}
