// Address port is A
// I/O port is C

#include <Arduino.h>
#include <avr/iom2560.h>

#define IODDR DDRC
#define IOPORT PORTC
#define IOPIN PINC

#define ADDRESSDDR DDRA
#define ADDRESSPORT PORTA

class CustomEEPROM {
	private:
		uint16_t size;

		// Enables the EEPROM
		uint8_t ChipEnable;

		// Enables writing
		uint8_t WriteEnable;

		// Enables the Output
		uint8_t OutputEnable;

		/*
			REMEMBER Pin flags are enabled when LOW, for sake of my brain, the setXEnable() functions invert the value given to them
			Read:
				ChipEnable LOW
				OutputEnable: LOW
				WriteEnable: HIGH

				Put a address on the Address Port ( by defualt this program uses port A )
				and the data at that address should be set on the I/O Port ( by default this program uses port C)
			
			Write:
				ChipEnable: HIGH
				OutputEnable: HIGH

				A low pulse on the WriteEnable initiates a byte write, the maximum time the pulse should be is 200 microseconds,
				the address is latched on the last falling edge of WriteEnable, the data is latched on the first rising edge

				Highest WriteEnable time is 1000ns or 1μs, and the smallest ammount we can delay is 1μs so we should be fine
		*/


		enum class DataDirection {
			OUT,
			IN
		};

		// Set the data direction of the I/O port
		void SetIODirection(DataDirection dd) {
			if (dd == DataDirection::IN)
				IODDR = 0x00;
			else if (dd == DataDirection::OUT)
				IODDR = 0xff;
		}

		// Set the CE pin on the EEPROM
		void setChipEnable(bool value) {
			digitalWrite(CustomEEPROM::ChipEnable, !value);
		}

		// Set the WE pin on the EEPROM
		void setWriteEnable(bool value) {
			digitalWrite(CustomEEPROM::WriteEnable, !value);
		}

		// Set the OE pin on the EEPROM
		void setOutputEnable(bool value) {
			digitalWrite(CustomEEPROM::OutputEnable, !value);
		}

		// Set all flag pins to their disabled values
		void resetAllFlagPins() {
			setChipEnable(false);
			setWriteEnable(false);
			setOutputEnable(false);
		}

		// Reset everything to do with the EEPROM
		void resetAll() {
			resetAllFlagPins();
			ADDRESSPORT = 0x00;

			// Reset the I/O port to 0
			SetIODirection(DataDirection::OUT);
			IOPORT = 0x00;
			SetIODirection(DataDirection::IN);
		}

		// Sets the chip to be able to read
		void prepareRead() {
			// Reset all state
			resetAll();

			// Set I/O direction to input
			SetIODirection(DataDirection::IN);

			// Set our enable pins to what they need to be to read
			setChipEnable(true);
			setOutputEnable(true);
		}

		// Set common things to prepare for write to EEPROM
		void prepareWrite() {
			// Reset all state
			resetAll();

			// Turn on the chip enable pin
			setChipEnable(true);

			// Set the I/O pins to be output
			SetIODirection(DataDirection::OUT);
		}
	public:
		CustomEEPROM(uint16_t size, uint8_t CE, uint8_t OE, uint8_t WE) {
			// Store size
			CustomEEPROM::size = size;

			// Set the pins
			WriteEnable = WE;
			OutputEnable = OE;
			ChipEnable = CE;
					
			// Configure flag pins
			pinMode(WriteEnable, OUTPUT);
			pinMode(OutputEnable, OUTPUT);
			pinMode(ChipEnable, OUTPUT);

			// Set address port to output 
			ADDRESSDDR = 0xff;

			// Reset all the Ports, pin flags, etc...
			resetAll();
		}

		// Write a byte to a memory address in the EEPROM
		// singleShot describes weather to optimise for sequential writes
		// when setting singleShot to false, The following need to be set beforehand
		// SetIODirection(DataDirection::OUT);, prepareWrite();
		// and resetAll() needs to be called after
		void writeByte(uint16_t addr, byte data, bool singleShot = true) {
			// Set some common things before writing
			if (singleShot) prepareWrite();

			// Send the data to the I/O port
			IOPORT = data;

			// Send the address to the address port
			ADDRESSPORT = addr;

			// Set write enable
			setWriteEnable(true);

			// Wait 1000 nanoseconds
			delayMicroseconds(1);

			// Reset write enable
			setWriteEnable(false);

			// Clear everything
			// Refer to previous if (singleShot) for explanation
			if (singleShot) resetAll();

			// We wait some to let the EEPROM stablize
			delay(10);
		}

