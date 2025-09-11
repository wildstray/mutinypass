// OpenAuthentication Time-based One-time Password Algorithm (RFC 6238)
// For the complete description of the algorithm see
// http://tools.ietf.org/html/rfc4226#section-5.3
//
// Luca Dentella (http://www.lucadentella.it)

#include "TOTP.h"

// Init the library with the private key, its length and the timeStep duration
TOTP::TOTP(uint8_t* hmacKey, int keyLength, int timeStep, int digits, mbedtls_md_type_t algorithm) {

	_hmacKey = hmacKey;
	_keyLength = keyLength;
	_timeStep = timeStep;
	_digits = digits;
	_algorithm = algorithm;
};

// Init the library with the private key, its length and a time step of 30sec (default for Google Authenticator)
TOTP::TOTP(uint8_t* hmacKey, int keyLength) {

	_hmacKey = hmacKey;
	_keyLength = keyLength;
	_timeStep = 30;
	_digits = 6;
	_algorithm = MBEDTLS_MD_SHA1;
};

// Generate a code, using the timestamp provided
char* TOTP::getCode(long timeStamp) {
	
	long steps = timeStamp / _timeStep;
	return getCodeFromSteps(steps);
}

// Generate a code, using the number of steps provided
char* TOTP::getCodeFromSteps(long steps) {
	
	// STEP 0, map the number of steps in a 8-bytes array (counter value)
	_byteArray[0] = 0x00;
	_byteArray[1] = 0x00;
	_byteArray[2] = 0x00;
	_byteArray[3] = 0x00;
	_byteArray[4] = (int)((steps >> 24) & 0xFF);
	_byteArray[5] = (int)((steps >> 16) & 0xFF);
	_byteArray[6] = (int)((steps >> 8) & 0XFF);
	_byteArray[7] = (int)((steps & 0XFF));
	
	// STEP 1, get the HMAC-SHA hash from counter and key
  	mbedtls_md_context_t ctx;
  	mbedtls_md_type_t md_type = _algorithm;
  	mbedtls_md_init(&ctx);
  	mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(md_type), 1);
  	mbedtls_md_hmac_starts(&ctx, _hmacKey, _keyLength);
  	mbedtls_md_hmac_update(&ctx, (const unsigned char *) _byteArray, sizeof(_byteArray));
  	mbedtls_md_hmac_finish(&ctx, _hash);
  	mbedtls_md_free(&ctx);

	// STEP 2, apply dynamic truncation to obtain a 4-bytes string
	_offset = _hash[20 - 1] & 0xF; 
	_truncatedHash = 0;
	for (int j = 0; j < 4; ++j) {
		_truncatedHash <<= 8;
		_truncatedHash  |= _hash[_offset + j];
	}

	// STEP 3, compute the OTP value
	uint64_t _digits_power[] = {1, 10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000, 1000000000, 10000000000};
	_truncatedHash &= 0x7FFFFFFF;
	_truncatedHash %= _digits_power[_digits];

  	memset(_code, 0, sizeof(_code));
	// convert the value in string, with heading zeroes
  	snprintf(_code, sizeof(_code), "%0*ld", _digits, _truncatedHash);
	return _code;
}
