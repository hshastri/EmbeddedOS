#include <Wire.h>
#include "FileLoader.h"

//the Wire library helps us to communicate with I2C devices

FileLoader loader;

void setup()
{
  Wire.begin(4);                // join i2c bus with address #4
  Wire.onReceive(interrupt);    // register event
  Serial.begin(9600);           // start serial for output
}

void loop(){
  while(1) {}
}

//this function essentially makes the interrupts happen - interrupts are nothing but signals for the device
void interrupt() {
  //get Interrupt num which tells us 
   String params[3]; //array of the characters taken as parameters
   int count = 0; 
   int charCount = 0; 
   int x = Wire.read();  //stores the data in bytes 
   //read function essentially reads the bytes from master to slave device
  
  //the available() function in the wire library returns the number of bytes available for retrieval
  //this loop runs as long as there are bytes available 
  while(Wire.available()) {
    char c = Wire.read(); 
    if(c == ',') { count++; charCount = 0; empty(params[1]); empty(params[1]); empty(params[1]);} 
    else {params[count] += c;   charCount++;}
  }
  handleInterrupt21(x,params[0],params[1],params[2]); //call handle interrupt on the given chraters in the params array
}

//this function entails interrupt service routine 
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

//this function takes a character array as a parameter
void printString(char* string) {
  Serial.print(string);
}

//executes the program. Takes in name of the program as parameter
void executeProgram(char* name) {
  char buffer[512];
  readFile(name, buffer);
  loader.startProgramming();
  loader.getSignature();
  loader.getFuseBytes();
  loader.writeFlashContents(buffer);
  loader.stopProgramming();
}

//this function reads the sector from the device 
void readSector(char buffer[512], int sector) {
    SDCARD.readblock(sector,53,buffer);
}

//writes the sector and takes in sector and the character array
void writeSector(char buffer[512], int sector) { 
  SDCARD.writeblock(sector,53,buffer);
}

//takes character array containing the file name and reads the file into buffer
void readFile(char* name , char fileBuffer[4096]) {
  char buff[512];
  int x;
  int i = 0;
  int j = 0;
  int p = 0;
  int k = 0;
  int end = 0;
  int wordLen;
  char temp[512]; //storage capacity of the sector is 512 bytes
 
  //going to the directory to file name and load the file sector by sector into the buff - buffer array

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

//writes the given file. Takes in file name, a character array holding the file name, and number of sectors as parameters
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
    } else if(directory[x] == 0x00) { //finding a free directory entry 
      for(int j = 0; j < 6; j++){ //name is 6 bytes
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
          if(map[k] == 0x00) { //finding a free sector through the map
            
            map[k] = 0xFF; //setting the sector to 0xFF
            directory[x+6+i] = k;

            //add the sector number to the file's directory entry
            for(int p = 0; p < 512; p++) {
              temp[p] = buffer[t*512 + p];
              
            }
            //writing the bytes from buffer to that sector
            writeSector(temp,k);
            t++;
            break;  
          }
        }
      }
      //filling the remaining bytes entry to 0x00
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

//deletes the file. Takes in the file name as a parameter
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

//empties the given array. Takes in the array to empty as a parameter
void empty(String array) {
 array = "";
}
