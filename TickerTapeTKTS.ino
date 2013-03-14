/*
 * DNS and DHCP-based Web client
 * Circuit:
 * Ethernet shield attached to pins 10, 11, 12, 13
 */

#include <SPI.h>
#include <Ethernet.h>
#include <SoftwareSerial.h>
#include <Thermal.h>
#include <TextFinder.h>


// Enter a MAC address for your controller below.
// Newer Ethernet shields have a MAC address printed on a sticker on the shield
byte mac[] = {  0x90, 0xA2, 0xDA, 0x00, 0x9D, 0x51 };
char serverName[] = "iphone.tdf.org";

const int ledGreen = 7;
const int ledRed = 6;
const int ledYellow = 8;
const int button = 5;
#define ON LOW
#define OFF HIGH

int printer_RX_Pin = 14;
int printer_TX_Pin = 15;

Thermal printer(printer_RX_Pin, printer_TX_Pin);
EthernetClient client;

void setup() {
  pinMode(ledGreen, OUTPUT);
  pinMode(ledRed, OUTPUT);
  pinMode(ledYellow, OUTPUT);
  pinMode(button, INPUT);
  digitalWrite(button, HIGH); //turn on internal pull-up resister  
  
  Serial.begin(9600);
  // start the Ethernet connection:
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    digitalWrite(ledRed, HIGH);
    // no point in carrying on, so do nothing forevermore:
    while(true);
  }
  // give the Ethernet shield a second to initialize:
  delay(1000);
  digitalWrite(ledYellow, HIGH);
  
  printer.setSize('S');
  printer.justify('L');
}

void connectToServer();

#define MAX_TITLE 26
#define RIGHT_ALIGN 29
int length;
char s[MAX_TITLE], t[MAX_TITLE];
int percentage;
TextFinder finder(client);

void loop()
{
  if(digitalRead(button) == ON && !client.connected())
  {
    delay(500);
    Serial.println("connecting...");
    connectToServer();
  }
  if (client.available()) {
    digitalWrite(ledRed, LOW);
    digitalWrite(ledYellow, LOW);
    digitalWrite(ledGreen, HIGH);
    
    //finder doesn't use a buffer so must create a test first
    //that doesn't pass here so we don't search file for PostDateTime
    //because it only exists once = !strlen(t)
    if(!strlen(t) && finder.find("PostDateTime\":")){
        length = finder.getString("\"", "\"", t, MAX_TITLE);
        //Serial.println("****TKTS BOOTH TICKER TAPE****");
        //Serial.println(t);
        
        printer.justify('C');
        printer.println("****TKTS BOOTH TICKER TAPE****");
        printer.println(t);
        printer.justify('L');
    }

    while(finder.findUntil("Name\":", "OffBroadwayBoard"))
    {
        length = finder.getString("\"", "\"", s, MAX_TITLE);
        //Serial.print(s);
        //Serial.print(": ");
        printer.print(s);
      
      if(finder.find("Percentage\":")) {
          percentage = finder.getValue();
          for(int i = 0; i < RIGHT_ALIGN-length; i++) {
            //Serial.print(" ");
            printer.print(" ");
          }
          //Serial.print(percentage);
          //Serial.print("%");
          //Serial.print("\n");
          
          printer.print(percentage);
          printer.print("%");
          printer.print("\n");
      }
    }
    client.stop();
  }
  if(!client.connected())
  {
    Serial.println("disconnecting.");
    printer.feed();
    printer.feed();
    printer.feed();
    t[0] = 0; //revert so that the title prints
    
    client.stop();
    digitalWrite(ledGreen, LOW);
    digitalWrite(ledYellow, HIGH);
    while(digitalRead(button) == OFF);
  }
}

    
void connectToServer()
{
    if (client.connect(serverName, 91)) {
    Serial.println("connected");
    // Make a HTTP request:
    client.println("GET /Services/TKTSShowList.aspx?location=TiSq");
    client.println();
  } 
  else {
    // kf you didn't get a connection to the server:
    Serial.println("connection failed");
    digitalWrite(ledRed, HIGH);
    digitalWrite(ledYellow, LOW);
  }
}
