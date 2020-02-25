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
  Wire.begin();
}
void interrupt(int ax,char* bx,char* cx,char* dx) {
  Wire.beginTransmission(4); 
  Wire.write(ax);
  char* send = malloc((sizeof(bx)+sizeof(cx)+sizeof(dx)));
  strcpy(send,bx);
  strcat(send,",");
  strcat(send,cx);
  strcat(send,",");
  strcat(send,dx);
  Wire.write(send);
  Wire.endTransmission();
}
