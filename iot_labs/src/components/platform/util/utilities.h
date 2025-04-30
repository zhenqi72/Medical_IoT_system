#ifndef __UTILITIES_H__
#define __UTILITIES_H__

#include <ctype.h>
#include "labs_defines.h"
/**
 * @brief Find substring in string
 * @param s1 String to search
 * @param s2 Substring to search for
 * @return Pointer to the substring in the string
 */
char* strrstr(const char* s1, const char* s2);
/**
 * @brief Find a char in a string
 * @param s1 String to search
 * @param c Character to search for
 * @param n Size of the string to search
 * @return Pointer to the character in the string
 */
char* strnchr(const char* s1, char c, size_t n);
/**
 * @brief Remove character from end of string
 * @param s String to remove character from
 * @param len Length of the string
 * @param c Character to remove
 * @return Number of characters removed
 */
int str_rmtail(char *s, int len, char c);
/**
 * @brief Turns a string number into an unsigned 32 bit number
 * @param c Pointer to string
 * @param n Size of the string
 * @return The numerical value of the string ("14" -> 14)
 */
uint32_t plt_atoun(const char* c, size_t n);
/**
 * @brief Turns a string into a double
 * @param c Pointer to string
 * @param n Size of the string
 * @return The numerical value of the string
 */
double plt_atofn(const char* c, size_t n);
/**
 * @brief Turns a hex string into a byte array
 * @param in Pointer to string to convert
 * @param in_size Length of string to convert
 * @param out Pointer to array to store converted data to
 * @param out_size Size of output array
 * @param in_len Optional difference between input size and actual length of input buffer
 * @return Amount of data inserted into the output buffer
 */
size_t  plt_axtobuf(const char* in, size_t in_size, uint8_t* out, size_t out_size, size_t *in_len);
/**
 * @brief Turns a number represented as a string to an unsigned 64 bit number
 * @param c Pointer to the string to convert
 * @param n Size of the string
 * @return The numerical value of the string
 */
uint64_t plt_atou64n(const char* c, size_t n);
/**
 * @brief Print the data as a hex string
 * @param buf Buffer to store the formatted string to
 * @param buf_size Size available in the buffer
 * @param data Pointer to the data to format
 * @param len Length of the data
 * @param style Style to store the string as
 * @return The numerical value of the string
 */
int  snprintf_hex(char *buf, size_t buf_size, const uint8_t *data, size_t len, char style);
/**
 * @brief String to long long integer (64 bit signed)
 * @param c String to convert to number
 * @param n Size of the string
 * @return The numerical value of the string
 */
int64_t plt_atos64n(const char* c, size_t n);
/**
 * @brief String to integer (32 bit signed)
 * @param c String to convert to number
 * @param n Size of the string
 * @return The numerical value of the string
 */
int32_t plt_atoin(const char * c, size_t n);
/**
 * @brief Unsigned 64bit number to string
 * @param data Number to convert to string
 * @param c Pointer to buffer to put string in
 * @return True on success false on failure
 */
int plt_u64toa(uint64_t data, char *c);
/**
 * @brief Signed 64bit number to string
 * @param data Number to convert to string
 * @param c Pointer to buffer to put string in
 * @return True on success false on failure
 */
int plt_s64toa(int64_t data, char *c);
/**
 * @brief Check if the string is all one character
 * @param str Pointer to string to check against
 * @param c Character to check
 * @param len Size of the string
 * @return true if all one character false otherwise
 */
bool str_all_c(uint8_t *str, uint8_t c, size_t len);
/**
 * @brief Compare memory while ignoring character case (used for strings)
 * @param s1 Pointer to memory to compare
 * @param s2 Pointer to memory to compare
 * @param len Size of memory to compare
 * @return 0 if s1 = s2 else -1
 */
int memcasecmp( const void *s1, const void *s2, size_t len );

#define snprintf_sf( buffer,size,format,... ) ({int r__ = snprintf(buffer,size,format,##__VA_ARGS__); (r__ > (size)) ? (size) : r__;})

#endif