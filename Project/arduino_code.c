#include <Wire.h>
#define SLAVE_ADDRESS 0x08
unsigned char c;
unsigned int pin,mode;
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
  int counter = 0;
 
while (Wire.available()>0) // loop through all but the last
  {
    
    Serial.println("Recieving Event Start\n");
     
              c = Wire.read();// receive byte as a character
              Serial.print(c);
              Serial.print(" \npin\n");
              int x=(int)c;
              if(x==120)
              {
                pinMode(12,OUTPUT);
                digitalWrite(12,HIGH);
                digitalWrite(13,HIGH);
              }
              else
              {
                pinMode(12,OUTPUT);
                digitalWrite(12,LOW);
                digitalWrite(13,LOW);
              }
    Serial.println("\nRecieving Event End");
    counter++;
  }
  //Serial.println(counter);
  //unsigned int x = Wire.read();    // receive byte as an integer
  //Serial.println(x);
}
void sendEvent()
{
  Serial.print("send data\n");
  Wire.write(mode);
}
