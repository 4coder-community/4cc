#pragma once
#ifndef _BASE64_H_
#define _BASE64_H_


#define _HAS_EXCEPTIONS (0)
#define _CRT_SECURE_NO_WARNINGS

#include <cstdlib>


/**
 * encode three bytes using base64 (RFC 3548)
 *
 * @param triple three bytes that should be encoded
 * @param result buffer of four characters where the result is stored
 */
void _base64_encode_triple(unsigned char triple[3], char result[4]);

/**
 * encode an array of bytes using Base64 (RFC 3548)
 *
 * @param source the source buffer
 * @param sourcelen the length of the source buffer
 * @param target the target buffer
 * @param targetlen the length of the target buffer
 * @return 1 on success, 0 otherwise
 */
int base64_encode(unsigned char *source, size_t sourcelen, char *target, size_t targetlen);

/**
 * determine the value of a base64 encoding character
 *
 * @param base64char the character of which the value is searched
 * @return the value in case of success (0-63), -1 on failure
 */
int _base64_char_value(char base64char);

/**
 * decode a 4 char base64 encoded byte triple
 *
 * @param quadruple the 4 characters that should be decoded
 * @param result the decoded data
 * @return length of the result (1, 2 or 3), 0 on failure
 */
int _base64_decode_triple(char quadruple[4], unsigned char *result);

/**
 * decode base64 encoded data
 *
 * @param source the encoded data (zero terminated)
 * @param target pointer to the target buffer
 * @param targetlen length of the target buffer
 * @return length of converted data on success, -1 otherwise
 */
size_t base64_decode(const char *source, unsigned char *target, size_t targetlen);

/** get memory consumption */
size_t base64_needed(const size_t &iSize);


#endif