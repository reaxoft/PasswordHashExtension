#include "stdafx.h"
#include "Hash.h"
#include <memory>
#include <winternl.h>
#include <bcrypt.h>


int sha256(unsigned char *data, int dataSize, unsigned char *hash) {
	NTSTATUS Status;
	BCRYPT_ALG_HANDLE AlgHandle = NULL;
	BCRYPT_HASH_HANDLE  HashHandle = NULL;

	Status = BCryptOpenAlgorithmProvider(&AlgHandle, BCRYPT_SHA256_ALGORITHM, NULL, BCRYPT_HASH_REUSABLE_FLAG);
	if (!NT_SUCCESS(Status))
	{
		goto cleanup;
	}
	Status = BCryptCreateHash(AlgHandle, &HashHandle, NULL, 0, NULL, 0, 0);
	if (!NT_SUCCESS(Status))
	{
		goto cleanup;
	}
	Status = BCryptHashData(HashHandle, data, dataSize, 0);
	if (!NT_SUCCESS(Status))
	{
		goto cleanup;
	}
	Status = BCryptFinishHash(HashHandle, hash, 32, 0);

cleanup:
	if (NULL != HashHandle)
	{
		BCryptDestroyHash(HashHandle);
	}

	if (NULL != AlgHandle)
	{
		BCryptCloseAlgorithmProvider(AlgHandle, 0);
	}

	return NT_SUCCESS(Status) ? 0 : -1;
}

unsigned char* _fromHex(const char *hex, unsigned char* dst, int *dataSize) {
	int hexLen = strlen(hex);
	if ((hexLen & 0x01) != 0)
		return NULL;
	int len = hexLen >> 1;

	if (len > *dataSize)
		return NULL;
	*dataSize = len;

	if (dst == NULL)
		return NULL;

	unsigned char *p = dst;
	for (int i = 0; i < hexLen; i++) {
		unsigned char val;
		if (hex[i] >= '0' && hex[i] <= '9') {
			val = hex[i] - '0';
		}
		else if (hex[i] >= 'a' && hex[i] <= 'f') {
			val = hex[i] - 'a';
		}
		else if (hex[i] >= 'A' && hex[i] <= 'F') {
			val = hex[i] - 'A';
		}
		else {
			return NULL;
		}
		if ((i & 0x01) == 0) {
			*p = (val << 4) & 0xF0;
		}
		else {
			*p |= val & 0x0F;
			p++;
		}
	}
	return dst;
}

const char hexTable[] = { '0','1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };

char* _toHex(char *dst, int dstLen, const unsigned char *src, int srcLen) {
	int len = srcLen << 1;
	if (dstLen < len + 1) {
		return NULL;
	}
	char *p = dst;
	for (int i = 0; i < srcLen; i++) {
		*p++ = hexTable[(src[i] >> 4) & 0x0F];
		*p++ = hexTable[src[i] & 0x0F];
	}
	*p = '\0';
	return dst;
}

char* asciiWhartToChar(const wchar_t* src, char* dst, int len) {
	for (int i = 0; i < len; i++) {
		dst[i] = (char)src[i + 1];
	}
	dst[len] = '\0';
	return dst;
}

std::vector<char> convertWchartToUtf8(const WCHAR* src)
{
	std::string convertedString;
	int requiredSize = WideCharToMultiByte(CP_UTF8, 0, src, -1, 0, 0, 0, 0);
	std::vector<char> buffer(requiredSize);
	if (requiredSize > 0)
	{
		WideCharToMultiByte(CP_UTF8, 0, src, -1, &buffer[0], requiredSize, 0, 0);
	}
	return buffer;
}

int CalcPasswordHash(wchar_t *dst, int dstLen, Platform::String ^_salt, WCHAR* psswd) {
	int saltLen = _salt->Length() - 2;
	std::unique_ptr<char> ascii(new char[saltLen + 1]);
	asciiWhartToChar(_salt->Data(), ascii.get(), saltLen);

	std::vector<char> password{ convertWchartToUtf8(psswd) };
	int passwordLen = password.size() - 1;

	int byteSaltLen = saltLen >> 1;
	std::unique_ptr<unsigned char> rawSalt(new unsigned char[byteSaltLen + passwordLen]);
	if (!_fromHex(ascii.get(), rawSalt.get(), &byteSaltLen)) {
		return -1;
	}
	memcpy(rawSalt.get() + byteSaltLen, &password[0], passwordLen);

	unsigned char byteHash[32];
	if (sha256(rawSalt.get(), byteSaltLen + passwordLen, byteHash)) {
		return -1;
	}

	char hash[65];
	if (!_toHex(hash, dstLen, byteHash, 32)) {
		return -1;
	}
	MultiByteToWideChar(CP_UTF8, 0, hash, -1, dst, dstLen);
	return 0;
}
