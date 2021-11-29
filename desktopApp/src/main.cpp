#include <bits/stdc++.h>
#include "serialib/lib/serialib.h"

#include "../../common.h"

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

template <typename T>
void toByteArray(std::vector<byte>& array, T item) {
    for (unsigned i=0; i < sizeof(item); i++)
        array.push_back(((byte*)&item)[i]);
}

void sendByteArray(const std::vector<byte>& bytes, serialib& serial) {
    serial.writeBytes(bytes.data(), bytes.size()); 
}

int main(int argc, char const *argv[]) {
    serialib serial;

    serial.openDevice("/dev/ttyACM0", 115200);
    // serial.flushReceiver();

    std::this_thread::sleep_for(std::chrono::seconds(1));

    // Construct header
    struct RequestHeader header;

    // We must remember this for later
    header.option = RequestType::READALL;

    // Convert header to array
    std::vector<byte> headerBytes;
    toByteArray(headerBytes, header);

    // Send bytes
    sendByteArray(headerBytes, serial);

    // AAAHHH hoursWasted = 6
    std::this_thread::sleep_for(std::chrono::seconds(1));

    // Now we are waiting to recive enough bytes to fulfil the request
    switch(header.option) {
        case RequestType::READALL: {
            byte buffer[sizeof(ReadallResponseHeader)];

            // Recive all the bytes
            serial.readBytes(buffer, sizeof(ReadallResponseHeader));

            // Now we construct a header object from the buffer
            ReadallResponseHeader recvHeader = *((ReadallResponseHeader*)buffer);

            std::cout << recvHeader.bytesRead << std::endl;

            // Create a buffer to recive the body
            auto bodybuf = new byte[recvHeader.bytesRead];

            // Recive all the bytes
            serial.readBytes(bodybuf, recvHeader.bytesRead);

            // Create a body vector
            std::vector<byte> body{ bodybuf, bodybuf + recvHeader.bytesRead };

            // Delete the buffer
            delete[] bodybuf;

            // Now lets make a hexdump
            std::ofstream hexdump("dump.bin", std::ios::out | std::ios::binary);
            hexdump.write((const char *)body.data(), body.size());
            hexdump.close();

            // Hexdump
            hexDump(body);
        } break;
    }

    // Destroy the serial object
    serial.~serialib();

    return 0;
}
