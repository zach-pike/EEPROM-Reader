#ifndef byte
typedef uint8_t byte;
#endif

enum class RequestType {
    READALL,
    WRITE
};

struct RequestHeader {
    RequestType option;
} __attribute__((packed));

struct ReadallResponseHeader {
    uint32_t bytesRead;
} __attribute__((packed));