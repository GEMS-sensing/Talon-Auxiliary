#include <Wire.h>
 // #include "Adafruit_MCP23008.h"
// #include <Adafruit_MCP23X17.h>
// #include <MCP2301x.h>
#include <Arduino.h>
#include <MCP23018.h>

// #define NUM_MCP2301x_ICs   1

//Input Pins
const uint8_t TX = 1;
const uint8_t RX = 0;

//IO Expander Pins - ALPHA
const uint8_t REG_EN = 7;
const uint8_t ADC_INT = 6;

//IO Expander Pins - BETA
const uint8_t LOAD = 7;
const uint8_t RST = 6;
const uint8_t OUT1 = 0;
// const uint8_t COUNT_EN3 = 5;
// const uint8_t COUNT_EN2 = 4;
const uint8_t COUNT_EN1 = 3;
const uint8_t OVF1 = 8;

const uint8_t EN1 = 1;
const uint8_t FAULT1 = 0;

//Configured for pins on the Particle Boron
//const uint8_t EXT_EN = 8;
//const uint8_t SDA_CTRL = 2;

 const uint8_t EXT_EN = 13;
 const uint8_t SDA_CTRL = 5;

char ReadArray[25] = {0};

 // Adafruit_MCP23008 io;
// Adafruit_MCP23X17 io;
MCP23018 ioAlpha(0x20);
MCP23018 ioBeta(0x23);
MCP23018 ioGamma(0x24);
// MCP2301x io;

// #define IOEXP_MODE  (IOCON_INTCC | IOCON_INTPOL | IOCON_ODR | IOCON_MIRROR)
// #define ADDRESS      (MCP2301X_CTRL_ID+7)
// #define LED_PIN      D4   // WeMOS D1 & D1 mini
// #define IN_PORT      0
// #define OUT_PORT     1

// #define RELAY_ON      LOW
// #define RELAY_OFF     HIGH
// #define ALL_OFF       ALL_LOW

// #define DIR 0x00
// #define GPIO 0x12
// #define LAT 0x14
// #define PU 0x0C
// #define ADR 0x20

///////////// ANALOG SENSING /////////////
#include <Adafruit_ADS1015.h>

Adafruit_ADS1115 ads(0x49); 

const float VoltageDiv = 0.1875; //Program voltage divider [mV/bit]

void setup() {

	pinMode(EXT_EN, OUTPUT); 
	digitalWrite(EXT_EN, HIGH);
	Serial.begin(115200);
	// delay(5000);
	pinMode(SDA_CTRL, OUTPUT);
	digitalWrite(SDA_CTRL, LOW);
	delay(5000);

	Wire.begin();
	Serial.println(ioAlpha.begin());
	Serial.println(ioBeta.begin());
	Serial.println(ioGamma.begin());
 //DEBUG1!
	SetDefaultPins(); //Set the default configuration for sensing 
	delay(1000);
	Serial.println("Begin Aux Demo...");
}

void loop() {
	// io.pinMode(0, TX, LOW);
	// io.pinMode(0, FOut, LOW);
	// delay(1000);
	// io.pinMode(0, FOut, LOW);
	// io.pinMode(0, TX, HIGH);
	// delay(1000);
	// io.pinMode(0, FOut, HIGH);
	// io.pinMode(0, TX, LOW);
	// delay(1000);
	// io.pinMode(0, FOut, HIGH);
	// io.pinMode(0, TX, HIGH);
	// delay(1000);
	static int ReadLength = 0;
  	String ReadString;
	if(Serial.available() > 0) {
    char Input = Serial.read();

    // Increment counter while waiting for carrage return or newline
    // if(Input != 13 || Input != 10) {
    if(Input != '#') { //Wait for SDI12 line end or control line end 
//      Serial.println(Input); //DEBUG!
      ReadArray[ReadLength] = Input;
//      Serial.println(ReadArray[ReadLength]); //DEBUG!
      ReadLength++;
    }

    // if(Input == 13 || Input == 10) { // carriage or newline
	//Format - ppCs#
	//pp - two digit pin number (base 10)
	//C - command ('M' - Mode, 'R' - Read, 'W' - Write)
	//s - state (0 - Output/Off, 1 - Input/On), required only for write and mode operations 
	if(Input == '#') { //Control line ending
		ReadString = String(ReadArray);
		ReadString.trim();
		memset(ReadArray, 0, sizeof(ReadArray));
		ReadLength = 0;
		int Val = (ReadString.substring(0,2)).toInt(); //Grab the pin to operate on
		String Operation = ReadString.substring(2,3); //Grab the middle char
		int State = (ReadString.substring(3,4)).toInt(); //Grab the state to set
		unsigned int Result = 0; //Used to grab result of either setting of pin mode or response from a read

		if(Operation.equals("M")) { //if call is for pinmode setting
			Result = ioAlpha.pinMode(Val, !State); //Use inverse of state to corespond with 1 = input, 0 = output
		}

		if(Operation.equals("m")) { //if call is for taking measurment of counter
			Result = ReadCounters(Val, true); //Return desired counter after taking new measure
		}
		
		if(Operation.equals("R")) { //If call is for digital read
			Result = ioAlpha.digitalRead(Val);
		}

		if(Operation.equals("r")) { //if call is for taking reading of counter
			Result = ReadCounters(Val, false); //Read previously retrived value for counter
		}

		if(Operation.equals("W")) { //If call is for digital write
			Result = ioAlpha.digitalWrite(Val, State);

		}

		if(!Operation.equals("S")) { //If any function but voltage sense called, report normally
			Serial.print(">");
			Serial.println(ReadString); //Echo back to serial monitor
			Serial.print(">");
			Serial.println(Result); //Return various result 
		}

		if(Operation.equals("S")) { //Run voltage sense program
			VoltageSense();
		}
		
	}
	// GetAddress();
	// delay(5000);

}
}

