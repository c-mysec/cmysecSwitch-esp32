#ifndef KEYMANAGER_H_
#define KEYMANAGER_H_
#include <Arduino.h>
#include "Crypto.h"

#define HMAC_KEY_LENGTH 16
#define AES_KEY_LENGTH 16
#define BUF_KEY_LENGTH 32

void makeKeys(const char * username, const char * passwd);
extern uint8_t* keyEncrypt;
extern uint8_t* keyHmac;
extern uint8_t iv[AES_KEY_LENGTH];


class CryptoClass {
private:
    AES aes;
public:
    CryptoClass();
    size_t calcEnvelopSize(size_t packetSize);
    size_t createEnvelop(uint8_t* envelop, const uint8_t* packet, size_t packetSize);
    size_t calcMaxDecryptedSize(size_t envelopSize);
    size_t openEnvelop(const uint8_t* envelop, size_t envelopSize, uint8_t* packet, size_t packetSize);
};

#endif /* KEYMANAGER_H_ */
