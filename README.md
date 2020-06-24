# Embedded Operating Systems

## Installation

Run the following commands to install the source code:

```
git clone https://github.com/hshastri/EmbeddedOS.git
cd EmbeddedOS
```

We are building an Operating System using two Arduinos which is going to handle all fundamental roles of an Operating System. Build The Operating System to handle system calls and to load files and execute programs by storing them in an SD card, which is inserted in one of the Arduinos (the Mega Arduino). One of the Arduinos acts like the Kernel (Mega) and the other (Uno) acts like the Shell. The interaction is done using I2C protocol. The Wire library is heavily used to develop this. An SD card is also used since Arduinos have very limited memory.

Here are some links which are indeed helpful for the project:
https://www.arduino.cc/en/reference/wire
http://www.rjhcoding.com/
https://i2c.info/

The SDCARD and FileLoader, created by Kevin Monteith, are the two libraries heavily used.

## System calls
All the code for this part goes in Kernel.ino 
The interrupt() function is essentially responsible to make the interrupts happen.
Initialize an array of  string which stores the register parameters.
Make a variable which reads in the bytes from the incoming device.
Using a while loop fill the array which stores the register parameters. 
Call handleInterrupt() function on those register parameters which will handle interrupt routines per the  signals from the device .

The handleInterrupt21() function is responsible for Interrupt Service Routine.
The function takes in three registers and an integer (which is data stored in bytes) as parameters.
The function handles interrupt service routines 
The readSector() reads sectors from the device.
Takes in two parameters: buffer array, and the sector as an integer
Use writeBlock() function since we are using an SD Card.
The printString() takes a character array as a parameter and print the string.

## Loading and Executing Files
In Kernel.ino:
The readFile() function takes a character array containing the file name and reads the file into a buffer.
The directory sector is stored into a character array of 512 bytes using the readSector() function
Match the file name by going through the directory. Return if the file is not found.
Load the file by each sector into the buffer array using the sector numbers.
Everytime readSector is called, the buffer address should be added by 512.
Return
The executeProgram() takes in the program as a parameter and executes it.
Create a character buffer array.
Call readFile() function, passing in the name of the program and the buffer array created.
Use the FileLoader library (created by Kevin Monteith). You would need to use these functions: startProgramming(), getSignature(), getFuseBytes(), writeFlashContents(), and stopProgramming().
In Shell.ino:
The initInterrupt() function initializes the interrupt.
The function takes four parameters - three char pointers and an integer.
You would just use the begin() function in the Wire library
The interrupt() function begins transmission between the two I2C devices (the arduino), and runs a session. 
Using the wire library, begin the transition to the I2C device at 4.
Write the integer passed in as parameter using the wire library.
Let’s say b, c and d are the integer parameters you passed in. Do something like this :
 ```
 char* a = malloc((sizeof(b)+sizeof(c)+sizeof(d))); //allocates the size by assing the size
 strcpy(a,b);
 strcat(a,",");
 strcat(a,c);
 strcat(a,",");
 strcat(a,d);
 Wire.write(a); //send is number of bytes to write
```
End the transmission now.
## Writing Files
The writeSector() function takes the sector and the buffer array and writes the sector.
Use the writeblock() function in the SDCARD library
The deleteFile() function takes the name of the file you want to delete and deletes the file basically.
The name of the file is passed in as a char*.
Create two arrays of size 512 bytes. One of the arrays is the directory and the other is map.
Navigate the directory using loops and find the file name.
Then, the first byte of the file name is supposed to be set to 0x00
Go through the sectors numbers listed as corresponding to the file.
Write the character arrays.

The writeFile() function is responsible for actually writing the file.
Takes in the file name, the character array holding the file name and number of sectors as parameters.
The map and directory sectors are loaded into buffers.
Search free directory entry - it should begin with 0x00.
For each sector:
Search a free sector by searching through the Map for a 0x00. 
It is important to not overwrite the bootloader after entry 4.
The remaining bytes are now filed in the directory entry to 0x00.
Now it is time to finally write the Map and Directory sectors back to the disk.

