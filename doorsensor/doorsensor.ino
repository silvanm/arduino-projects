/**
 * Sender for the door lock sensor
 * 
 * Author: Silvan Mühlemann, based on Basic Beacon by Daniel Berenguer
 */
#ifdef SERIAL
#include "HardwareSerial.h"
#endif

#define RFCHANNEL        0       // Let's use channel 0
#define SYNCWORD1        0xB5    // Synchronization word, high byte
#define SYNCWORD0        0x47    // Synchronization word, low byte
#define SOURCE_ADDR      5       // Device address

#define GREENLED      A0
#define REDLED        A1
#define YELLOWLED     A2
#define SENDLED       A3
#define LOCKCONTACT   3

#define LOCKCLOSED    LOW

int packetReceived = 0;
int currentState;
long passCounter = 0;

CCPACKET packet;

void myIsr()
{
  panstamp.wakeUp();
  // This function is called whenever digital pin changes
  //lastState = currentState;
  #ifdef SERIAL
  Serial.println("ISR");
  #endif

  packetReceived = 0;
  passCounter = 0;
}

/**
   This function is called whenever a wireless packet is received
*/
void rfPacketReceived(CCPACKET *packet)
{
  // The LED will toggle when a new packet is received
  digitalWrite(LED, !digitalRead(LED));

  #ifdef SERIAL
  Serial.println("Packet received");
  #endif
  
  if (packet->length >= 0)
  {
    currentState = packet->data[0];
  }

  packetReceived = 1;
}

void setup()
{
  byte i;

  // Setup LED output pin
  pinMode(SENDLED, OUTPUT);
  digitalWrite(SENDLED, LOW);
  pinMode(GREENLED, OUTPUT);
  digitalWrite(GREENLED, LOW);
  pinMode(REDLED, OUTPUT);
  digitalWrite(REDLED, LOW);
  pinMode(YELLOWLED, OUTPUT);
  digitalWrite(YELLOWLED, LOW);

  pinMode(LOCKCONTACT, INPUT);

  panstamp.radio.setChannel(RFCHANNEL);
  panstamp.radio.setSyncWord(SYNCWORD1, SYNCWORD0);
  panstamp.radio.setDevAddress(SOURCE_ADDR);
  panstamp.radio.setCCregs();
  panstamp.setHighTxPower();

  // Declare RF callback function
  panstamp.radio.disableAddressCheck();
  panstamp.setPacketRxCallback(rfPacketReceived);

  #ifdef SERIAL
  Serial.begin(9600);
  #endif

  packet.length = 10;

  for(i=0 ; i<packet.length ; i++)
    packet.data[i] = i;

  attachInterrupt(LOCKCONTACT, myIsr, CHANGE);

}


void loop()
{

  passCounter++;
  currentState = digitalRead(LOCKCONTACT);

  #ifdef SERIAL
  Serial.print("Currentstate: ");
  Serial.println(currentState);
  #endif

  digitalWrite(YELLOWLED, HIGH);
 

  // Light the LED for only a limited time to save power
  if (passCounter < 5) {
    if (currentState == LOCKCLOSED) {
      digitalWrite(GREENLED, LOW);
      digitalWrite(REDLED, HIGH);
    } else {
      digitalWrite(GREENLED, HIGH);
      digitalWrite(REDLED, LOW);
    }
  } else {
    digitalWrite(GREENLED, LOW);
    digitalWrite(REDLED, LOW);
  }
//    Serial.println("Packet received? ");
  //  Serial.println(packetReceived);
    // only go to sleep if our packet has been confirmed
   // if (packetReceived == 1) {
    //  panstamp.sleep();  
    //}
  

  // Packets are sent in each loop
  digitalWrite(SENDLED, HIGH);
  packet.data[0] = currentState;
  panstamp.radio.sendData(packet);
  digitalWrite(SENDLED, LOW);

  if (passCounter < 5) {
    panstamp.sleepSec(2);
  } else {
    digitalWrite(YELLOWLED, LOW);
    panstamp.sleepSec(120);
  }
}
