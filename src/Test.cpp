#include "Test.h"
#include "globals.h"
#include "KeyManager.h"



void testEnc() {
    uint8_t packet[100];
    for (int i = 0; i < 100; i++) packet[i] = i;

    CryptoClass cc;
    size_t envsize = cc.calcEnvelopSize(100);
    Serial.println("TESTING");
    Serial.printf("envsize %d\n", envsize);
    uint8_t *envelop = new uint8_t[envsize];
    size_t envsize2 = cc.createEnvelop(envelop, packet, 100);
    Serial.printf("envsize2 %d\n", envsize);

    CryptoClass cc2;
    size_t recpacketSize = cc.calcMaxDecryptedSize(envsize2);
    Serial.printf("recpacketSize %d\n", recpacketSize);
    uint8_t *recpacket = new uint8_t[recpacketSize];
    size_t recpacketSize2 = cc.openEnvelop(envelop, envsize2, recpacket, recpacketSize);
    Serial.printf("recpacketSize2 %d\n", recpacketSize2);
    if (memcmp(packet, recpacket, 100)==0) {
        Serial.println("success");
    } else {
        Serial.println("fail");
        printBlockLn(recpacket, recpacketSize2);
    }
    delete [] envelop;
    delete [] recpacket;
}