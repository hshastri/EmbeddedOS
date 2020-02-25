#ifndef FILELOADER_H
#define FILELOADER_H

#include <EEPROM.h>
#include <Arduino.h>
#include "SDCARD.h"

//preferences
const bool allowTargetToRun = true; // if true, programming lines are freed when not programming
const unsigned int ENTER_PROGRAMMING_ATTEMPTS = 2;
const byte BB_DELAY_MICROSECONDS = 6;
const unsigned long NO_PAGE = 0xFFFFFFFF;
const int MAX_FILENAME = 13;
const unsigned long kb = 1024;
const byte NO_FUSE = 0xFF;

//fuse types
enum {
    lowFuse,
    highFuse,
    extFuse,
    lockByte,
    calibrationByte
};

//programming commands
enum {
    progamEnable = 0xAC,

        // writes are preceded by progamEnable
        chipErase = 0x80,
        writeLockByte = 0xE0,
        writeLowFuseByte = 0xA0,
        writeHighFuseByte = 0xA8,
        writeExtendedFuseByte = 0xA4,

        pollReady = 0xF0,

        programAcknowledge = 0x53,

        readSignatureByte = 0x30,
        readCalibrationByte = 0x38,

        readLowFuseByte = 0x50, readLowFuseByteArg2 = 0x00,
        readExtendedFuseByte = 0x50, readExtendedFuseByteArg2 = 0x08,
        readHighFuseByte = 0x58, readHighFuseByteArg2 = 0x08,
        readLockByte = 0x58, readLockByteArg2 = 0x00,

        readProgramMemory = 0x20,
        writeProgramMemory = 0x4C,
        loadExtendedAddressByte = 0x4D,
        loadProgramMemory = 0x40,

};

//records in hex file
enum {
    hexDataRecord, // 00
    hexEndOfFile, // 01
    hexExtendedSegmentAddressRecord, // 02
    hexStartSegmentAddressRecord, // 03
    hexExtendedLinearAddressRecord, // 04
    hexStartLinearAddressRecord // 05
};
//chip specifics 
struct signatureType{
    signatureType() {}
    byte sig[3] = {0x1E, 0x95, 0x0F};
    const char * desc ="ATmega328P";
    unsigned long flashSize =32 * kb;
    unsigned int baseBootSize = 512;
    unsigned long pageSize = 128; // bytes
    byte fuseWithBootloaderSize = highFuse; // ie. one of: lowFuse, highFuse, extFuse
    byte timedWrites = false; // if pollUntilReady won't work by polling the chip
}; 
const byte fuseCommands[4] = {
    writeLowFuseByte,
    writeHighFuseByte,
    writeExtendedFuseByte,
    writeLockByte
};
//action types
enum {
    checkFile,
    verifyFlash,
    writeToFlash,
};

class FileLoader {
    private:
        //pins to use
        const byte MSPIM_SCK = 4; // port D bit 4
        const byte MSPIM_SS = 5; // port D bit 5
        const byte BB_MISO = 6; // port D bit 6
        const byte BB_MOSI = 7; // port D bit 7

        // which switch to close to start programming the target chip
        const byte startSwitch = 2;

        //fixed file to install
        boolean file = true;
        const char name[13] = "test.hex";

        // the three "status" LEDs
        const int errorLED = A0;
        const int readyLED = A1;
        const int workingLED = A2;

        unsigned char buff[512];
        char line[45];
        long sector = 580;
        bool stopped = false;
        int i = 0;

        //uhhh
        #define BB_MISO_PORT PIND
        #define BB_MOSI_PORT PORTD
        #define BB_SCK_PORT PORTD
        const byte BB_SCK_BIT = 4;
        const byte BB_MISO_BIT = 6;
        const byte BB_MOSI_BIT = 7;
        const byte RESET = MSPIM_SS;
        const uint8_t chipSelect = SS;
        //declare variables 
       //SdFat sd;
       SDCARDclass sd;
        byte fuses[5];

        unsigned int errors;
        int foundSig = -1;
        byte lastAddressMSB = 0;
        const signatureType currentSignature;
        bool gotEndOfFile;
        unsigned long pagesize;
        unsigned long pagemask;
        unsigned long oldPage;
        unsigned int progressBarCount;

        unsigned long extendedAddress;

        unsigned long lowestAddress;
        unsigned long highestAddress;
        unsigned long bytesWritten;
        unsigned int lineCount;
    public:
        FileLoader();
        void writeData(const unsigned long addr,const byte * pData,const int length);
        byte program(const byte b1, const byte b2 = 0, const byte b3 = 0, const byte b4 = 0); 		
        bool processLine(const char * pLine,const byte action);
        bool readHexFile(const char * fName,const byte action,char* buffer);
        void writeFuse(const byte newValue, const byte instruction);
        bool updateFuses(const bool writeIt);
        bool startProgramming();
        bool writeFlashContents(char* buffer);
        void getFuseBytes();
        void getSignature();
        byte readFlash(unsigned long addr);
        void writeFlash(unsigned long addr, const byte data);
        bool hexConv(const char * ( & pStr), byte & b);
        void pollUntilReady();
        void clearPage();
        void commitPage(unsigned long addr);
        byte SPITransfer(byte c);
        void stopProgramming();


};

#endif