		// Read a byte from a address
		// singleAction determines weather we should auto disable the chip after a read
		byte readByte(uint16_t address, bool singleAction = true) {
			// Enable chip & output
			if (singleAction) prepareRead();

			// Set address port
			PORTA = address;

			// Let set values stabilize
			delay(1);

			// Read value and set Address port to 0
			byte value = IOPIN;

			// Disable chip & output
			if (singleAction) resetAll();

			// Return the read value
			return value;
		}

		// Write an array of bytes to the EEPROM with a few additional parameters
		void writeBinary(byte data[], size_t sizeOfArray, uint16_t offset = 0x00, byte defaultByte = 0x00) {
			// Prepare a few things before we write
			prepareWrite();

			// Loop over entire EEPROM memory
			for (uint16_t i=offset; i < size; i++) {
				if (i < sizeOfArray)
					writeByte(i, data[i], false);
				else
					writeByte(i, defaultByte, false);
				}

			// Reset all common things
			resetAll();
		}

		void readBytes(void* dest, uint16_t address, size_t bytes) {
			prepareRead();

			for (unsigned i=address; i < address + bytes; i++) {
				((byte*)dest)[i] = readByte(i, false);
			}

			resetAll();
		}

		// Read a value of type T from address in EEPROM
		template <typename T>
		T readValue(unsigned int address) {
			// Allocate memory for our data
			byte* value = (byte*)malloc(sizeof(T));

			// We do this outside of the readByte() function to optimise read speeds
			prepareRead();

			// Read n ammout of bytes to the buffer
			for (uint16_t i = 0; i < sizeof(T); i++)
				value[i] = readByte(address + i, false);
					
			// Refer to previous refrences comment
			resetAll();

			// Dereference the pointer the the final value
			T finalVal = *((T*)value);

			// Free the buffer
			free((void*)value);

			return finalVal;
		}

		// Dump contents of eeprom
		void dumpEEPROM(uint16_t to = 0xff) {
			// Print the key
			Serial.println(":)\t00\t01\t02\t03\t04\t05\t06\t07\t08\t09\t0A\t0B\t0C\t0D\t0E\t0F");

			// Enable chip reading here to optimise
			prepareRead();

			// Format string
			const char* fmt = "%X\t%X\t%X\t%X\t%X\t%X\t%X\t%X\t%X\t%X\t%X\t%X\t%X\t%X\t%X\t%X\t%X";

			for (unsigned i=0; i < to; i += 16) {
				char buf[100];
				for (int i=0; i < sizeof(buf); i++) buf[i] = 0;

				// Format the data
				sprintf(buf, fmt,
					i,
					readByte(i, false),
					readByte(i+1, false),
					readByte(i+2, false),
					readByte(i+3, false),
					readByte(i+4, false),
					readByte(i+5, false),
					readByte(i+6, false),
					readByte(i+7, false),
					readByte(i+8, false),
					readByte(i+9, false),
					readByte(i+10, false),
					readByte(i+11, false),
					readByte(i+12, false),
					readByte(i+13, false),
					readByte(i+14, false),
					readByte(i+15, false)
				);
						
				// Print the formatted string
				Serial.println(buf);
			}

			// Reset all state
			resetAll();
		}
};

// Holds size of EEPROM
const unsigned int EEPROMSize = 0x3E80;

CustomEEPROM eeprom(EEPROMSize, 2, 3, 4);

struct ReadData {
    uint32_t dataSize;
} __attribute__((packed));

void setup() {
	Serial.begin(115200);

	// Serial.println("Reading uint16_t from address 0x00");
	// auto val = eeprom.readValue<uint16_t>(0x00);
	// Serial.println(val);
}


void loop() {
	// put your main code here, to run repeatedly:
	String input = Serial.readString();

	if (input.startsWith("READ")) {
		// Construct header
		struct ReadData header;

		// We want to read 256 bytes
		// aka address 0x00 - 0xFF
		header.dataSize = 256;

		// Buffer to hold data we are about to read
		byte buffer[header.dataSize];

		// Read data from EEPROM
		eeprom.readBytes(buffer, 0x00, header.dataSize);

		// Send header
		for (unsigned i=0; i < sizeof(ReadData); i++)
			Serial.write(((byte*)&header)[i]);

		// Send body
		for (unsigned i=0; i < sizeof(buffer); i++) 
			Serial.write(buffer[i]);

	} else if (input.startsWith("WRITE")) {
		// For now nothing
	}
}