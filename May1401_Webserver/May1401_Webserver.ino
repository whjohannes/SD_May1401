/*--------------------------------------------------------------

Webserver for ISU Senior Design - Ames High Greenhouse System

This code will be built using the Arduino compiler and operates
with an Ethernet shield using the WizNet chipset.
--------------------------------------------------------------*/

#include <SPI.h>
#include <Ethernet.h>
#include <SD.h>
#include "Globals.h"
#include <config_parser.ino>
#include <EthernetUdp.h>

// size of buffer used to capture HTTP requests
#define REQ_BUF_SZ   60
#define NUM_ZONES 16
#define NUM_PROPS 9
#define BUFFER_SIZE 64

// MAC address from Ethernet shield sticker under board
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192, 168, 8, 177); // IP address, may need to change depending on network
EthernetServer server(80);  // create a server at port 80

unsigned int localPort = 8888;      // local port to listen for UDP packets
IPAddress timeServer(132, 163, 4, 101); // time-a.timefreq.bldrdoc.gov NTP server
const int NTP_PACKET_SIZE= 48; // NTP time stamp is in the first 48 bytes of the message
byte packetBuffer[ NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets 
// A UDP instance to let us send and receive packets over UDP
EthernetUDP Udp;

File webFile;               // the web page file on the SD card
char HTTP_req[REQ_BUF_SZ] = {0}; // buffered HTTP request stored as null terminated string
int req_index = 0;              // index into HTTP_req buffer
boolean LED_state[4] = {0}; // stores the states of the LEDs
char ZoneState[NUM_ZONES] = {0}; //stores zone active states
String config_array[NUM_ZONES][NUM_PROPS]; //stores values from website
String parsed_GET[10];
int count = 0; //for buffering packet
byte httpBuffer[BUFFER_SIZE];
File return_file;
String Http_req_full = "";

int hours,minutes;
int time_check =0;
char after_noon = 'A';
String time = "";



// typedef struct 
// {
// String Name; 
// int Visible; 
// String Time1; 
// int  duration1;
// String Time2; 
// int  duration2;
// String Time3; 
// int  duration3;
// String Time4; 
// int  duration4;
// } zone_properties;


//Define all zone scructs
zone_properties zone[NUM_ZONES];

void setup()
{
    // disable Ethernet chip
    pinMode(10, OUTPUT);
    digitalWrite(10, HIGH);
    
    Serial.begin(19200);       // for debugging
    
    // initialize SD card
    Serial.println("Initializing SD card...");
    if (!SD.begin(4)) {
        Serial.println("ERROR - SD card initialization failed!");
        return;    // init failed
    }
    Serial.println("SUCCESS - SD card initialized.");
    // check for index.htm file
    if (!SD.exists("index.htm")) {
        Serial.println("ERROR - Can't find index.htm file!");
        return;  // can't find index file
    }

    if (!SD.exists("config.h")) {
        Serial.println("ERROR - Can't find config.h - resetting to default settings");
        return;
    }

    Serial.println("SUCCESS - Found index.htm file.");
    Serial.println("SUCCESS - Found config.h Parsing now...");
    split_config(config_array);
  
    for(int i=0; i<NUM_ZONES;i++)
    {
        zone[i].Name = config_array[i][0];
        zone[i].Visible = config_array[i][1].toInt();
        zone[i].Time1 = config_array[i][2];
        zone[i].duration1 = config_array[i][3].toInt();
        zone[i].Time2 = config_array[i][4];
        zone[i].duration2 = config_array[i][5].toInt();
        zone[i].Time3 = config_array[i][6];
        zone[i].duration3 = config_array[i][7].toInt();
        zone[i].Time4 = config_array[i][8];
        zone[i].duration4 = config_array[i][9].toInt();

    }

    Serial.println("Parse -> Struct: Complete.");
    for(int i =0; i<NUM_ZONES; i++)
        Serial.println(zone[i].Name);
    Ethernet.begin(mac, ip);  // initialize Ethernet device
    server.begin();           // start to listen for clients
    Udp.begin(localPort);

}

void loop()
{
    EthernetClient client = server.available();  // try to get client

    if (client) {  // got client?
        boolean currentLineIsBlank = true;
        while (client.connected()) {
            if (client.available()) {   // client data available to read
                char c = client.read(); // read 1 byte (character) from client
                // limit the size of the stored received HTTP request
                // buffer first part of HTTP request in HTTP_req array (string)
                // leave last element in array as 0 to null terminate string (REQ_BUF_SZ - 1)
                if (req_index < (REQ_BUF_SZ - 1)) {
                    Http_req_full += c;
                    HTTP_req[req_index] = c;          // save HTTP request character
                    req_index++;

                }

                if (c == '\n' && currentLineIsBlank) {
                    // send a standard http response header
                    client.println("HTTP/1.1 200 OK");
                    // remainder of header follows below, depending on if
                    // web page or XML page is requested
                    // Ajax request - send XML file
                    char * pch;
                    pch = strtok (HTTP_req,"&,");
                    int elemountcount=0;
                    while (pch != NULL)
                    {
                       // Serial.println(pch);
                        pch = strtok (NULL, "&,");
                        parsed_GET[elemountcount] = pch;
                        elemountcount++;
                    }

                    if (StrContains(HTTP_req, "ajax_inputs")) {
                        // send rest of HTTP header
                        client.println("Content-Type: text/xml");
                        client.println("Connection: keep-alive");
                        client.println();
                        Zone_States();
                        // send XML file containing input states
                        XML_response(client);
                    }
                    else {  // web page request
                        // send rest of HTTP header
                        client.println("Content-Type: text/html");
                        client.println("Connection: keep-alive");
                        client.println();
                        // send web page
                        webFile = SD.open("index.htm");        // open web page file
                        if (webFile) {
                            while(webFile.available()) {
                                for(int i = 0; i<BUFFER_SIZE; i++)
                                httpBuffer[i] = 0;

                                for(count = 0; count <BUFFER_SIZE; count++)
                                {
                                    httpBuffer[count] = webFile.read();
                                }
                                client.write( (byte*) httpBuffer, BUFFER_SIZE); // send web page to client
                            
                            }
                            webFile.close();
                        }
                    }
                    // display received HTTP request on serial port
                   // Serial.println("reg string");

                    Serial.println(HTTP_req);
                    Http_req_full = "";
                    // reset buffer index and all buffer elements to 0
                    req_index = 0;
                    StrClear(HTTP_req, REQ_BUF_SZ);
                    break;
                }
                // every line of text received from the client ends with \r\n
                if (c == '\n') {
                    // last character on line of received text
                    // starting new line with next character read
                    currentLineIsBlank = true;
                } 
                else if (c != '\r') {
                    // a text character was received from client
                    currentLineIsBlank = false;
                }
            } // end if (client.available())
        } // end while (client.connected())
        delay(1);      // give the web browser time to receive the data
        client.stop(); // close the connection
    } // end if (client)
    time_check++;
    //TURN ON ZONES?
    if(time_check >600)
    {
        updateTime();
        water_time();
        time_check = 0;
    }
}

void Zone_States(void)
{
    if(parsed_GET[0].equals("config"))
    {
        Serial.println("FUCK YES");
    }

    if(parsed_GET[0].equals("setup"))
    {
        Serial.println("FUCK YES");
    }

    if(parsed_GET[0].equals("button"))
    {
        Serial.println("FUCK YES");
    }
    ///////////////////////////////
    if (StrContains(HTTP_req, "Zone1=on")) 
    {
        ZoneState[0] = 1;  // save zone1 state
        digitalWrite(6, HIGH); //TODO: define pin 
    }
    else if (StrContains(HTTP_req, "Zone1=off")) 
    {
        ZoneState[0] = 0;  // save zone1 state
        digitalWrite(6, LOW); //TODO: define pin 
    }
    else if (StrContains(HTTP_req, "Zone1=auto")) 
    {
        ZoneState[0] = 0;  // save zone1 state
        digitalWrite(6, LOW); //TODO: define pin 
    }


    if (StrContains(HTTP_req, "Zone2=1")) 
    {
        ZoneState[1] = 1;  // save zone2 state
        digitalWrite(6, HIGH); //TODO: define pin 
    }
    else if (StrContains(HTTP_req, "Zone2=0")) 
    {
        ZoneState[1] = 0;  // save zone2 state
        digitalWrite(6, LOW); //TODO: define pin 
    }
    
    if (StrContains(HTTP_req, "Zone3=1")) 
    {
        ZoneState[2] = 1;  // save zone3 state
        digitalWrite(6, HIGH); //TODO: define pin 
    }
    else if (StrContains(HTTP_req, "Zone3=0")) 
    {
        ZoneState[2] = 0;  // save zone3 state
        digitalWrite(6, LOW); //TODO: define pin 
    }

    if (StrContains(HTTP_req, "Zone4=1")) 
    {
        ZoneState[3] = 1;  // save zone4 state
        digitalWrite(6, HIGH); //TODO: define pin 
    }
    else if (StrContains(HTTP_req, "Zone4=0")) 
    {
        ZoneState[3] = 0;  // save zone4 state
        digitalWrite(6, LOW); //TODO: define pin 
    }
    
    if (StrContains(HTTP_req, "Zone5=1")) 
    {
        ZoneState[4] = 1;  // save zone state
        digitalWrite(6, HIGH); //TODO: define pin 
    }
    else if (StrContains(HTTP_req, "Zone5=0")) 
    {
        ZoneState[4] = 0;  // save zone state
        digitalWrite(6, LOW); //TODO: define pin 
    }

    if (StrContains(HTTP_req, "Zone6=1")) 
    {
        ZoneState[5] = 1;  // save zone state
        digitalWrite(6, HIGH); //TODO: define pin 
    }
    else if (StrContains(HTTP_req, "Zone6=0")) 
    {
        ZoneState[5] = 0;  // save zone state
        digitalWrite(6, LOW); //TODO: define pin 
    }

    if (StrContains(HTTP_req, "Zone7=1")) 
    {
        ZoneState[6] = 1;  // save zone state
        digitalWrite(6, HIGH); //TODO: define pin 
    }
    else if (StrContains(HTTP_req, "Zone7=0")) 
    {
        ZoneState[6] = 0;  // save zone state
        digitalWrite(6, LOW); //TODO: define pin 
    }

    if (StrContains(HTTP_req, "Zone8=1")) 
    {
        ZoneState[7] = 1;  // save zone state
        digitalWrite(6, HIGH); //TODO: define pin 
    }
    else if (StrContains(HTTP_req, "Zone8=0")) 
    {
        ZoneState[7] = 0;  // save zone state
        digitalWrite(6, LOW); //TODO: define pin 
    }

    if (StrContains(HTTP_req, "Zone9=1")) 
    {
        ZoneState[8] = 1;  // save zone state
        digitalWrite(6, HIGH); //TODO: define pin 
    }
    else if (StrContains(HTTP_req, "Zone9=0")) 
    {
        ZoneState[8] = 0;  // save zone state
        digitalWrite(6, LOW); //TODO: define pin 
    }

    if (StrContains(HTTP_req, "Zone10=1")) 
    {
        ZoneState[9] = 1;  // save zone state
        digitalWrite(6, HIGH); //TODO: define pin 
    }
    else if (StrContains(HTTP_req, "Zone10=0")) 
    {
        ZoneState[9] = 0;  // save zone state
        digitalWrite(6, LOW); //TODO: define pin 
    }

    if (StrContains(HTTP_req, "Zone11=1")) 
    {
        ZoneState[10] = 1;  // save zone state
        digitalWrite(6, HIGH); //TODO: define pin 
    }
    else if (StrContains(HTTP_req, "Zone11=0")) 
    {
        ZoneState[10] = 0;  // save zone state
        digitalWrite(6, LOW); //TODO: define pin 
    }

    if (StrContains(HTTP_req, "Zone12=1")) 
    {
        ZoneState[11] = 1;  // save zone state
        digitalWrite(6, HIGH); //TODO: define pin 
    }
    else if (StrContains(HTTP_req, "Zone12=0")) 
    {
        ZoneState[11] = 0;  // save zone state
        digitalWrite(6, LOW); //TODO: define pin 
    }

    if (StrContains(HTTP_req, "Zone13=1")) 
    {
        ZoneState[12] = 1;  // save zone state
        digitalWrite(6, HIGH); //TODO: define pin 
    }
    else if (StrContains(HTTP_req, "Zone13=0")) 
    {
        ZoneState[12] = 0;  // save zone state
        digitalWrite(6, LOW); //TODO: define pin 
    }

    if (StrContains(HTTP_req, "Zone14=1")) 
    {
        ZoneState[13] = 1;  // save zone state
        digitalWrite(6, HIGH); //TODO: define pin 
    }
    else if (StrContains(HTTP_req, "Zone14=0")) 
    {
        ZoneState[13] = 0;  // save zone state
        digitalWrite(6, LOW); //TODO: define pin 
    }

    if (StrContains(HTTP_req, "Zone15=1")) 
    {
        ZoneState[14] = 1;  // save zone state
        digitalWrite(6, HIGH); //TODO: define pin 
    }
    else if (StrContains(HTTP_req, "Zone15=0")) 
    {
        ZoneState[14] = 0;  // save zone state
        digitalWrite(6, LOW); //TODO: define pin 
    }

    if (StrContains(HTTP_req, "Zone16=1")) 
    {
        ZoneState[15] = 1;  // save zone state
        digitalWrite(6, HIGH); //TODO: define pin 
    }
    else if (StrContains(HTTP_req, "Zone16=0")) 
    {
        ZoneState[15] = 0;  // save zone state
        digitalWrite(6, LOW); //TODO: define pin 
    }

    if (StrContains(HTTP_req, "setup"))
    {
            //WRITE SD_CARD
        // while(SD.exists("config.h"))
        // {
        //     Serial.println("removing previous config");
        //     SD.remove("config.h");
        // }
        // return_file = SD.open("config.h", FILE_WRITE);        // open web page file
        //     if (return_file) 
        //     {
        //         for(int i = 0; i<NUM_ZONES; i++)
        //         {
        //             return_file.print( (String) zone[i].Name);
        //             return_file.print(",");
        //             return_file.print(zone[i].Visible);
        //             return_file.print(",");
        //             return_file.print(zone[i].Time1);
        //             return_file.print(",");
        //             return_file.print(zone[i].duration1);
        //             return_file.print(",");
        //             return_file.print(zone[i].Time2);
        //             return_file.print(",");
        //             return_file.print(zone[i].duration2);
        //             return_file.print(",");
        //             return_file.print(zone[i].Time3);
        //             return_file.print(",");
        //             return_file.print(zone[i].duration3);
        //             return_file.print(",");
        //             return_file.print(zone[i].Time4);
        //             return_file.print(",");
        //             return_file.print(zone[i].duration4);
        //             return_file.print(",");
        //             Serial.println(zone[i].Name);
        //         }
            
        //         Serial.println("Finished Writing to SD");
        //         return_file.close();
        //     }
        //     else
        //     {
        //         Serial.println("Couldn't write to config.h");
        //     }
        //END WRITE SD CARD
        }
}

void config_parse()
{
    Serial.println("enter config_parse function");
    temp_string = Http_req_full;    
    int comma_position = Http_req_full.indexOf(',');
    for( int i =0; i<NUM_ZONES; i++)
    {
           
    }

}

void setup_parse()
{

    Serial.println("enter setup_parse function");

    
}

// send the XML file
void XML_response(EthernetClient cl)
{

    cl.print("<?xml version = \"1.0\" ?>");
    cl.print("<zones>");
    
    for(int i = 0; i<NUM_ZONES; i++)
    {
        cl.print("<name>");
        cl.print(zone[i].Name);
        cl.print("</name>");

        cl.print("<visible>");
        cl.print(zone[i].Visible);
        cl.print("</visible>");

        cl.print("<time1>");
        cl.print(zone[i].Time1);
        cl.print("</time1>");
 
        cl.print("<duration1>");
        cl.print(zone[i].duration1);
        cl.print("</duration1>");

        cl.print("<time2>");
        cl.print(zone[i].Time2);
        cl.print("</time2>");

        cl.print("<duration2>");
        cl.print(zone[i].duration2);
        cl.print("</duration2>");

        cl.print("<time3>");
        cl.print(zone[i].Time3);
        cl.print("</time3>");

        cl.print("<duration3>");
        cl.print(zone[i].duration3);
        cl.print("</duration3>");

        cl.print("<time4>");
        cl.print(zone[i].Time4);
        cl.print("</time4>");

        cl.print("<duration4>");
        cl.print(zone[i].duration4);
        cl.print("</duration4>");


    }
    
    cl.print("</zones>");
}

// sets every element of str to 0 (clears array)
void StrClear(char *str, char length)
{
    for (int i = 0; i < length; i++) {
        str[i] = 0;
    }
}

// searches for the string sfind in the string str
// returns 1 if string found
// returns 0 if string not found
char StrContains(char *str, char *sfind)
{
    char found = 0;
    char index = 0;
    char len;

    len = strlen(str);
    
    if (strlen(sfind) > len) {
        return 0;
    }
    while (index < len) {
        if (str[index] == sfind[found]) {
            found++;
            if (strlen(sfind) == found) {
                return 1;
            }
        }
        else {
            found = 0;
        }
        index++;
    }

    return 0;
}


void updateTime()
 {
    sendNTPpacket(timeServer); // send an NTP packet to a time server

    // wait to see if a reply is available
  delay(1000);  
  if ( Udp.parsePacket() ) {  
    // We've received a packet, read the data from it
    Udp.read(packetBuffer,NTP_PACKET_SIZE);  // read the packet into the buffer

    //the timestamp starts at byte 40 of the received packet and is four bytes,
    // or two words, long. First, esxtract the two words:

    unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
    unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);  
    // combine the four bytes (two words) into a long integer
    // this is NTP time (seconds since Jan 1 1900):
    unsigned long secsSince1900 = highWord << 16 | lowWord;            

    // time offset from Central Time
    const unsigned long centralTime = -18000L;
    // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
    const unsigned long seventyYears = 2208988800UL;     
    // subtract seventy years:
    unsigned long epoch = secsSince1900 - seventyYears + centralTime;                              


    // print the hour, minute and second:
   // Serial.print("The UTC time is ");       // UTC is the time at Greenwich Meridian (GMT)
    //Serial.print((epoch  % 86400L) / 3600); // print the hour (86400 equals secs per day)
    hours = (epoch  % 86400L) / 3600;
   // Serial.print(':');  
    if ( ((epoch % 3600) / 60) < 10 ) {
      // In the first 10 minutes of each hour, we'll want a leading '0'
     // Serial.print('0');
    }
    //Serial.print((epoch  % 3600) / 60); // print the minute (3600 equals secs per minute)
    minutes = (epoch  % 3600) / 60;
    if ( (epoch % 60) < 10 ) {
      // In the first 10 seconds of each minute, we'll want a leading '0'
      //Serial.print('0');
        }
    }
    if(hours > 12)
    {
        after_noon = 'P';
    }
    else
    {
        after_noon = 'A';
    }
    hours = hours%12;
    //Serial.println(hours);
    time = "";
    time += hours;
    time += ":";
    if(minutes <10)
        time+="0";
    time += minutes;
    time += after_noon;
    time += "M";
    //Serial.println(time);
}

unsigned long sendNTPpacket(IPAddress& address)
{
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE); 
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49; 
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:         
  Udp.beginPacket(address, 123); //NTP requests are to port 123
  Udp.write(packetBuffer,NTP_PACKET_SIZE);
  Udp.endPacket(); 
}

void water_time(void)
{
    for(int i = 0; i<NUM_ZONES; i++)
        {
            if(zone[i].Time1.equals(time) && ZoneState[i] == 'A')
            {

            }
            else if (zone[i].Time2.equals(time)  && ZoneState[i] == 'A')
            {

            }
            else if (zone[i].Time3.equals(time) && ZoneState[i] == 'A')
            {
                
            }
            else if (zone[i].Time4.equals(time) && ZoneState[i] == 'A')
            {
                
            }
        }
}