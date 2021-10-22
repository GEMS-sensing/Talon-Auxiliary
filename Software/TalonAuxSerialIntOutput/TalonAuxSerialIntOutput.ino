#include <Wire.h>
#include <Arduino.h>
#include <MCP23018.h>

// #define OVERFLOW_ONLY 

//Input Pins
const uint8_t GPIOA = 5;
const uint8_t GPIOB = 6;

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

//Configured for pins on the Particle Boron
//const uint8_t EXT_EN = 8;
//const uint8_t SDA_CTRL = 2;

const uint8_t EXT_EN = 13;
const uint8_t SDA_CTRL = 5;

volatile bool OverflowFlags[3] = {0}; //Set by OverflowISR, cleared in main loop
volatile bool EventFlags[3] = {0}; //Set by EventISR, cleared in main loop

volatile bool NewOverflow = false; //Used to indicate a new overflow condition
volatile bool NewEvent = false; //Used to indicate a new event condition

char ReadArray[25] = {0};

MCP23018 ioAlpha(0x23); //FIX! RETURN to 0x20
MCP23018 ioBeta(0x20); //FIX! RETURN to 0x23
MCP23018 ioGamma(0x24);


void setup() {

	// pinMode(EXT_EN, OUTPUT); 
	// digitalWrite(EXT_EN, HIGH);
	Serial.begin(115200);
	// delay(5000);
	// pinMode(SDA_CTRL, OUTPUT);
	// digitalWrite(SDA_CTRL, LOW);
	// delay(5000);
	ioAlpha.begin();
	ioBeta.begin();
	ioGamma.begin();
	// Wire.begin();
 //DEBUG1!
	SetDefaultPins(); //Set the default configuration for sensing 
	attachInterrupt(digitalPinToInterrupt(GPIOA), EventISR, FALLING); //Setup interrupt for GPIOA
	attachInterrupt(digitalPinToInterrupt(GPIOB), OverflowISR, FALLING); //Setup interrupt for GPIOA
	delay(1000);

	Serial.println("Begin Aux Demo...");
}

void loop() {
	while(1) {
		if(NewOverflow) {
			Serial.print("OVERFLOW:\t");
			for(int i = 0; i < 3; i++) { //Find more elegant solution??
				Serial.print(OverflowFlags[i]);
				Serial.print("\t");
				OverflowFlags[i] = 0; //clear flag once printed out
			}
			Serial.print("\n");
			NewOverflow = false; //Clear indicator flag once done printing
		}

		#if !defined(OVERFLOW_ONLY) //Don't print out all the pulses if we only want to look at overflow events
			if(NewEvent) {
				Serial.print("EVENT:\t");
				for(int i = 0; i < 3; i++) { //Find more elegant solution??
					Serial.print(EventFlags[i]);
					Serial.print("\t");
					EventFlags[i] = 0; //clear flag once printed out
				}
				Serial.print("\n");
				NewEvent = false; //Clear indicator flag once done printing
			}
		#endif
	}
}

void SetDefaultPins()
{
	for(int i = 0; i < 16; i++) { //Set all Gamma pins to input
		ioGamma.pinMode(i, INPUT);
	} 

	for(int i = 0; i < 3; i++) { //Set each group of IO pins seperately
		ioBeta.pinMode(OUT1 + i, INPUT); //Set all outputs from counters as inputs to I/O expander
		ioBeta.pinMode(COUNT_EN1 + i, OUTPUT); //Set enable control as output
		ioBeta.pinMode(OVF1 + i, INPUT); //Set overflow pins as inputs
		ioBeta.digitalWrite(COUNT_EN1 + 1, HIGH); //Default to enable all counters
		ioBeta.setIntPinConfig(OVF1 + i, false, 1); //Set overflow pins to trigger an interrupt whenever they are low
		ioBeta.setIntPinConfig(OUT1 + i, true); //Set output pins to trigger an interrupt anytime they change from the previous value
		ioBeta.setInterrupt(OVF1 + i, true); //Turn on interrupt for overflow pins
		ioBeta.setInterrupt(OUT1 + i, true); //Turn on interrupt for output pins
	}
	ioBeta.pinMode(LOAD, OUTPUT);
	ioBeta.pinMode(RST, OUTPUT);
	ioBeta.digitalWrite(LOAD, LOW); //Start with load in low position, will load on high going edge
	ioBeta.digitalWrite(RST, HIGH); //Negate reset by default

	ioAlpha.pinMode(REG_EN, OUTPUT);
	ioAlpha.digitalWrite(REG_EN, HIGH); //Default 5V to on
	ioAlpha.pinMode(ADC_INT, INPUT_PULLUP); //Setup ADC int as an input pullup for the open drain
	ioBeta.clearInterrupt(BOTH); //Make sure there are no pending interrupts 
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
			ioBeta.digitalWrite(COUNT_EN1 + i, HIGH); //Turn on outputs of counter n 
			Count[i] = ioGamma.readBus(); //Grab bus
			ioBeta.digitalWrite(COUNT_EN1 + i, LOW); //Turn off outputs of countern
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

void EventISR()
{
	NewEvent = true; //Set indicator flag
	for(int i = 0; i < 3; i++) {
		EventFlags[i] = ioBeta.getInterrupt(OUT1 + i); //Grab each interrupt flag
	}
	// EventFlags[0] = ioBeta.getInterrupt(OUT1); //DEBUG!
	ioBeta.clearInterrupt(A); //Clear interrupt A
}

void OverflowISR()
{
	NewOverflow = true; //Set indicator flag
	for(int i = 0; i < 3; i++) {
		OverflowFlags[i] = ioBeta.getInterrupt(OVF1 + i); //Grab each interrupt flag
	}
	ioBeta.clearInterrupt(B); //Clear interrupt B
}


