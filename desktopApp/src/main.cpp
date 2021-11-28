#include <bits/stdc++.h>
#include "serialib/lib/serialib.h"

struct ReadHeader {
    uint32_t dataSize;
} __attribute__((packed));

typedef uint8_t byte;

ReadHeader readHeader(serialib& serial) {
    // Request data from the board
    serial.writeString("READ");

    // Create a buffer
    unsigned char buffer[sizeof(ReadHeader)];
    size_t headerBytesRead = 0;

    // Read the header
    for(;;) {
        if (headerBytesRead == sizeof(ReadHeader)) break;
        headerBytesRead += serial.readBytes(buffer + headerBytesRead, sizeof(ReadHeader) - headerBytesRead);
    }

    // Now we read the data
    return *((ReadHeader*)buffer);
}

std::vector<byte> readBody(ReadHeader header, serialib& serial) {
    size_t bodyBytesRead = 0;
    auto body = new byte[header.dataSize];

    while (bodyBytesRead < header.dataSize) {
        bodyBytesRead += serial.readBytes(body + bodyBytesRead, header.dataSize - bodyBytesRead);
    }

    std::vector<byte> data{ body, body + header.dataSize };

    delete[] body;

    return data;
}

void hexDump(std::vector<byte>& body) {
    printf("\t00\t01\t02\t03\t04\t05\t06\t07\t08\t09\t0A\t0B\t0C\t0D\t0E\t0F\n");
    for(unsigned i=0; i < body.size(); i += 16) {
        printf("%X\t%2X\t%X\t%X\t%X\t%X\t%X\t%X\t%X\t%X\t%X\t%X\t%X\t%X\t%X\t%X\t%X\n",
            i,
            body[i],
            body[i+1],
            body[i+2],
            body[i+3],
            body[i+4],
            body[i+5],
            body[i+6],
            body[i+7],
            body[i+8],
            body[i+9],
            body[i+10],
            body[i+11],
            body[i+12],
            body[i+13],
            body[i+14],
            body[i+15]
        );
    }
}

int main(int argc, char const *argv[]) {
    serialib serial;

    serial.openDevice("/dev/ttyACM0", 115200);

    std::this_thread::sleep_for(std::chrono::seconds(1));

    auto header = readHeader(serial);
    auto body = readBody(header, serial);

    // Destroy the serial object
    serial.~serialib();

    hexDump(body);

    return 0;
}
