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
#define NUM_PROPS 8
#define BUFFER_SIZE 64

// MAC address from Ethernet shield sticker under board
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192, 168, 1, 177); // IP address, may need to change depending on network
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
char ZoneState[NUM_ZONES] = {'C'}; //stores zone active states
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

//TODO: IMPLEMENT LOG FILE AND USER LIST
//char //log_file[LOG_SIZE][LOG_MESSAGE] = {0};
//char email_list[NUM_USERS][30] = {0};


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
                    pch = strtok (HTTP_req,"&,  ");
                    int elemountcount=0;
                    while (pch != NULL)
                    {
                       // Serial.println(pch);
                        pch = strtok (NULL, "&,");
                        parsed_GET[elemountcount] = pch;
                        elemountcount++;
                    }
                    for(int i =0; i<elemountcount; i++)
                    {
                        Serial.println(parsed_GET[i]);
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
    if(parsed_GET[0].equals("button")) //EXAMPLE GET REQUEST: "&button,1,A"
    {
        switch(parsed_GET[1].toInt())
        {
            //ZONE 1
            case 1:         
                if(parsed_GET[2].charAt(0) == 'O')
                {
                    Serial.println("top level");
                    if(ZoneState[0] == 'O')
                    {
                        break;
                    }
                    else
                    {
                        Serial.println("enter");
                        ZoneState[0] = 'O';
                        digitalWrite(ZONE1_PIN_OUT,OPEN);
                        //log_file[ZONE1][0] = 'O';
                        //log_file[ZONE1][1] = 'M';
                        //for(int i =0; i< time.length(); i++)
                        //{
                        //    //log_file[ZONE1][i+2] = time.charAt(i);
                        //}

                        //for (int i =0; i< 10; i++)
                        //{
                        //    Serial.print(//log_file[ZONE1][i]);
                        //}

                        break;
                    }
                }

                else if(parsed_GET[2].charAt(0) == 'C')
                {
                    if(ZoneState[0] == 'C')
                    {
                        break;
                    }
                    else
                    {
                        ZoneState[0] = 'C';
                        digitalWrite(ZONE1_PIN_OUT,CLOSE);
                        //log_file[ZONE1][0] = 'C';
                        //log_file[ZONE1][1] = 'M';
                        //for(int i =0; i< time.length(); i++)
                        //{
                        //    log_file[ZONE1][i+2] = time.charAt(i);
                        //}

                    }
                }

                else if(parsed_GET[2].charAt(0) == 'A')
                {
                    ZoneState[0] = 'A';

                }
            break;

            //ZONE 2
            case 2:
            if(parsed_GET[2].charAt(0) == 'O')
                {
                    if(ZoneState[1] == 'O')
                    {
                        break;
                    }                    
                    else
                    {
                        ZoneState[1] = 'O';
                        digitalWrite(ZONE2_PIN_OUT,OPEN); 
                        //log_file[ZONE2][0] = 'O';
                        //log_file[ZONE2][1] = 'M';
                        for(int i =0; i< time.length(); i++)
                        {
                            //log_file[ZONE1][i+2] = time.charAt(i);
                        }  

                     
                        break;
                    }
                }

                else if(parsed_GET[2].charAt(0) == 'C')
                {
                    if(ZoneState[1] == 'C')
                    {
                        break;
                    }
                    else
                    {
                        ZoneState[1] = 'C';                        
                        digitalWrite(ZONE2_PIN_OUT,CLOSE);    

                    

                    }
                }

                else if(parsed_GET[2].charAt(0) == 'A')
                {
                    ZoneState[1] = 'A';
                }

            break;

            //ZONE 3
            case 3:
            if(parsed_GET[2].charAt(0) == 'O')
                {
                    if(ZoneState[2] == 'O')
                    {
                        break;
                    }                    
                    else
                    {
                        ZoneState[2] = 'O';
                        digitalWrite(ZONE3_PIN_OUT,OPEN);     
              
                        break;
                    }
                }

                else if(parsed_GET[2].charAt(0) == 'C')
                {
                    if(ZoneState[2] == 'C')
                    {
                        break;
                    }
                    else
                    {
                        ZoneState[2] = 'C';                        
                        digitalWrite(ZONE3_PIN_OUT,CLOSE);   

                    }
                }

                else if(parsed_GET[2].charAt(0) == 'A')
                {
                    ZoneState[2] = 'A';
                }
                
            break;

            //ZONE 4
            case 4:
            if(parsed_GET[2].charAt(0) == 'O')
                {
                    if(ZoneState[3] == 'O')
                    {
                        break;
                    }                    
                    else
                    {
                        ZoneState[3] = 'O';
                        digitalWrite(ZONE4_PIN_OUT,OPEN);  
             
                        break;
                    }
                }

                else if(parsed_GET[2].charAt(0) == 'C')
                {
                    if(ZoneState[3] == 'C')
                    {
                        break;
                    }
                    else
                    {
                        ZoneState[3] = 'C';                        
                        digitalWrite(ZONE4_PIN_OUT,CLOSE);  
                
                    }
                }

                else if(parsed_GET[2].charAt(0) == 'A')
                {
                    ZoneState[1] = 'A';
                }

            break;

            case 5:
           if(parsed_GET[2].charAt(0) == 'O')
                {
                    if(ZoneState[ZONE5] == 'O')
                    {
                        break;
                    }                    
                    else
                    {
                        ZoneState[ZONE5] = 'O';
                        digitalWrite(ZONE5_PIN_OUT,OPEN);  
                  
                        break;
                    }
                }

                else if(parsed_GET[2].charAt(0) == 'C')
                {
                    if(ZoneState[ZONE5] == 'C')
                    {
                        break;
                    }
                    else
                    {
                        ZoneState[ZONE5] = 'C';                        
                        digitalWrite(ZONE5_PIN_OUT,CLOSE);   
           

                    }
                }

                else if(parsed_GET[2].charAt(0) == 'A')
                {
                    ZoneState[ZONE5] = 'A';
                }
                
            break;

            //ZONE 6
            case 6:
            if(parsed_GET[2].charAt(0) == 'O')
                {
                    if(ZoneState[ZONE6] == 'O')
                    {
                        break;
                    }                    
                    else
                    {
                        ZoneState[ZONE6] = 'O';
                        digitalWrite(ZONE6_PIN_OUT,OPEN);
                   
                        break;
                    }
                }

                else if(parsed_GET[2].charAt(0) == 'C')
                {
                    if(ZoneState[ZONE6] == 'C')
                    {
                        break;
                    }
                    else
                    {
                        ZoneState[ZONE6] = 'C';                        
                        digitalWrite(ZONE6_PIN_OUT,CLOSE);
                  

                    }
                }

                else if(parsed_GET[2].charAt(0) == 'A')
                {
                    ZoneState[ZONE6] = 'A';
                }

            break;

            //ZONE 7
            case 7:
            if(parsed_GET[2].charAt(0) == 'O')
                {
                    if(ZoneState[ZONE7] == 'O')
                    {
                        break;
                    }                    
                    else
                    {
                        ZoneState[ZONE7] = 'O';
                        digitalWrite(ZONE7_PIN_OUT,OPEN);  
                 
                        break;
                    }
                }

                else if(parsed_GET[2].charAt(0) == 'C')
                {
                    if(ZoneState[ZONE7] == 'C')
                    {
                        break;
                    }
                    else
                    {
                        ZoneState[ZONE7] = 'C';                        
                        digitalWrite(ZONE7_PIN_OUT,CLOSE); 
                      

                    }
                }

                else if(parsed_GET[2].charAt(0) == 'A')
                {
                    ZoneState[6] = 'A';
                }
            break;

            //ZONE 8
            case 8:
            if(parsed_GET[2].charAt(0) == 'O')
                {
                    if(ZoneState[7] == 'O')
                    {
                        break;
                    }                    
                    else
                    {
                        ZoneState[7] = 'O';
                        digitalWrite(ZONE8_PIN_OUT,OPEN);                        
                        break;
                    }
                }

                else if(parsed_GET[2].charAt(0) == 'C')
                {
                    if(ZoneState[7] == 'C')
                    {
                        break;
                    }
                    else
                    {
                        ZoneState[7] = 'C';                        
                        digitalWrite(ZONE8_PIN_OUT,CLOSE);                        

                    }
                }

                else if(parsed_GET[2].charAt(0) == 'A')
                {
                    ZoneState[7] = 'A';
                }
            break;

            case 9:
            if(parsed_GET[2].charAt(0) == 'O')
                {
                    if(ZoneState[8] == 'O')
                    {
                        break;
                    }                    
                    else
                    {
                        ZoneState[8] = 'O';
                        digitalWrite(ZONE9_PIN_OUT,OPEN);                        
                        break;
                    }
                }

                else if(parsed_GET[2].charAt(0) == 'C')
                {
                    if(ZoneState[8] == 'C')
                    {
                        break;
                    }
                    else
                    {
                        ZoneState[8] = 'C';                        
                        digitalWrite(ZONE9_PIN_OUT,CLOSE);                        

                    }
                }

                else if(parsed_GET[2].charAt(0) == 'A')
                {
                    ZoneState[8] = 'A';
                }
            break;

            //ZONE 10
            case 10:
            if(parsed_GET[2].charAt(0) == 'O')
                {
                    if(ZoneState[9] == 'O')
                    {
                        break;
                    }                    
                    else
                    {
                        ZoneState[9] = 'O';
                        digitalWrite(ZONE10_PIN_OUT,OPEN);                        
                        break;
                    }
                }

                else if(parsed_GET[2].charAt(0) == 'C')
                {
                    if(ZoneState[9] == 'C')
                    {
                        break;
                    }
                    else
                    {
                        ZoneState[9] = 'C';                        
                        digitalWrite(ZONE10_PIN_OUT,CLOSE);                        

                    }
                }

                else if(parsed_GET[2].charAt(0) == 'A')
                {
                    ZoneState[9] = 'A';
                }
            break;

            case 11:
            if(parsed_GET[2].charAt(0) == 'O')
                {
                    if(ZoneState[10] == 'O')
                    {
                        break;
                    }                    
                    else
                    {
                        ZoneState[10] = 'O';
                        digitalWrite(ZONE11_PIN_OUT,OPEN);                        
                        break;
                    }
                }

                else if(parsed_GET[2].charAt(0) == 'C')
                {
                    if(ZoneState[10] == 'C')
                    {
                        break;
                    }
                    else
                    {
                        ZoneState[10] = 'C';                        
                        digitalWrite(ZONE11_PIN_OUT,CLOSE);                        

                    }
                }

                else if(parsed_GET[2].charAt(0) == 'A')
                {
                    ZoneState[1] = 'A';
                }   
            break;

            case 12:
            if(parsed_GET[2].charAt(0) == 'O')
                {
                    if(ZoneState[11] == 'O')
                    {
                        break;
                    }                    
                    else
                    {
                        ZoneState[11] = 'O';
                        digitalWrite(ZONE12_PIN_OUT,OPEN);                        
                        break;
                    }
                }

                else if(parsed_GET[2].charAt(0) == 'C')
                {
                    if(ZoneState[11] == 'C')
                    {
                        break;
                    }
                    else
                    {
                        ZoneState[11] = 'C';                        
                        digitalWrite(ZONE12_PIN_OUT,CLOSE);                        

                    }
                }

                else if(parsed_GET[2].charAt(0) == 'A')
                {
                    ZoneState[11] = 'A';
                }
            break;

            case 13:
            if(parsed_GET[2].charAt(0) == 'O')
                {
                    if(ZoneState[12] == 'O')
                    {
                        break;
                    }                    
                    else
                    {
                        ZoneState[12] = 'O';
                        digitalWrite(ZONE13_PIN_OUT,OPEN);                        
                        break;
                    }
                }

                else if(parsed_GET[2].charAt(0) == 'C')
                {
                    if(ZoneState[12] == 'C')
                    {
                        break;
                    }
                    else
                    {
                        ZoneState[12] = 'C';                        
                        digitalWrite(ZONE13_PIN_OUT,CLOSE);                        

                    }
                }

                else if(parsed_GET[2].charAt(0) == 'A')
                {
                    ZoneState[12] = 'A';
                }
            break;

            case 14:
            if(parsed_GET[2].charAt(0) == 'O')
                {
                    if(ZoneState[13] == 'O')
                    {
                        break;
                    }                    
                    else
                    {
                        ZoneState[13] = 'O';
                        digitalWrite(ZONE14_PIN_OUT,OPEN);                        
                        break;
                    }
                }

                else if(parsed_GET[2].charAt(0) == 'C')
                {
                    if(ZoneState[13] == 'C')
                    {
                        break;
                    }
                    else
                    {
                        ZoneState[13] = 'C';                        
                        digitalWrite(ZONE14_PIN_OUT,CLOSE);                        

                    }
                }

                else if(parsed_GET[2].charAt(0) == 'A')
                {
                    ZoneState[13] = 'A';
                }
            break;

            case 15:
            if(parsed_GET[2].charAt(0) == 'O')
                {
                    if(ZoneState[14] == 'O')
                    {
                        break;
                    }                    
                    else
                    {
                        ZoneState[14] = 'O';
                        digitalWrite(ZONE15_PIN_OUT,OPEN);                        
                        break;
                    }
                }

                else if(parsed_GET[2].charAt(0) == 'C')
                {
                    if(ZoneState[14] == 'C')
                    {
                        break;
                    }
                    else
                    {
                        ZoneState[14] = 'C';                        
                        digitalWrite(ZONE15_PIN_OUT,CLOSE);                        

                    }
                }

                else if(parsed_GET[2].charAt(0) == 'A')
                {
                    ZoneState[14] = 'A';
                }
            break;

            case 16:
            if(parsed_GET[2].charAt(0) == 'O')
                {
                    if(ZoneState[15] == 'O')
                    {
                        break;
                    }                    
                    else
                    {
                        ZoneState[15] = 'O';
                        digitalWrite(ZONE16_PIN_OUT,OPEN);                        
                        break;
                    }
                }

                else if(parsed_GET[2].charAt(0) == 'C')
                {
                    if(ZoneState[15] == 'C')
                    {
                        break;
                    }
                    else
                    {
                        ZoneState[15] = 'C';                        
                        digitalWrite(ZONE16_PIN_OUT,CLOSE);                        

                    }
                }

                else if(parsed_GET[2].charAt(0) == 'A')
                {
                    ZoneState[15] = 'A';
                }

            break;

    }

    if(parsed_GET[0].equals("setup"))
    {
        int zone_update = parsed_GET[1].toInt();
        zone_update = zone_update-1; //CONVERT TO 0 BASED NUMBERING


        zone[zone_update].Name = parsed_GET[2];
        zone[zone_update].Visible = parsed_GET[3].toInt();


        if(SD.exists("config.h"))
        {
            SD.remove("config.h");
        }
        File config_file = SD.open("config.h", FILE_WRITE);
        if (config_file) {
            for(int i =0; i< NUM_ZONES; i++)
            {
                config_file.print(zone[i].Name);
                config_file.print(",");
                config_file.print(zone[i].Visible);
                config_file.print(",");
                config_file.print(zone[i].Time1);
                config_file.print(",");
                config_file.print(zone[i].duration1);
                config_file.print(",");                
                config_file.print(zone[i].Time2);
                config_file.print(",");
                config_file.print(zone[i].duration2);
                config_file.print(",");                
                config_file.print(zone[i].Time3);
                config_file.print(",");
                config_file.print(zone[i].duration3);
                config_file.print(",");

            }
        }
        config_file.close();

    }

    if(parsed_GET[0].equals("config"))
    {
        int zone_update = parsed_GET[1].toInt();
        zone_update = zone_update-1; //CONVERT TO 0 BASED NUMBERING
        zone[zone_update].Time1 = parsed_GET[2];
        zone[zone_update].duration1 = parsed_GET[3].toInt();
        zone[zone_update].Time2 = parsed_GET[4];
        zone[zone_update].duration2 = parsed_GET[5].toInt();
        zone[zone_update].Time3 = parsed_GET[6];
        zone[zone_update].duration3 = parsed_GET[7].toInt();

        //UPDATE config.h
        if(SD.exists("config.h"))
        {
            SD.remove("config.h");
        }
        File config_file = SD.open("config.h", FILE_WRITE);
        if (config_file) {
            for(int i =0; i< NUM_ZONES; i++)
            {
                config_file.print(zone[i].Name);
                config_file.print(",");
                config_file.print(zone[i].Visible);
                config_file.print(",");
                config_file.print(zone[i].Time1);
                config_file.print(",");
                config_file.print(zone[i].duration1);
                config_file.print(",");                
                config_file.print(zone[i].Time2);
                config_file.print(",");
                config_file.print(zone[i].duration2);
                config_file.print(",");                
                config_file.print(zone[i].Time3);
                config_file.print(",");
                config_file.print(zone[i].duration3);
                config_file.print(",");

            }
        }
        config_file.close();
    }
    
    }
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

        cl.print("<state>");
        cl.print(ZoneState[i]);
        cl.print("</state>");
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
    Serial.print("test ");
    char found = 0;
    char index = 0;
    char len;

    len = strlen(str);
    Serial.println((int)len);
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
        }
}
