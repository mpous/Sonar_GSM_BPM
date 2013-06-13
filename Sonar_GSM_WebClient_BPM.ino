/*
 *  Sonar 2013 - OneSeatAway
 *
 *  GSM WebClient polling data from the BPM database
 * 
 *  Marc Pous
 *  marc.pous@gmail.com
 *  
 *  http://marcpous.com
 *  http://thethings.io
 *
 *  
 */

//------------------------------------------------------------------------
//  Includes
//------------------------------------------------------------------------
#include "Timer.h"

// libraries
#include <GSM.h>
#include <TextFinder.h>


#define NUM_MOTORS 4
// PIN Number
#define PINNUMBER ""

// APN data
#define GPRS_APN       "bluevia.movistar.es" // replace your GPRS APN
#define GPRS_LOGIN     ""    // replace with your GPRS login
#define GPRS_PASSWORD  "" // replace with your GPRS password

// initialize the library instance
GSMClient client;
GPRS gprs;
GSM gsmAccess; 

// URL, path & port (for example: arduino.cc)
char server[] = "thethings.io";

// paths to the BPM API
char *path[4] = { "/getBPM/TWITTER", "/getBPM/BUS", "/getBPM/BICING", "/getBPM/TRAFFIC" };

int port = 80; // port 80 is the default for HTTP

// array with results
byte data[500];
int i = 1;

// connection state
boolean notConnected = true;

//------------------------------------------------------------------------
//  Properties
//------------------------------------------------------------------------
// the output pins for the motors
int VMOTORS[ NUM_MOTORS ] = { 4, 5, 6, 9 };

// this is a holder for the volume
int VOLUME = 0;
int VOLUME_PIN = A0;

// the timer (should match the number of motors)
Timer timer[ NUM_MOTORS ];

//------------------------------------------------------------------------
//   Max Packet Handling
//------------------------------------------------------------------------
// this is a holder for a component of
// the string from max ie( "_1|100" )
// the array length should match the number of motors
String COMP[ NUM_MOTORS ];
// the component index to be read
int COMP_INDEX[ NUM_MOTORS ] = { 1,1,1,1 };

// the size of the packet (i.e. the number of different rhythms)
int COMP_SIZE[ NUM_MOTORS ] = { 1,1,1,1 };

// this is a holder for the motor strength
// the array length should match the number of motors
int STRENGTH[ NUM_MOTORS ] = { 255,255,255,255 };

// this is a holder for time interval
// the array length should match the number of motors
float INTERVAL[ NUM_MOTORS ] = { 500,500,500,500 };


void setup()
{
  // initialize serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }

  Serial.println("Starting Arduino web client.");
  
  connectTo(server, port, path[0]);
}


void connectTo(char *server, int port, char *path)
{
  // After starting the modem with GSM.begin()
  // attach the shield to the GPRS network with the APN, login and password
  while(notConnected)
  {
    if((gsmAccess.begin(PINNUMBER)==GSM_READY) &
        (gprs.attachGPRS(GPRS_APN, GPRS_LOGIN, GPRS_PASSWORD)==GPRS_READY))
      notConnected = false;
    else
    {
      Serial.println("Not connected");
      delay(1000);
    }
  }

  Serial.println("connecting...");
  
  // if you get a connection, report back via serial:
  if (client.connect(server, port))
  {
    //Serial.println(millis());
    Serial.print("connected to ");
    Serial.print(server);
    Serial.println(path);
    // Make a HTTP request:
    client.print("GET ");
    client.print(path);
    client.println(" HTTP/1.0");
    client.println();
  } 
  else
  {
    // if you didn't get a connection to the server:
    Serial.println("connection failed");
  }

}


void loop()
{
  //Serial.println("-------------- LOOP --------------------");

  // if there are incoming bytes available 
  // from the server, read them and print them:
  if (client.available())
  {   
    TextFinder finder(client);
   
    char bpm[300];
    finder.getString("#", "#", bpm, 300);
    
    Serial.println(bpm);
    
    int ibpm = atoi(bpm);
    float y = (ibpm / 60.0);
    float x = 1000.0 / y;

    moveYourAss(bpm, i, x);
    
    Serial.print("MILLIS = "); 
    Serial.println(millis());
    Serial.println("----------------------------------------");
    client.stop();
    client.flush();
    delay(5000);

    connectTo(server, port, path[i]);
    i++;
    if (i > 3) i = 0;

  }
  
  // if the server's disconnected, stop the client:
  if (!client.available() && !client.connected())
  {
    Serial.println();
    Serial.println("disconnecting.");
    client.stop();

    // do nothing forevermore:
    for(;;)
      ;
  }
}


void moveYourAss(char *bpm, int index, float interval)
{
  
  /*
   *  Output signal
   */
  Serial.println("Output");
  
  /*
   *  read volume value
   */
  VOLUME = analogRead( VOLUME_PIN );
  float volumePct = (float)(map(VOLUME, 0,1023, 40,100)*0.01);

  // update motor strength
  int strengthStr = atoi (bpm);
  Serial.println(strengthStr);

  STRENGTH[index] = (int)(map(strengthStr, 0, 255, 0, 255));
  Serial.println("NEW STRENGTH");
  Serial.println(STRENGTH[index]);
  
   /*
    *  this configuration will turn off
    *  the motor for exactly 1 "frame"
    *  and keep the motor ON for the
    *  length set in the interval
    */
    analogWrite( VMOTORS[index], STRENGTH[index] );
   
    INTERVAL[index] = (int)interval;

    // set Timer with new interval
    timer[index].stop();
    timer[index].setFreq( INTERVAL[index] );

    Serial.print("INTERVAL = ");
    Serial.println(INTERVAL[index]);
}


