#include <Wire.h>
#include "FileLoader.h"



FileLoader loader;

void setup()
{
  Wire.begin(4);                // join i2c bus with address #4
  Wire.onReceive(interrupt); // register event
  Serial.begin(9600);           // start serial for output
}

void loop(){
  while(1) {}
}

void interrupt() {
  //get Interrupt num
   String params[3];
   int count = 0;
   int charCount = 0;
   int x = Wire.read(); 
  while(Wire.available()) {
    char c = Wire.read(); 
    if(c == ',') { count++; charCount = 0; empty(params[1]); empty(params[1]); empty(params[1]);} 
    else {params[count] += c;   charCount++;}
  }
  handleInterrupt21(x,params[0],params[1],params[2]);
}

void handleInterrupt21(int ax, String bx, String cx, String dx) {
  switch(ax) {
    case 0:
      char* newBx;
      bx.toCharArray(newBx,bx.length());
      printString(newBx);
      break;
    case 1:
      //readString(bx);
      break;
    case 2:
      //readSector(bx,cx);
      break;
    case 3: 
      //readFile(bx,cx);
      break;
    case 4:
      //executeProgram(bx,cx);
      break;
    case 5:
      //terminate();
      break;
    case 6:
      //writeSector(bx,cx);
      break;
    case 7:
      //deleteFile(bx);
      break;
    case 8: 
      //writeFile(bx,cx,dx);
      break;
    default:
      printString("You have done goofed up\0"); 
      break;
  }
}

void printString(char* string) {
  Serial.print(string);
}

void executeProgram(char* name) {
  char buffer[512];
  readFile(name, buffer);
  loader.startProgramming();
  loader.getSignature();
  loader.getFuseBytes();
  loader.writeFlashContents(buffer);
  loader.stopProgramming();
}


void readSector(char buffer[512], int sector) {
    SDCARD.readblock(sector,53,buffer);
}


void writeSector(char buffer[512], int sector) { 
  SDCARD.writeblock(sector,53,buffer);
}

void readFile(char* name , char fileBuffer[4096]) {
  char buff[512];
  int x;
  int i = 0;
  int j = 0;
  int p = 0;
  int k = 0;
  int end = 0;
  int wordLen;
  char temp[512];
 
  for(p = 0 ; p < 4096; p++) { fileBuffer[p] = 0x00; } 
  readSector(buff,2);
  while(name[j] != 0x00) { j++; }
  wordLen = j;
  for(x = 0; x <= 512; x++) {
     if(i == wordLen && buff[x] == 0x00) {
      x = x-wordLen+6;
     // Serial.print(buff[x]);
      for(j = 0; j < 4; j++){
          if(buff[x+j] == 0x00) {
            fileBuffer[(j*512)] = 0x00;
            break;
          }
          
          readSector(temp, buff[x+j]); 
          for(k = 0 ; k < 512; k++) { fileBuffer[k+(j*512)] = temp[k]; }     
        }
        break;
    } 
    else if(buff[x] == name[i]) { i++; }
    else { 
      x = x -i+31;
      i = 0; 
    }
  } 


}
void writeFile(char* name, char* buffer, int numberOfSectors) {
  char map[512];
  char temp[512];
  char directory[512];
  int end = 0;
  int i = 0;
  int t = 0;
  readSector(map,1);
  readSector(directory,2);
  for(int x = 0; x <= 512; x++) {
    if(directory[x] != 0x00) {
        x = x +31;
        i = 0;  
    } else if(directory[x] == 0x00) { 
      for(int j = 0; j < 6; j++){
        if(end == 1) {
          directory[x+j] = 0x00;
        }
        else if(name[j] == 0x00) { 
          directory[x+j] = 0x00;
          end = 1;
        } else {
          directory[x+j] = name[j];
        }   
      }
      
      for(int i = 0; i < numberOfSectors; i++) {
        for(int k = 5; k < 512; k++) {
          if(map[k] == 0x00) {
            
            map[k] = 0xFF;
            directory[x+6+i] = k;
            for(int p = 0; p < 512; p++) {
              temp[p] = buffer[t*512 + p];
              
            }
            writeSector(temp,k);
            t++;
            break;  
          }
        }
      }
      for(int i = i+1; i < 26-numberOfSectors; i++) {
        directory[x+6+i] = 0x00;    
      }
      break;
    }
    else { i++; }
  }
  writeSector(map,1);
  writeSector(directory,2);
}

void deleteFile(char* name) {
  char map[512];
  char directory[512];
  int x;
  int i = 0;
  int j = 0;
  int k = 0;
  int wordLen;
  
  readSector(map,1);
  readSector(directory,2);
  
  while(name[j] != 0x00) {
    j++;
  }
  wordLen = j;
  for(x = 0; x <= 512; x++) {
     if(i == wordLen && directory[x] == 0x00) { 
      directory[x-i] = 0x00;  
      for(j = 0; j < 26; j++){
        map[x+j] = 0x00;    
      }
      break;
    } else if(directory[x] == name[i]) {
      i++;
      
    }
    else { 
      x = x-i +31;
      i = 0; 
    }
  }
  writeSector(map,1);
  writeSector(directory,2);

}

void empty(String array) {
 array = "";
}
