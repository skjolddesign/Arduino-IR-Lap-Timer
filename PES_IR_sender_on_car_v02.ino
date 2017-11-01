/*
TRANSMITTER CODE FOR IR LAPT TIMER, BY PER EMIL SKJOLD

You need: 
1x "arduino pro mini"
1x R1 1000K resistor. 
1x R2, 68ohm resistor (this must be calculated to match your led forward current)
1x LED, IR LED, DC FORWARD CURRENT:30~100mA.
1x T1, 2N2222A transistor

//Wiring:
//bat.+6v -> R2 -> LED anode -> LED katode -> T1 Collector -> T1 Emitter -> bat.0v
//Arduino D3 -> R1 -> T1 base
//bat +6v to Arduino VIN
//bat 0v to arduino GND


 * IRremote: IRsendDemo - demonstrates sending IR codes with IRsend
 * An IR LED must be connected to Arduino PWM pin 3.
 * Version 0.1 July, 2009
 * Copyright 2009 Ken Shirriff
 * http://arcfn.com
 */

#include <IRremote.h>
//#include "LowPower.h"
IRsend irsend;
void setup()
{
  Serial.begin(9600);
}

void loop() {
//TEST
// irsend.sendSony(0x810, 12); // Sony TV power code
//  irsend.sendSony(0x810, 4); // Sony TV power code
//  irsend.sendSony(0x2, 4); // Sony TV power code
//  irsend.sendNEC(0x3, 4);    
//  irsend.sendRC5(0x10099089, 30); // TESTING LONG STRING, CAR 010, TEMP 099, VOLT 08,9. (problem receiving this one)
//irsend.sendRaw(0X123, 3, 38);

//IN USE:
//CAR 1
irsend.sendRC6(0x1, 4); //code and signal bits.
//CAR 2
//irsend.sendRC6(0x2, 4); //code and signal bits.
// 4bit er rask og kan telle opp til 9. bruk code repeater for rceiver


 delay(100); //PLAY WITH THIS ONE
//LowPower.powerDown(SLEEP_60MS, ADC_OFF, BOD_OFF);  //SLEEP
}

