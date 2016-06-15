/**
 * Receiver for the door sensor. 
 * 
 * Author: Silvan Mühlemann, based on Basic Beacon by Daniel Berenguer
 */
#define SERIAL 1
 
#ifdef SERIAL
#include "HardwareSerial.h"
#endif

#define RFCHANNEL        0       // Let's use channel 0
#define SYNCWORD1        0xB5    // Synchronization word, high byte
#define SYNCWORD0        0x47    // Synchronization word, low byte
#define SOURCE_ADDR      4       // Sender address
#define DESTINATION_ADDR 5       // Receiver address

#define PHOTORESISTOR A4
#define GREENLED      18
#define REDLED        19

#define YELLOWLED     15
#define BUTTON        16

#define LOCKCLOSED    0
#define LOCKOPEN      1
#define LOCKUNKNOWN   2


unsigned long lastPacketReceived = 0;  
long buttonPressTimer = 0;  
int currentState = LOCKUNKNOWN;
CCPACKET txPacket;  // packet object
byte count = 0;
bool isNight = false;
int packetReceiveToggler = LOW;
int photoResistorValue = 0;

bool greenLedState = false;
bool redLedState = false;

/**
   This function is called whenever a wireless packet is received
*/
void rfPacketReceived(CCPACKET *packet)
{

  packetReceiveToggler = !packetReceiveToggler;

  if (packet->length >= 0)
  {
    currentState = packet->data[0];
  }

  // Send a confirmation
  //sendpacket.length = 1;
  //sendpacket.data[0] = 1;
  //panstamp.radio.sendData(sendpacket);

  lastPacketReceived = millis();

  displayCurrentState();
}


void fadeIn(int led) {
  if (led == GREENLED) {
    if (greenLedState) {
      return;
    }
    greenLedState = true;
  }
  if (led == REDLED) {
    if (redLedState) {
      return;
    }
    redLedState = true;
  }

  for (int fadeValue = 0 ; fadeValue <= 255; fadeValue += 5) {
    analogWrite(led, fadeValue);
    delay(30);
  }
}

void fadeOut(int led) {
  if (led == GREENLED) {
    if (!greenLedState) {
      return;
    }
    greenLedState = false;
  }
  if (led == REDLED) {
    if (!redLedState) {
      return;
    }
    redLedState = false;
  }
  for (int fadeValue = 255 ; fadeValue >= 0; fadeValue -= 5) {
    analogWrite(led, fadeValue);
    delay(30);
  }
}

void setup()
{
  // Setup LED output pin
  pinMode(LED, OUTPUT);
  digitalWrite(LED, LOW);

  pinMode(PHOTORESISTOR, INPUT);

  pinMode(GREENLED, OUTPUT);
  digitalWrite(GREENLED, LOW);
  pinMode(REDLED, OUTPUT);
  digitalWrite(REDLED, LOW);
  pinMode(YELLOWLED, OUTPUT);
  digitalWrite(YELLOWLED, LOW);

  pinMode(BUTTON, INPUT);

  lastPacketReceived = millis();

#ifdef SERIAL
  Serial.begin(9600);
#endif

  panstamp.radio.setChannel(RFCHANNEL);
  panstamp.radio.setSyncWord(SYNCWORD1, SYNCWORD0);
  panstamp.radio.setDevAddress(SOURCE_ADDR);
  panstamp.radio.setCCregs();

  // Let's disable address check for the current project so that our device
  // will receive packets even not addressed to it.
  panstamp.radio.disableAddressCheck();

  // Declare RF callback function
  panstamp.setPacketRxCallback(rfPacketReceived);

  buttonPressTimer = millis();

}

void displayCurrentState()
{

  if ((millis() > (buttonPressTimer + 20000)) && isNight) {
    fadeOut(GREENLED);
    fadeOut(REDLED);
    digitalWrite(LED, LOW);
  } else {
    digitalWrite(LED, packetReceiveToggler);

#ifdef SERIAL
    Serial.println("displayCurrentState");
    Serial.println(currentState);
#endif

    if (currentState == LOCKCLOSED) {
      fadeOut(GREENLED);
      fadeIn(REDLED);
    } else if (currentState == LOCKOPEN) {
      fadeIn(GREENLED);
      fadeOut(REDLED);
    } else {
      fadeIn(GREENLED);
      fadeIn(REDLED);
    }
  }
}

void loop()
{
  digitalWrite(YELLOWLED, LOW);

  //txPacket.length = 2;  // Let's send a single data byte plus the destination address

  //txPacket.data[0] = DESTINATION_ADDR;   // First data byte has to be the destination address
  //txPacket.data[1] = count++;            // Self-incrementing value
  //panstamp.radio.sendData(txPacket);     // Transmit packet

  if (digitalRead(BUTTON) == HIGH) {
    buttonPressTimer = millis();
    displayCurrentState();
#ifdef SERIAL
    Serial.println("Button pressed");
#endif
  }

  
  if ((millis() - lastPacketReceived) > 500000) {
    currentState = LOCKUNKNOWN;
#ifdef SERIAL
    Serial.println("No data");
#endif
  }

  photoResistorValue = analogRead(PHOTORESISTOR);
  // override
  //photoResistorValue = 300;
  if (photoResistorValue != 65535) {

    if (!isNight && (photoResistorValue < 5)) {
      buttonPressTimer = millis();
      isNight = true;
      digitalWrite(YELLOWLED, HIGH);
#ifdef SERIAL
      Serial.println("Nightmode");
#endif
    }

    if (isNight && (photoResistorValue > 10)) {
      isNight = false;
      digitalWrite(YELLOWLED, HIGH);
#ifdef SERIAL
      Serial.println("Daymode");
#endif
    }

#ifdef SERIAL
    Serial.println(photoResistorValue);
    Serial.println(millis() - lastPacketReceived);
#endif

  }
  displayCurrentState();
  delay(1000);
}
