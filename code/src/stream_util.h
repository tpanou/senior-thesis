/**
* @file
* @addtogroup stream_utils Stream Utilities
* @ingroup http_server
*
* @brief Utility functions that operate on an input stream, typically from the
* network module.
*/

#ifndef STREAM_UTIL_H_INCL
#define STREAM_UTIL_H_INCL

#include <inttypes.h>

/**
* @brief Indicates any character (based on the context of its use).
*/
#define OTHER   -5

#ifndef EOF
/**
* @brief Indicates that the End-of-File has been reached.
*/
#define EOF     -1
#endif

void stream_set_source(int8_t (*next_char)(uint8_t*));

/**
* @brief Read up to a four-digit unsigned hexadecimal number from stream
* (0 up to FFFF).
*
* Any leading zeros are ignored. Prefix "0x" is not supported.
*
* @param[out] value The number read. Defaults to @c 0.
* @param[in,out] c The first character to start parsing from and the last one
*   read from the stream.
* @returns One of:
*   - @c 0; if the last character read is not a digit or EOF.
*   - #OTHER; if a greater number than FFFF is available.
*   - EOF
*/
int8_t parse_hex16(uint16_t* value, uint8_t* c);

/**
* @brief Read up to a 4-digit unsigned number from stream (0 up to 9999).
*
* Any leading zeros are ignored.
*
* @param[out] value The number read. Defaults to @c 0.
* @param[in,out] c The first character to start parsing from and the last one
*   read from the stream.
* @returns One of:
*   - 0; if the last character read is not a digit or EOF.
*   - #OTHER; if a greater number than 9999 is available.
*   - EOF
*/
int8_t parse_uint16(uint16_t* value, uint8_t* c);

/**
* @brief Read up to a two-digit unsigned number from stream (0 up to 99).
*
* Any leading zeros are ignored.
*
* @param[out] value The number read. Defaults to @c 0.
* @param[in,out] c The first character to start parsing from and the last one
*   read from the stream.
* @returns One of:
*   - 0; if the last character read is not a digit or EOF.
*   - #OTHER; if a greater number than 99 is available.
*   - EOF
*/
int8_t parse_uint8(uint8_t* value, uint8_t* c);

/**
* @brief Find the closest match from an array of strings with the stream.
*
* Accepts an array of strings (descriptors) and compares the against the input
* stream. The descriptors should be sorted in ascending order beforehand, if
* necessary, and no duplicates should exist. Otherwise, the result is undefined.
*
* The return value is the index of the closest matching descriptor (which may or
* may not be a valid match -- more on this later); the value #OTHER, if it is
* certain no descriptor provides a match; or @c EOF if the end of the stream
* has been reached in the meantime.
* It should be noted that this function irreversibly consumes bytes from the
* input stream which are not reinstated back into it even if no match is found.
*
* More specifically, on each iteration a single character is extracted from the
* input stream and is compared against the known descriptors and, in particular,
* against the character at a position equal to the number of characters read
* (ie, the first character read is compared against the first character of all
* descriptors, etc).
*
* Initially, all the descriptors are included in the comparisons. However, as
* the iterations progress, descriptors that fail a match are omitted from the
* remaining iterations. Because the descriptors are given in ascending order,
* this is easily done by maintaining two moving boundaries.
*
* By the end of the iterations, the two boundaries have converged and specify
* a particular descriptor. That descriptor is a match if two conditions apply:
* - Its last character has been reached (null-character).
* - The input character that caused that last mismatch is an acceptable
* delimiter for this particular input.
*
* The former is readily determined whilst the latter, not. This is because this
* function is content-unaware and, thus, ignorant of what constitutes an
* acceptable delimiter. Consequently, it cannot fully determine whether the
* index returned actually corresponds to a full match; that is the
* responsibility of the caller. It can, however, determine with certainty when
* there is no match, in which case, it returns #OTHER.
*
* @param[in] desc Array with strings (descriptors).
* @param[in] min The lower boundary of @p desc; index of the first literal to
*   use in the comparisons.
* @param[in] max The upper boundary of @p desc; index of the last literal to use
*   in the comparisons, incremented by 1.
* @param[in,out] c The first character to compare against the strings and the
*   last one read from the stream.
* @returns One of:
*   - Index of @p desc with a possible match.
*   - #OTHER; on certainty of no match.
*   - EOF; if end of stream has been reached before hitting a match/mismatch.
*/
int8_t stream_match(uint8_t** desc, uint8_t min, uint8_t max, uint8_t* c);

/**
* @brief Find the closest match from an array of strings with the stream.
*
* It provides comparable functionality to stream_match() but with greater
* flexibility as far as the iteration termination is concerned. More
* specifically, new iterations take place as long as there is at least one match
* per iteration. Upon failure, the function returns with neither @p max or @p
* cmp_idx being updated.
* It is the caller's responsibility to determine whether the iterations should
* proceed as well as whether they should do so from where they had previously
* terminated or even with which character.

* stream_match() could actually be implemented as a wrapper around this one.
*
* @param[in] desc Array with strings (descriptors).
* @param[in] abs_min Index of the first acceptable element of the array. It is
*   different than @p min in that this one represents the absolute minimum value
*   of the array (below which there is no valid descriptor).
* @param[in,out] min The lower boundary of matches in @p desc. Upon first
*   invocation, this should be equal to @p abs_min. As iterations take place,
*   this may be increased internally.
* @param[in,out] max The upper boundary of @p desc. Upon first invocation, this
*   should be equal to the index of the last descriptor plus 1.
* @param[in,out] cmp_idx Character index of strings found within @p desc that
*   is to be compared against @p c and the rest of the stream. Upon first
*   invocation, this should be 0.
* @param[in,out] c The first character to compare against the strings and the
*   last one read from the stream.
* @returns One of:
*   - Index of @p desc with a possible match.
*   - #OTHER; on certainty of no match.
*   - EOF; if end of stream has been reached before hitting a match/mismatch.
*/
int8_t stream_match_ext(uint8_t** desc,
                        uint8_t abs_min,
                        uint8_t* min,
                        uint8_t* max,
                        uint8_t* cmp_idx,
                        uint8_t* c);

#endif /* STREAM_UTIL_H_INCL */
