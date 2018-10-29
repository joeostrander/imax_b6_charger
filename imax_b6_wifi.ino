
#include <ESP8266WiFi.h>

const char* ssid = "<enter your ssid>";
const char* password = "<your wifi password>";

// Create an instance of the server
// specify the port to listen on as an argument
WiFiServer server(80);

unsigned long lastDataMs;

byte buf[75]; //74 data bytes + end }  this will store the last response

void setup() {
  Serial.begin(9600);
  delay(10);

  // prepare GPIO2
  pinMode(2, OUTPUT);
  digitalWrite(2, 0);
  
  // Connect to WiFi network
  //Serial.println();
  //Serial.println();
  //Serial.print("Connecting to ");
  //Serial.println(ssid);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  
  // Start the server
  server.begin();
  Serial.println("Server started");

  // Print the IP address
  Serial.println(WiFi.localIP());
}

void clearBuffer() 
{
  for (int i = 0; i < 74; i++) 
  {
    buf[i] = 0;
  }
}

void updateSerial() 
{
  int count = 0;  
  if (Serial.available()>0) 
  {

    byte incomingByte = Serial.read();
    //Serial.print(incomingByte,HEX);
    //Serial.print(" ");
    Serial.println("***** INCOMING DATA *****");
    if (incomingByte == '{') // { = 0x7B 
    {
      
      Serial.readBytes(buf,75);
      
      if (buf[74] == '}')
      {
        for (int i = 0; i < 74; i++) 
        {
          buf[i] &= ~(1<<7); // clear bit 8
          /*
          
          Serial.print( buf[i],HEX);
          Serial.print(" ");
          if ((i+1) % 4 == 0) 
          {
            Serial.println();
          }
          */
        }
        //Serial.println("");
        Serial.println((buf[23] & (1<<0)) ? "Status:  Running" : "Status:  Standby");
        Serial.println("***** END OF TRANSMISSION *****");
  
      }
  
      lastDataMs = millis();
    }

  }
}

void loop() {

  //update serial data
  updateSerial();


  String html = "";
  if (buf[0])
  {
    //change LED based on charging/discharging
    int state = (buf[23] & (1<<0)) ? HIGH : LOW;
    digitalWrite(2, state);

    html += (state) ? "Status:  Running<br>\n" : "Status:  Standby<br>\n";
    html += (buf[7] & (1<<0)) ? "Mode:  Charge<br>\n" : "Mode:  Discharge<br>\n";
    html += "Program select:  ";
    switch (buf[22]){
      case 0x00:
        html += "Config<br>\n";
        break;
      case 0x05:
        html += "Save<br>\n";
        break;
      case 0x06:
        html += "Load<br>\n";
        break;
      case 0x01:
        html += "LiPo<br>\n";
        break;
      case 0x02:
        html += "NiMH<br>\n";
        break;
      case 0x03:
        html += "NiCd<br>\n";
        break;
      case 0x04:
        html += "Pb<br>\n";
        break;
      default:
        break;
    }

    html += "Cells:  ";
    int intCells = buf[17];
    html += String(intCells);
    html += "<br>\n";
    
    html += "Charge Current:  ";
    html += String(buf[16]/10.0);
    html += "<br>\n";

    for (int i = 0; i < intCells; i++)
    {
      html += " - Cell ";
      html += String(i+1);
      html += " Voltage:  ";
      html += String(buf[44+(i*2)] + buf[45+(i*2)]/100.0 );
      html += "<br>\n";
    }


    html += "Current:  ";
    html += String(buf[32] + buf[33]/100.0);
    html += "<br>\n";
    
    html += "Voltage:  ";
    html += String(buf[34] + buf[35]/100.0);
    html += "<br>\n";
    
    html += "Input Voltage:  ";
    html += String(buf[40] + buf[41]/100.0);
    html += "<br>\n";
    
    html += "Charge:  ";
    html += String(buf[42]*100.0 + buf[43]);
    html += "<br>\n";
    
    //add seconds??
    html += "Time:  ";
    html += String(buf[69]);
    html += " minutes<br>\n";

    Serial.println(html);
  
  }

  
  clearBuffer();


    // Check if a client has connected
  WiFiClient client = server.available();
  if (!client) {
    return;
  }

  // Read the first line of the request
  String req = client.readStringUntil('\r');
  Serial.println(req);
  client.flush();
  
  // Wait until the client sends some data
  Serial.println("new client");
  while(!client.available()){
    delay(1);
  }
  

  // Prepare the response
  String s = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!doctype html>\r\n<html>\r\n";

  String style = "<style>";
  style += "body {\n";
  style += "background-color: black;\n";
  style += "color: cornflowerblue;\n";
  style += "}\n";
  style += "</style>";

      if (millis() - lastDataMs > 10000)
    {
      Serial.println("NO RESPONSE!");
      html="NO RESPONSE!";
      //delay(1000);
    }
    s+= html;
  s += "</html>\n";

  // Send the response to the client
  client.print(s);
  delay(1);
  Serial.println("Client disconnected");

  // The client will actually be disconnected 
  // when the function returns and 'client' object is detroyed
}


