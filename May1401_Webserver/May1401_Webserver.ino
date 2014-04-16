/*--------------------------------------------------------------

Webserver for ISU Senior Design - Ames High Greenhouse System

This code will be built using the Arduino compiler and operates
with an Ethernet shield using the WizNet chipset.
--------------------------------------------------------------*/

#include <SPI.h>
#include <Ethernet.h>
#include <SD.h>
//#include <Globals.ino>
#include <config_parser.ino>

// size of buffer used to capture HTTP requests
#define REQ_BUF_SZ   60
#define NUM_ZONES 16
#define NUM_PROPS 9

// MAC address from Ethernet shield sticker under board
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192, 168, 1, 177); // IP address, may need to change depending on network
EthernetServer server(80);  // create a server at port 80
File webFile;               // the web page file on the SD card
char HTTP_req[REQ_BUF_SZ] = {0}; // buffered HTTP request stored as null terminated string
char req_index = 0;              // index into HTTP_req buffer
boolean LED_state[4] = {0}; // stores the states of the LEDs
boolean ZoneState[NUM_ZONES] = {0}; //stores zone active states
String config_array[NUM_ZONES][NUM_PROPS]; //stores values from website

typedef struct 
{
String Name; 
int Visible; 
String Time1; 
int  Dur1;
String Time2; 
int  Dur2;
String Time3; 
int  Dur3;
String Time4; 
int  Dur4;
} zone_properties;


//Define all zone scructs
zone_properties zone[NUM_ZONES];

void setup()
{
    // disable Ethernet chip
    pinMode(10, OUTPUT);
    digitalWrite(10, HIGH);
    
    Serial.begin(115200);       // for debugging
    
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
        zone[i].Dur1 = config_array[i][3].toInt();
        zone[i].Time2 = config_array[i][4];
        zone[i].Dur2 = config_array[i][5].toInt();
        zone[i].Time3 = config_array[i][6];
        zone[i].Dur3 = config_array[i][7].toInt();
        zone[i].Time4 = config_array[i][8];
        zone[i].Dur4 = config_array[i][9].toInt();

    }

    Serial.println("Parse -> Struct: Complete.");


    
    Ethernet.begin(mac, ip);  // initialize Ethernet device
    server.begin();           // start to listen for clients
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
                    HTTP_req[req_index] = c;          // save HTTP request character
                    req_index++;
                }
                // last line of client request is blank and ends with \n
                // respond to client only after last line received
                if (c == '\n' && currentLineIsBlank) {
                    // send a standard http response header
                    client.println("HTTP/1.1 200 OK");
                    // remainder of header follows below, depending on if
                    // web page or XML page is requested
                    // Ajax request - send XML file
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
                                client.write(webFile.read()); // send web page to client
                            }
                            webFile.close();
                        }
                    }
                    // display received HTTP request on serial port
                    Serial.print(HTTP_req);
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
}

void Zone_States(void)
{
    if (StrContains(HTTP_req, "Zone1=1")) 
    {
        ZoneState[0] = 1;  // save zone1 state
        digitalWrite(6, HIGH); //TODO: define pin 
    }
    else if (StrContains(HTTP_req, "Zone1=0")) 
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
}

// send the XML file with analog values, switch status
//  and LED status
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
 
        cl.print("<dur1>");
        cl.print(zone[i].Dur1);
        cl.print("</dur1>");

        cl.print("<time2>");
        cl.print(zone[i].Time2);
        cl.print("</time2>");

        cl.print("<dur2>");
        cl.print(zone[i].Dur2);
        cl.print("</dur2>");

        cl.print("<time3>");
        cl.print(zone[i].Time3);
        cl.print("</time3>");

        cl.print("<dur3>");
        cl.print(zone[i].Dur3);
        cl.print("</dur3>");

        cl.print("<time4>");
        cl.print(zone[i].Time4);
        cl.print("</time4>");

        cl.print("<dur4>");
        cl.print(zone[i].Dur4);
        cl.print("</dur4>");


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
