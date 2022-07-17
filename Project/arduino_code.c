#include <Wire.h>
#define SLAVE_ADDRESS 0x09


unsigned char c,pin,mode;
void setup()
{
  Serial.begin(9600); // open the serial port at 9600 bps:
  Wire.begin(SLAVE_ADDRESS);
  Wire.onReceive(receiveEvent);
  Wire.onRequest(sendEvent);
  pinMode(13,OUTPUT);

}
void loop()
{
  delay(100);
}
void receiveEvent(int howMany)
{
  Serial.println("Start of While Loop");

 
if (Wire.available()>0) 
  {
    
    Serial.println("Recieving Event Start\n");
     
          c = Wire.read();// receive byte as a character
          int x=(int)c;
          pin=x/10;
          Serial.print("\n pin :");
          Serial.print(pin);
    
          mode=x%10;
          Serial.print("\n mode :");
          Serial.print(mode);
          
          switch(mode)
          {
              case 0: digitalWrite(pin,LOW);
                      break;
              case 1: digitalWrite(pin,HIGH);
                      break;
              case 2: pinMode(pin,OUTPUT);
                      break;
              case 3: pinMode(pin,INPUT);
                      break;
              case 4:break;
          }
         
    Serial.println("\nRecieving Event End");
   
  }
   Serial.println("End of while loop.");
  
}
void sendEvent()
{
  Serial.print("sending event start\n");
  mode=digitalRead(pin);
  
  Wire.write(mode);
  Serial.print("\n pin :");
          Serial.print(pin);
           Serial.print("\n mode :");
          Serial.print(mode);
   Serial.print("sending event end\n");
}
