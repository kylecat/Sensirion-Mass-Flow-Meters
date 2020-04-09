/*
 Name:    ClassTest.ino
 Created: 2020/4/9 下午 03:22:43
 Author:  Liu
*/
//-- SFM Defines --
#include "SFM3000.h"
SFM3000 SFM;

//-- Fan Defines --
#define FAN_pin 2
unsigned int FAN_setting = 255;

float MassFlowRate;

void setup() {
    Serial.begin(115200);
    SFM.setDebuger(Serial);
    SFM.init();

    SFM.getSlop();
    SFM.getOffset();

    SFM.Start();
    pinMode(FAN_pin, OUTPUT);
    analogWrite(FAN_pin, FAN_setting);
}


void loop() {
    MassFlowRate = SFM.getFlowRate();
    Serial.print(MassFlowRate,4);
    Serial.println(" SLM");
    delay(1000);
}