void SetDefaultPins()
{
	for(int i = 0; i < 16; i++) { //Set all Gamma pins to input
		ioGamma.pinMode(i, INPUT_PULLUP);
	} 

	for(int i = 8; i < 16; i++) { //Set all Alpha pins to input
		ioAlpha.pinMode(i, INPUT_PULLUP);
		// ioAlpha.pinMode(i, OUTPUT);
		// ioAlpha.digitalWrite(i, LOW);
	} 

	for(int i = 11; i < 16; i++) { //Set all Beta pins to input
		ioBeta.pinMode(i, INPUT_PULLUP);
		// ioBeta.pinMode(i, OUTPUT);
		// ioBeta.digitalWrite(i, LOW);
	} 


	for(int i = 0; i < 3; i++) { //Set each group of IO pins seperately
		ioBeta.pinMode(OUT1 + i, INPUT); //Set all outputs from counters as inputs to I/O expander
		ioBeta.pinMode(COUNT_EN1 + i, OUTPUT); //Set enable control as output
		ioBeta.pinMode(OVF1 + i, INPUT); //Set overflow pins as inputs
		ioBeta.digitalWrite(COUNT_EN1 + i, HIGH); //Default to enable all counters
		ioAlpha.pinMode(FAULT1 + 2*i, INPUT_PULLUP);
		ioAlpha.pinMode(EN1 + 2*i, OUTPUT);
		ioAlpha.digitalWrite(EN1 + 2*i, HIGH);
	}
	ioBeta.pinMode(LOAD, OUTPUT);
	ioBeta.pinMode(RST, OUTPUT);
	ioBeta.digitalWrite(LOAD, LOW); //Start with load in low position, will load on high going edge
	ioBeta.digitalWrite(RST, HIGH); //Negate reset by default

	ioAlpha.pinMode(REG_EN, OUTPUT);
	ioAlpha.digitalWrite(REG_EN, HIGH); //Default 5V to on
	// ioAlpha.pinMode(ADC_INT, INPUT_PULLUP); //Setup ADC int as an input pullup for the open drain
	ioAlpha.pinMode(ADC_INT, INPUT); //Setup ADC int as an input pullup for the open drain
}

void VoltageSense() //Voltage sense (AKA Sensor0)
{
	Serial.print(">Voltage Sense:\n");
	ads.begin();

	for(int i = 0; i < 4; i++){ //Increment through 4 voltages
  		Serial.print("\tPort");
  		Serial.print(i);
  		Serial.print(": ");
  		Serial.print(ads.readADC_SingleEnded(i)*VoltageDiv, 4); //Print high resolution voltage
  		Serial.print(" mV\n");  
	}
}

uint16_t ReadCounters(uint8_t Port, bool NewRead) //Pass in which counter to read
{
	// static uint16_t Count1 = 0; //Keep all counters as local statics to be returned at request
	// static uint16_t Count2 = 0;
	// static uint16_t Count3 = 0; 

	static uint16_t Count[3] = {0}; //Keep all counters as local statics to be returned at request 

	if(NewRead) {
		ioBeta.digitalWrite(LOAD, HIGH); //Load all values
		delayMicroseconds(1); //Delay min number of microseconds to ensure a pulse
		ioBeta.digitalWrite(LOAD, LOW);

		for(int i = 0; i < 3; i++) { //Get count n
			ioBeta.digitalWrite(COUNT_EN1 + i, LOW); //Turn on outputs of counter n 
			Count[i] = ioGamma.readBus(); //Grab bus
			ioBeta.digitalWrite(COUNT_EN1 + i, HIGH); //Turn off outputs of countern
		}

		// ioBeta.digitalWrite(COUNT_EN1, HIGH); //Get count 1
		// Count1 = ioGamma.readWord();
		// ioBeta.digitalWrite(COUNT_EN1, LOW);

		// ioBeta.digitalWrite(COUNT_EN2, HIGH); //Get count 2
		// Count2 = ioGamma.readWord();
		// ioBeta.digitalWrite(COUNT_EN2, LOW);

		// ioBeta.digitalWrite(COUNT_EN3, HIGH); //Get count 3
		// Count3 = ioGamma.readWord();
		// ioBeta.digitalWrite(COUNT_EN3, LOW);
	}

	return Count[Port]; //Return the desired 16 bit count
}

void ClearCounters() //Clear all counters
{
	ioBeta.digitalWrite(RST, LOW);
	// ASM('NOP'); //Delay minimal time, needs only to be longer than 200ns, can likely eliminate delay
	delayMicroseconds(1); //Delay min number of microseconds to ensure a pulse
	ioBeta.digitalWrite(RST, HIGH);
}


