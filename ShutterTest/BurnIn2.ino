/*
Program to simulate function generator for test XPM laser shutter control box
*/
#include <Ethernet.h>
//#include "HTTPSRedirect.h"


byte mac[] = { 0xA8, 0x61, 0x0A, 0xAE, 0x71, 0x1F };
IPAddress ip(10, 0, 0, 4); // example IPAddress ip(192, 168, 1, 100);
IPAddress gateway(10, 0, 0, 1);
IPAddress dns(10, 0, 0, 1);
IPAddress subnet(255, 255, 255, 0);
EthernetClient client;
EthernetServer ser = EthernetServer(5047);
String msg;

void setup() {
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  Serial.print("Hello\n");
  // start the Ethernet connection:
  Ethernet.begin(mac, ip);
    //Serial.println("Failed to configure Ethernet using DHCP");
    // try to congifure using IP address instead of DHCP:
    //Ethernet.begin(mac, ip);

  // give the Ethernet shield a second to initialize:
  delay(1000);
}

void receive()
{
    msg = String(""); 
    char c = '$';
    while( c != '\n' )
    {
      c = client.read();
      if( c<0 ) break;
      if( c>=0 ) msg = msg + String(c);
    }
    Serial.println(msg);
}


void wait_and_update(int sec) {
    for(int i = 0; i < sec; i++) {
      delay(1000);  // wait for 1 s
      float val = analogRead(A4); // read data
      String string_volt = String(val, DEC);
      receive();
      //Serial.println(msg.c_str());
      if( msg.startsWith( String("CWON") ) ) digitalWrite(13, HIGH);
      else if( msg.startsWith( String("CWOFF") ) ) digitalWrite(13, LOW);
      else
      {
        client.print(string_volt+String("\n")); // print data
        Serial.println (String("Sent ") + string_volt+String("\n"));
      }
   }
}

void pulse_it(int duration, float DC) {  //duration value in ms, DC percentage

  int cycles = duration;
  float t_on = 10*DC/100; //delay time when energized
  float t_off = 10 - t_on;  //delay time when de-energized

  for(int i = 0; i < cycles || cycles < 0; i++) {
    digitalWrite(13, LOW);  //de-energize solenoid
    delay(t_off); // wait for t_off ms
    digitalWrite(13, HIGH); //energize solenoid
    delay(t_on);  // wait for t_on ms
    if(i % 100 == 0) {
      float val = analogRead(A4); // read data
      String string_volt = String(val, DEC);
      receive();
      client.print(string_volt+String("\n")); // print data
      if( msg.startsWith( String("OFF") ) )
      {
        digitalWrite(13, LOW);  //de-energize solenoid
        break;
      }
    }
  }
}

void loop() {
  Serial.println("connecting");

  client = ser.available();
  while(!client) client = ser.available();
  Serial.println("connected!");
  if(client) 
  {
    // read bytes from the incoming client and write them back
    // to any clients connected to the server:
    float duty;
    /*
    receive();
    if( !msg.startsWith(String("CW")) && !msg.startsWith(String("OFF")) && !msg.startsWith(String("???")) )
    {
       duty = msg.toFloat();
       pulse_it( -1 , duty );
    }
    Serial.println(msg);
    */
    digitalWrite(13, LOW);  // de-energize solenoid
    wait_and_update(300);  // wait for 300 s
    digitalWrite(13, HIGH); // energize solenoid
    wait_and_update(32); // wait for 32 s
    digitalWrite(13, LOW);  // de-energize solenoid
    wait_and_update(14400);  // wait 4 hours (4*60*60) s
    digitalWrite(13, HIGH); // energize solenoid
    wait_and_update(258);  // wait for 258 s
  }
}
