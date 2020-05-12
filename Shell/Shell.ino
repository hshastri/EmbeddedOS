#include <Wire.h>

void setup() {
  // put your setup code here, to run once:
  initInterrupt();
}

void loop() {
  // put your main code here, to run repeatedly:
interrupt(1,"PO\0","op\0","w\0");
delay(3000);
}


void initInterrupt() {
  Wire.begin(); //initiates the Wire library and joins the I2C bus
}
void interrupt(int ax,char* bx,char* cx,char* dx) {
  Wire.beginTransmission(4); //begin the transition to the I2C device
  Wire.write(ax); //writes data from the slave device. ax is the character sent as a series of bytes
  char* send = malloc((sizeof(bx)+sizeof(cx)+sizeof(dx))); //allocates the size by assing the size
  strcpy(send,bx);
  strcat(send,",");
  strcat(send,cx);
  strcat(send,",");
  strcat(send,dx);
  Wire.write(send); //send is number of bytes to write 
  Wire.endTransmission(); //ends the transition to the I2C device
}
