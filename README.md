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

The SDCARD and FileLoader are the two libraries heavily used.

## System calls
All the code for this part goes in Kernel.ino </br>
The interrupt() function is essentially responsible to make the interrupts happen. </br>
- Initialize an array of  string which stores the register parameters. </br>
- Make a variable which reads in the bytes from the incoming device. </br>
- Using a while loop fill the array which stores the register parameters. </br>
- Call handleInterrupt() function on those register parameters which will handle interrupt routines per the  signals from the device. </br>
</br>
The handleInterrupt21() function is responsible for Interrupt Service Routine.</br>
- The function takes in three registers and an integer (which is data stored in bytes) as parameters.</br>
- The function handles interrupt service routines </br>
The readSector() reads sectors from the device.</br>
- Takes in two parameters: buffer array, and the sector as an integer </br>
- Use writeBlock() function since we are using an SD Card.</br>
The printString() takes a character array as a parameter and print the string.</br>

## Loading and Executing Files
In Kernel.ino:</br>
The readFile() function takes a character array containing the file name and reads the file into a buffer.</br>
- The directory sector is stored into a character array of 512 bytes using the readSector() function </br>
- Match the file name by going through the directory. Return if the file is not found. </br>
- Load the file by each sector into the buffer array using the sector numbers. </br>
- Everytime readSector is called, the buffer address should be added by 512. </br>
- Return </br>
</br>
The executeProgram() takes in the program as a parameter and executes it. </br>
- Create a character buffer array. </br>
- Call readFile() function, passing in the name of the program and the buffer array created. </br>
- Use the FileLoader library (created by Kevin Monteith). You would need to use these functions: startProgramming(), getSignature(), getFuseBytes(), writeFlashContents(), and stopProgramming(). </br>
In Shell.ino: </br>
The initInterrupt() function initializes the interrupt. </br>
- The function takes four parameters - three char pointers and an integer. </br>
- You would just use the begin() function in the Wire library </br>
</br>
The interrupt() function begins transmission between the two I2C devices (the arduino), and runs a session. </br>
- Using the wire library, begin the transition to the I2C device at 4. </br>
- Write the integer passed in as parameter using the wire library. </br>
- Let’s say b, c and d are the integer parameters you passed in. Do something like this : </br>

 ```
 char* a = malloc((sizeof(b)+sizeof(c)+sizeof(d))); //allocates the size by assing the size
 strcpy(a,b);
 strcat(a,",");
 strcat(a,c);
 strcat(a,",");
 strcat(a,d);
 Wire.write(a); //send is number of bytes to write
```
-End the transmission now. 
</br>
## Writing Files
The writeSector() function takes the sector and the buffer array and writes the sector.</br>
- Use the writeblock() function in the SDCARD library </br>
</br>
</br>
The deleteFile() function takes the name of the file you want to delete and deletes the file basically. </br>
* The name of the file is passed in as a char*. </br>
* Create two arrays of size 512 bytes. One of the arrays is the directory and the other is map. </br>
* Navigate the directory using loops and find the file name. </br>
* Then, the first byte of the file name is supposed to be set to 0x00 </br>
* Go through the sectors numbers listed as corresponding to the file. </br>
* Write the character arrays.
</br>
</br>
The writeFile() function is responsible for actually writing the file. </br>
* Takes in the file name, the character array holding the file name and number of sectors as parameters. </br>
* The map and directory sectors are loaded into buffers. </br>
* Search free directory entry - it should begin with 0x00. </br>
* For each sector: </br>
  * Search a free sector by searching through the Map for a 0x00. </br>
  * It is important to not overwrite the bootloader after entry 4. </br>
  * The remaining bytes are now filed in the directory entry to 0x00. </br>
  * Now it's the time to finally write the Map and Directory sectors back to the disk. </br>

