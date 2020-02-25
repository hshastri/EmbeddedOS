
//includes
#include "FileLoader.h"
#include "SDCARD.h"

#define NUMITEMS(arg)((unsigned int)(sizeof(arg) / sizeof(arg[0])))

    FileLoader::FileLoader() {
        //while (!sd.begin (chipSelect, SPI_HALF_SPEED)){
        //delay (1000);
        //}
    
    }
    void FileLoader::writeData(const unsigned long addr,const byte * pData,const int length) {
        // write each byte
        for (int i = 0; i < length; i++) {
            unsigned long thisPage = (addr + i) & pagemask;
            // page changed? commit old one
            if (thisPage != oldPage && oldPage != NO_PAGE)
                commitPage(oldPage);
            // now this is the current page
            oldPage = thisPage;
            // put byte into work buffer
            writeFlash(addr + i, pData[i]);
        } // end of for

    }
    byte FileLoader::program(const byte b1,const byte b2 = 0, const byte b3 = 0,const byte b4 = 0) {
        noInterrupts();
        SPITransfer(b1);
        SPITransfer(b2);
        SPITransfer(b3);
        byte b = SPITransfer(b4);
        interrupts();
        return b;
    }		
    bool FileLoader::processLine(const char * pLine,const byte action) {
        if ( * pLine++ != ':') {
            return true; // error
        }

        const int maxHexData = 40;
        byte hexBuffer[maxHexData];
        int bytesInLine = 0;

        if (action == checkFile) {
            if (lineCount++ % 40 == 0) {}
        }

                // convert entire line from ASCII into binary
        while (isxdigit( * pLine)) {
                    // can't fit?
            if (bytesInLine >= maxHexData) {
                return true;
            } // end if too long

            if (hexConv(pLine, hexBuffer[bytesInLine++])) {
                return true;
            }
        } // end of while

        if (bytesInLine < 5) {
            return true;
        }

        // sumcheck it

        byte sumCheck = 0;
        for (int i = 0; i < (bytesInLine - 1); i++)
            sumCheck += hexBuffer[i];

        // 2's complement
        sumCheck = ~sumCheck + 1;

        // check sumcheck
        if (sumCheck != hexBuffer[bytesInLine - 1]) {
            return true;
        }

        // length of data (eg. how much to write to memory)
        byte len = hexBuffer[0];

        // the data length should be the number of bytes, less
        //   length / address (2) / transaction type / sumcheck
        if (len != (bytesInLine - 5)) {
            return true;
        }

        // two bytes of address
        unsigned long addrH = hexBuffer[1];
        unsigned long addrL = hexBuffer[2];

        unsigned long addr = addrL | (addrH << 8);

        byte recType = hexBuffer[3];

        switch (recType) {
            // stuff to be written to memory
        case hexDataRecord:
            lowestAddress = min(lowestAddress, addr + extendedAddress);
            highestAddress = max(lowestAddress, addr + extendedAddress + len - 1);
            bytesWritten += len;

            switch (action) {
            case checkFile: // nothing much to do, we do the checks anyway
                break;
            case writeToFlash:
                writeData(addr + extendedAddress, & hexBuffer[4], len);
                break;
            } // end of switch on action
            break;

            // end of data
        case hexEndOfFile:
            gotEndOfFile = true;
            break;

            // we are setting the high-order byte of the address
        case hexExtendedSegmentAddressRecord:
            extendedAddress = ((unsigned long) hexBuffer[4]) << 12;
            break;

            // ignore these, who cares?
        case hexStartSegmentAddressRecord:
        case hexExtendedLinearAddressRecord:
        case hexStartLinearAddressRecord:
            break;

        default:
            return true;
        } // end of switch on recType

        return false;
    } // end of processLine
    bool FileLoader::readHexFile(const char * fName,const byte action, char* bufferTwo) {
        //const int maxLine = 80;
        //char buffer[maxLine];
        //ifstream sdin(name);
        int lineNumber = 0;
        gotEndOfFile = false;
        extendedAddress = 0;
        errors = 0;
        lowestAddress = 0xFFFFFFFF;
        highestAddress = 0;
        bytesWritten = 0;
        progressBarCount = 0;

        pagesize = currentSignature.pageSize;
        pagemask = ~(pagesize - 1);
        oldPage = NO_PAGE;
        
        // check for open error
        //if (!sdin.is_open()) {
         //   return true;
       // }
        
        switch (action) {
        case checkFile:
            break;

        case verifyFlash:
            break;

        case writeToFlash:
            program(progamEnable, chipErase); // erase it
            delay(20); // for Atmega8
            pollUntilReady();
            clearPage(); // clear temporary page
            break;
        } // end of switch
        
  line[43] = 0x0D;
  line[44] = 0x0A;
  lineCount = 0;
  while (sector < 586) {
    SDCARD.readblock(sector,53,buff);
    for (i = 0; i < 512; i++) {
        
      if (buff[i] == 0x3A) {
        lineCount = 0;

        //Serial.println(line);
        //Serial.print("F");
        processLine(line, action);
        //if (processLine(line, action)) {
          //return true; // error
        //}
        
        for (int v = 0; v < 45; v++) {
          line[v] = 0;
        }
        
        
      }
      
      if (buff[i] == 0x51) {
        stopped = true;
       // Serial.println(line);
        if (processLine(line, action)) {
         //return true; // error
        }
        break;
      }
      line[lineCount] = buff[i];

      lineCount++;
      
    }

    sector++;
    if (stopped) {
      break;
    }

  }

      //  while (sdin.getline(buffer, maxLine)) {
           
            // lineNumber++;
            // int count = sdin.gcount();
            
            // if (sdin.fail()) {
                
            //     return true;
                
            // } // end of fail (line too long?)
            //Serial.println(buffer);
            // ignore empty lines
            //if (count > 1) {
              //  if (processLine(buffer, action)) {
                //    Serial.print("ERROR");
                  //  return true; // error
              //  }
           // }
       // } // end of while each line
       // if (!gotEndOfFile) {
            
         //   return true;
        //}

        switch (action) {
        case writeToFlash:
            // commit final page
            if (oldPage != NO_PAGE)
                commitPage(oldPage);
            break;

        case verifyFlash:
            if (errors > 0) {
                return true;
            } // end if
            break;

        case checkFile:
            break;
        } // end of switch

        return false;
    } 
    void FileLoader::writeFuse(const byte newValue,const byte instruction) {
        if (newValue == 0)
            return; // ignore

        program(progamEnable, instruction, 0, newValue);
        pollUntilReady();
    }
    bool FileLoader::updateFuses(const bool writeIt) {
        unsigned long addr;
        unsigned int len;

        byte fusenumber = currentSignature.fuseWithBootloaderSize;

        // if no fuse, can't change it
        if (fusenumber == NO_FUSE) {
            //    ShowMessage (MSG_NO_BOOTLOADER_FUSE);   // maybe this doesn't matter?
            return false; // ok return
        }

        addr = currentSignature.flashSize;
        len = currentSignature.baseBootSize;

        if (lowestAddress == 0) {
            // don't use bootloader  
            fuses[fusenumber] |= 1;
        } else {
            byte newval = 0xFF;

            if (lowestAddress == (addr - len))
                newval = 3;
            else if (lowestAddress == (addr - len * 2))
                newval = 2;
            else if (lowestAddress == (addr - len * 4))
                newval = 1;
            else if (lowestAddress == (addr - len * 8))
                newval = 0;
            else {
                return true;
            }

            if (newval != 0xFF) {
                newval <<= 1;
                fuses[fusenumber] &= ~0x07; // also program (clear) "boot into bootloader" bit 
                fuses[fusenumber] |= newval;
            } // if valid

        } // if not address 0

        if (writeIt) {
            writeFuse(fuses[fusenumber], fuseCommands[fusenumber]);
        }

        return false;
    }
    bool FileLoader::startProgramming() {
        
        byte confirm;
        pinMode(RESET, OUTPUT);
        digitalWrite(MSPIM_SCK, LOW);
        pinMode(MSPIM_SCK, OUTPUT);
        pinMode(BB_MOSI, OUTPUT);
        unsigned int timeout = 0;

        // we are in sync if we get back programAcknowledge on the third byte
        do {
            // regrouping pause
            delay(100);

            // ensure SCK low
            noInterrupts();
            digitalWrite(MSPIM_SCK, LOW);
            // then pulse reset, see page 309 of datasheet
            digitalWrite(RESET, HIGH);
            delayMicroseconds(10); // pulse for at least 2 clock cycles
            digitalWrite(RESET, LOW);
            interrupts();

            delay(25); // wait at least 20 mS
            noInterrupts();
            SPITransfer(progamEnable);
            SPITransfer(programAcknowledge);
            confirm = SPITransfer(0);
            SPITransfer(0);
            interrupts();

            if (confirm != programAcknowledge) {
                if (timeout++ >= ENTER_PROGRAMMING_ATTEMPTS)
                    return false;
            } // end of not entered programming mode

        } while (confirm != programAcknowledge);
        
        return true; // entered programming mode OK
    }
    bool FileLoader::writeFlashContents(char buffer[4096]) {
        
        errors = 0;


        //  if (chooseInputFile ())
        //   return false;  

        // ensure back in programming mode  
        if (!startProgramming())
            return false;
        
        // now commit to flash
        if (readHexFile(name, writeToFlash,buffer))
            return false;
        // turn ready LED on during verification
        digitalWrite(readyLED, HIGH);
        // verify
        if (readHexFile(name, verifyFlash,buffer))
            return false;

        // now fix up fuses so we can boot    
        if (errors == 0)
            updateFuses(true);

        return errors == 0;
    }

    void FileLoader::getFuseBytes() {
        fuses[lowFuse] = program(readLowFuseByte, readLowFuseByteArg2);
        fuses[highFuse] = program(readHighFuseByte, readHighFuseByteArg2);
        fuses[extFuse] = program(readExtendedFuseByte, readExtendedFuseByteArg2);
        fuses[lockByte] = program(readLockByte, readLockByteArg2);
        fuses[calibrationByte] = program(readCalibrationByte);
    } // end of getFuseBytes

    void FileLoader::getSignature() {
        foundSig = 1;
        lastAddressMSB = 0;

        byte sig[3];
        for (byte i = 0; i < 3; i++) {
            sig[i] = program(readSignatureByte, 0, i);
        }

        program(loadExtendedAddressByte, 0, 0);

    }

    byte FileLoader::readFlash(unsigned long addr) {
        byte high = (addr & 1) ? 0x08 : 0; // set if high byte wanted
        addr >>= 1; // turn into word address

        // set the extended (most significant) address byte if necessary
        byte MSB = (addr >> 16) & 0xFF;
        if (MSB != lastAddressMSB) {
            program(loadExtendedAddressByte, 0, MSB);
            lastAddressMSB = MSB;
        }

        return program(readProgramMemory | high, highByte(addr), lowByte(addr));
    }
    void FileLoader::writeFlash(unsigned long addr,const byte data) {
        byte high = (addr & 1) ? 0x08 : 0; // set if high byte wanted
        addr >>= 1; // turn into word address
        program(loadProgramMemory | high, 0, lowByte(addr), data);
    };
    bool FileLoader::hexConv(const char * ( & pStr), byte & b) {
        if (!isxdigit(pStr[0]) || !isxdigit(pStr[1])) {
            return true;
        }
        b = * pStr++ - '0';
        if (b > 9)
            b -= 7;

        b <<= 4;

        byte b1 = * pStr++ - '0';
        if (b1 > 9)
            b1 -= 7;

        b |= b1;

        return false;
    }
    void FileLoader::pollUntilReady() {
        if (currentSignature.timedWrites)
            delay(10); // at least 2 x WD_FLASH which is 4.5 mS
        else {
            while ((program(pollReady) & 1) == 1) {}
        }
    }
    void FileLoader::clearPage() {
        unsigned int len = currentSignature.pageSize;
        for (unsigned int i = 0; i < len; i++)
            writeFlash(i, 0xFF);
    }
    void FileLoader::commitPage(unsigned long addr) {
        addr >>= 1; // turn into word address

        // set the extended (most significant) address byte if necessary
        byte MSB = (addr >> 16) & 0xFF;
        if (MSB != lastAddressMSB) {
            program(loadExtendedAddressByte, 0, MSB);
            lastAddressMSB = MSB;
        } // end if different MSB

        program(writeProgramMemory, highByte(addr), lowByte(addr));
        pollUntilReady();

        clearPage(); // clear ready for next page full
    }
    byte FileLoader::SPITransfer(byte c) {
        byte bit;
        for (bit = 0; bit < 8; bit++) {
            // write MOSI on falling edge of previous clock
            if (c & 0x80)
                BB_MOSI_PORT |= bit(BB_MOSI_BIT);
            else
                BB_MOSI_PORT &= ~bit(BB_MOSI_BIT);
            c <<= 1;

            // read MISO
            c |= (BB_MISO_PORT & bit(BB_MISO_BIT)) != 0;

            // clock high
            BB_SCK_PORT |= bit(BB_SCK_BIT);

            // delay between rise and fall of clock
            delayMicroseconds(BB_DELAY_MICROSECONDS);

            // clock low
            BB_SCK_PORT &= ~bit(BB_SCK_BIT);

            // delay between rise and fall of clock
            delayMicroseconds(BB_DELAY_MICROSECONDS);
        }

        return c;
    }
    void FileLoader::stopProgramming() {
        // turn off pull-ups
        digitalWrite(RESET, LOW);
        digitalWrite(MSPIM_SCK, LOW);
        digitalWrite(BB_MOSI, LOW);
        digitalWrite(BB_MISO, LOW);

        // set everything back to inputs
        pinMode(RESET, INPUT);
        pinMode(MSPIM_SCK, INPUT);
        pinMode(BB_MOSI, INPUT);
        pinMode(BB_MISO, INPUT);
    }