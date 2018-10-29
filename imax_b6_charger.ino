/*
 * Joe Ostrander
 * monitor skyrc imax charger
 * 20180316
 *Notes:  see https://blog.dest-unreach.be/2012/01/29/imax-b6-charger-protocol-reverse-engineered
 */


#include <SoftwareSerial.h>

//PIN 10 to middle pin of imax b6 charger
// pinout = 5v/TX/GND
SoftwareSerial mySerial(10, 11); // RX, TX

unsigned long lastDataMs;


void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(57600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }


  mySerial.begin(9600);
  
}

void loop() 
{
  int count = 0;  
  if (mySerial.available()>0) 
  {

    byte incomingByte = mySerial.read();
    
    if (incomingByte == '{') // { = 0x7B 
    {
      Serial.println();
      //Serial.println("************* INCOMING");
      
      byte buf[75]; //74 data bytes + end }
      mySerial.readBytes(buf,75);
      /*
      int pos = 0;
      while (mySerial.available()>0)
      {
        buf[pos] = mySerial.read();
        pos+=1;
        if (buf[pos]=='}')
        {
          Serial.println("************* END");
        }
        else if (pos > 74) 
        {
          Serial.println("OVERFLOW?");
          break;
        }
      }
      */
      if (buf[74] == '}')
      {
        for (int i = 0; i < 74; i++) 
        {
          buf[i] &= ~(1<<7); // clear bit 8
          /*
          Serial.print(buf[i],HEX);
          Serial.print(" ");
          if ((i+1) % 4 == 0)
          {
            Serial.println("");
          }
          */
        }

        Serial.println();
        Serial.println((buf[23] & (1<<0)) ? "Status:  Running" : "Status:  Standby");
        Serial.println(buf[7] & (1<<0) ? "Mode:  Charge" : "Mode:  Discharge");
        Serial.print("Program select:  ");
        switch (buf[22])
        {
          case 0x00:
            Serial.println("Config");
            break;
          case 0x05:
            Serial.println("Save");
            break;
          case 0x06:
            Serial.println("Load");
            break;
          case 0x01:
            Serial.println("LiPo");
            break;
          case 0x02:
            Serial.println("NiMH");
            break;
          case 0x03:
            Serial.println("NiCd");
            break;
          case 0x04:
            Serial.println("Pb");
            break;
          default:
            break;
        }

        int intCells = buf[17];
        Serial.print("LiPo Cells:  ");
        Serial.println(intCells);
        Serial.print("LiPo Charge Current:  ");
        Serial.println(buf[16]/10.0);

        for (int i = 0; i < intCells; i++)
        {
          Serial.print("Cell ");
          Serial.print(i+1);
          Serial.print(" Voltage:  ");
          Serial.println( buf[44+(i*2)] + buf[45+(i*2)]/100.0 );
        }

        Serial.print("Current:  ");
        Serial.println(buf[32] + buf[33]/100.0);

        Serial.print("Voltage:  ");
        Serial.println(buf[34] + buf[35]/100.0);

        Serial.print("Input Voltage:  ");
        Serial.println(buf[40] + buf[41]/100.0);

        Serial.print("Charge:  ");
        Serial.println(buf[42]*100.0 + buf[43]);

        //elapsed time??
        Serial.print("Time:  ");
        Serial.print(buf[69]);
        Serial.println(" minutes");
      }

    }

    lastDataMs = millis();
  }
  else
  {
    if (millis() - lastDataMs > 10000)
    {
      Serial.println("NO RESPONSE!");
      delay(1000);
    }
  }

}

