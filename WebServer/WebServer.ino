#include <Ethernet.h>
#include <SPI.h>

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED }; //physical mac address
byte ip[] = { 192, 168, 8, 177 }; // ip in lan
byte gateway[] = { 192, 168, 8, 177 }; // internet access via router
byte subnet[] = { 255, 255, 255, 0 }; //subnet mask
EthernetServer server(80); //server port

String readString; 
String BUTTON1_NAME = "TURN_ON";
int BUTTON1_BOOL = 0;


//////////////////////

String incoming; 

void setup(){

  pinMode(5, OUTPUT); //pin selected to control
  //start Ethernet
  Ethernet.begin(mac, ip);
  server.begin();

  //enable serial data print 
  Serial.begin(9600); 
  Serial.println(Ethernet.localIP()); // print your IP Address on Serial Monitor 
}

void loop(){
  // Create a client connection
  EthernetClient client = server.available();
  if (client) {
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();

        //read char by char HTTP request
        if (incoming.length() < 100) {

          //store characters to string 
          incoming += c; 
          //Serial.print(c);
        } 

        //if HTTP request has ended
        if (c == '\n') {

          ///////////////
          Serial.println(incoming); //print to serial monitor for debuging 

          //now output HTML data header
             if(incoming.indexOf('?') >=0) { //don't send new page
               client.println("HTTP/1.1 204 no data");
               client.println();
               client.println();  
             }
             else {
          client.println("HTTP/1.1 200 OK"); //send new page
          client.println("Content-Type: text/html");
          client.println();

          client.println("<html>");
          client.println("<head>");
          client.println("<TITLE> Ames High Greenhouse Control </TITLE>");
          client.println("</head>");
          client.println("<body>");

          client.println("<H2>Ames Greenhouse: Insert Junk here</H2>");
          client.println("<H4>Turn On/Off LED connected To pin D5 </H4>");
          client.print(  "<FORM action=\"192.168.8.177\" >");

          client.println("<input type= submit name =Button1 value=" + BUTTON1_BOOL) + "></FORM>");
          client.println("<input type= submit name = Button2 value=" + BUTTON1_BOOL + "></FORM>");
          client.println();
          client.println("<input type= submit name = Button3 value=" + BUTTON1_NAME + "></FORM>");

          Serial.println("Button1 test = " + incoming.indexOf("GET /?Button1=1"));
         
          client.println("</BODY>");
          client.println("</HTML>");
             }

          delay(1);
          //stopping client
          client.stop();

       
          if(incoming.indexOf("GET /?Button1=1") >=0)
          {
            digitalWrite(5, HIGH);    
            Serial.println("TURNED ON");
            BUTTON1_NAME = "TURN_OFF";
            BUTTON1_BOOL = 1;
          }
          if(incoming.indexOf("GET /?Button1=0") >=0)
          {
            digitalWrite(5, HIGH);    
            Serial.println("TURNED OFF");
            BUTTON1_NAME = "TURN_ON";
            BUTTON1_BOOL = 0;
          }

          if(incoming.indexOf("GET /?Button2=0") >=0)//checks for off
          {
            digitalWrite(5, LOW);    
            //Serial.println("Led Off");
           
          }
          delay(10);
          incoming=" ";//clear 

        }
      }
    }
  }
} 