#include "KeyManager.h"
#include "globals.h"

uint8_t keyHash[SHA256_SIZE];
uint8_t* keyEncrypt;
uint8_t* keyHmac;
uint8_t iv[AES_KEY_LENGTH];

SHA256 sha256;


void makeKeys(const char * username, const char * passwd) {
	SHA256 sha256;
	uint8_t key[BUF_KEY_LENGTH];
	uint8_t key2[BUF_KEY_LENGTH] = { 0x1C,0x3E,0x4B,0xAF,0x13,0x4A,0x89,0xC3,0xF3,0x87,0x4F,0xBC,0xD7,0xF3, 0x31, 0x31,
            0x1C,0x3E,0x4B,0xAF,0x13,0x4A,0x89,0xC3,0xF3,0x87,0x4F,0xBC,0xD7,0xF3, 0x31, 0x31 };
	// get SHA-256 hash of our secret key to create 256 bits of "key material"
	memcpy(key, key2, BUF_KEY_LENGTH);
	size_t len = strlen(username);
	len = len > MAX_NAME_LEN ? MAX_NAME_LEN : len;
	strncpy((char *)key, username, len);
	len = strlen(passwd);
	len = len > MAX_NAME_LEN ? MAX_NAME_LEN : len;
	strncpy(((char *)key) + MAX_NAME_LEN, passwd, len);
	sha256.doUpdate(key, BUF_KEY_LENGTH);
	Serial.print(F("passkey"));
	printBlock(key, BUF_KEY_LENGTH);
	sha256.doFinal(keyHash);
	Serial.print(("keyHash"));
	printBlock(keyHash, AES_KEY_LENGTH * 2);
	// keyEncrypt is a pointer pointing to the first 128 bits bits of "key material" stored in keyHash
	// keyHmac is a pointer poinging to the second 128 bits of "key material" stored in keyHashMAC
	keyEncrypt = keyHash;
	keyHmac = keyHash + AES_KEY_LENGTH;
	RNG::fill(iv, AES_KEY_LENGTH);
}

CryptoClass::CryptoClass() : aes(keyEncrypt, iv, AES::AES_MODE_128, AES::CIPHER_ENCRYPT)
{
}
size_t CryptoClass::calcEnvelopSize(size_t packetSize) {
	int encryptedSize = aes.calcSizeAndPad(packetSize);
	int ivEncryptedSize = encryptedSize + AES_KEY_LENGTH;
	size_t ivEncryptedHmacSize = ivEncryptedSize + SHA256HMAC_SIZE;
	return ivEncryptedHmacSize;
}
size_t CryptoClass::calcMaxDecryptedSize(size_t envelopSize) {
	return envelopSize - AES_KEY_LENGTH - SHA256HMAC_SIZE;
}
size_t CryptoClass::createEnvelop(uint8_t* envelop, const uint8_t* packet, size_t packetSize) {
	int encryptedSize = aes.calcSizeAndPad(packetSize);
	int ivEncryptedSize = encryptedSize + AES_KEY_LENGTH;
	int ivEncryptedHmacSize = ivEncryptedSize + SHA256HMAC_SIZE;
	uint8_t *ivEncryptedHmac = envelop;
	// copy IV to our final message buffer
	memcpy(ivEncryptedHmac, iv, AES_KEY_LENGTH);
	println(F("Encrypted (bytes)"),encryptedSize);

	// encrypted is a pointer that points to the encypted messages position in our final message buffer
	uint8_t* encrypted = ivEncryptedHmac + AES_KEY_LENGTH;
	Serial.print(F("Packet content: "));
	// AES 128 CBC and pkcs7 padding
	aes.process((uint8_t*)packet, encrypted, packetSize);
	printBlockLn((uint8_t*)packet, packetSize);

	// computedHmac is a pointer which points to the HMAC position in our final message buffer
	uint8_t* computedHmac = encrypted + encryptedSize;

	// compute HMAC/SHA-256 with keyHmac
	SHA256HMAC hmac(keyHmac, HMAC_KEY_LENGTH);
	hmac.doUpdate(ivEncryptedHmac, ivEncryptedSize);
	hmac.doFinal(computedHmac);

	Serial.print(F("Computed HMAC ")); Serial.println(SHA256HMAC_SIZE);
	printBlock(computedHmac, SHA256HMAC_SIZE);

	Serial.print(F("IV | encrypted | HMAC ")); Serial.println(ivEncryptedHmacSize);
	printBlockLn(ivEncryptedHmac, ivEncryptedHmacSize);
	return ivEncryptedHmacSize;
}

size_t CryptoClass::openEnvelop(const uint8_t* envelop, size_t envelopSize, uint8_t* packet, size_t packetSize) {
	// receivedHmac is a pointer which points to the received HMAC in our decoded message
	uint8_t* receivedHmac = ((uint8_t*)envelop) + envelopSize - SHA256HMAC_SIZE;
	// compute HMAC/SHA-256 with keyHmac
	uint8_t remote_computedHmac[SHA256HMAC_SIZE];
	SHA256HMAC remote_hmac(keyHmac, HMAC_KEY_LENGTH);
	remote_hmac.doUpdate(envelop, envelopSize - SHA256HMAC_SIZE);
	remote_hmac.doFinal(remote_computedHmac);

	println(F("Computed HMAC (bytes):"), SHA256HMAC_SIZE);
	printBlock(remote_computedHmac, SHA256HMAC_SIZE);
	if (memcmp(receivedHmac, remote_computedHmac, SHA256HMAC_SIZE) == 0) {
		uint8_t iv2[AES_KEY_LENGTH];
		memcpy(iv2, envelop, AES_KEY_LENGTH);
		println(F("Received IV (bytes):"), AES_KEY_LENGTH);
		printBlock(iv2, AES_KEY_LENGTH);
		// decrypt
		int decryptedSize = envelopSize - AES_KEY_LENGTH - SHA256HMAC_SIZE;
		if (packetSize < decryptedSize) return 0;
		AES aesDecryptor(keyEncrypt, iv2, AES::AES_MODE_128, AES::CIPHER_DECRYPT);
		aesDecryptor.process((uint8_t*)envelop + AES_KEY_LENGTH, (uint8_t*)packet, decryptedSize);

		println(F("Decrypted Packet HEX (bytes)"), decryptedSize);
		printBlock((uint8_t*)packet, decryptedSize);
		return decryptedSize;
	}
	return 0;
}