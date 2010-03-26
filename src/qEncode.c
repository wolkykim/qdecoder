/*
 * Copyright 2000-2010 The qDecoder Project. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE QDECODER PROJECT ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE QDECODER PROJECT BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * $Id$
 */

/**
 * @file qEncode.c Encoding/decoding API
 */


#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include "qDecoder.h"
#include "qInternal.h"

/**
 * Parse URL encoded query string
 *
 * @param entry		a pointer of Q_ENTRY structure. NULL can be used.
 * @param query		URL encoded string.
 * @param equalchar	separater of key, value pair.
 * @param sepchar	separater of line.
 * @param count		if count is not NULL, a number of parsed entries are stored.
 *
 * @return	a pointer of Q_ENTRY if successful, otherwise returns NULL.
 *
 * @code
 *   cont char query = "category=love&str=%C5%A5%B5%F0%C4%DA%B4%F5&sort=asc";
 *   Q_ENTRY *entry = qDecodeQueryString(NULL, req->pszQueryString, '=', '&', NULL);
 *   printf("category = %s\n", entry->getStr(entry, "category"));
 *   printf("str = %s\n", entry->getStr(entry, "str"));
 *   printf("sort = %s\n", entry->getStr(entry, "sort"));
 *   entry->free(entry);
 * @endcode
 */
Q_ENTRY *qParseQueries(Q_ENTRY *entry, const char *query, char equalchar, char sepchar, int *count) {
	if(entry == NULL) {
		entry = qEntry();
		if(entry == NULL) return NULL;
	}

	char *newquery = NULL;
	int cnt = 0;

	if(query != NULL) newquery = strdup(query);
	while (newquery && *newquery) {
		char *value = _q_makeword(newquery, sepchar);
		char *name = qStrTrim(_q_makeword(value, equalchar));
		qUrlDecode(name);
		qUrlDecode(value);

		if(entry->putStr(entry, name, value, false) == true) cnt++;
		free(name);
		free(value);
	}
	if(newquery != NULL) free(newquery);
	if(count != NULL) *count = cnt;

	return entry;
}

/**
 * Encode data using URL encoding(Percent encoding) algorithm.
 *
 * @param bin	a pointer of input data.
 * @param size	the length of input data.
 *
 * @return	a malloced string pointer of URL encoded string in case of successful, otherwise returns NULL
 *
 * @code
 *   const char *text = strdup("hello 'qDecoder' world");
 *
 *   char *encstr = qUrlEncode(text, strlen(text));
 *   if(encstr == NULL) return -1;
 *
 *   printf("Original: %s\n", text);
 *   printf("Encoded : %s\n", encstr);
 *
 *   size_t decsize = qUrlDecode(encstr);
 *
 *   printf("Decoded : %s (%zu bytes)\n", encstr, decsize);
 *   free(encstr);
 *
 *   --[output]--
 *   Original: hello 'qDecoder' world
 *   Encoded:  hello%20%27qDecoder%27%20world
 *   Decoded:  hello 'qDecoder' world (22 bytes)
 * @endcode
 */
char *qUrlEncode(const void *bin, size_t size) {
	const char URLCHARTBL[256] = {
		 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , /* 00-0F */
		 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , /* 10-1F */
		 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 ,'-','.','/', /* 20-2F */
		'0','1','2','3','4','5','6','7','8','9',':', 0 , 0 , 0 , 0 , 0 , /* 30-3F */
		'@','A','B','C','D','E','F','G','H','I','J','K','L','M','N','O', /* 40-4F */
		'P','Q','R','S','T','U','V','W','X','Y','Z', 0 ,'\\',0 , 0 ,'_', /* 50-5F */
		 0 ,'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o', /* 60-6f */
		'p','q','r','s','t','u','v','w','x','y','z', 0 , 0 , 0 , 0 , 0 , /* 70-7F */
		 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , /* 80-8F */
		 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , /* 90-9F */
		 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , /* A0-AF */
		 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , /* B0-BF */
		 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , /* C0-CF */
		 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , /* D0-DF */
		 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , /* E0-EF */
		 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0   /* F0-FF */
	}; // 0 means must be encoded.

	if(bin == NULL) return NULL;
	if(size == 0) return strdup("");

	// malloc buffer
	char *pszEncStr = (char *)malloc((size * 3) + 1);
	if (pszEncStr == NULL) return NULL;

	char *pszEncPt = pszEncStr;
	char *pBinPt = (char*)bin;
	const char *pBinEnd = (bin + size - 1);
	for (; pBinPt <= pBinEnd; pBinPt++) {
		if(URLCHARTBL[(int)(*pBinPt)] != 0) {
			*pszEncPt++ = *pBinPt;
		} else {
			*pszEncPt++ = '%';
			*pszEncPt++ = ((*pBinPt >> 4)   < 0x0A) ? ((*pBinPt >> 4) + '0')   : ((*pBinPt >> 4) + 'a');
			*pszEncPt++ = ((*pBinPt & 0x0F) < 0x0A) ? ((*pBinPt & 0x0F) + '0') : ((*pBinPt & 0x0F) + 'a');
		}
	}
	*pszEncPt = '\0';

	return pszEncStr;
}

/**
 * Decode URL encoded string.
 *
 * @param str	a pointer of URL encoded string.
 *
 * @return	the length of bytes stored in str in case of successful, otherwise returns NULL
 *
 * @note
 * This modify str directly. And the 'str' is always terminated by NULL character.
 */
size_t qUrlDecode(char *str) {
	if (str == NULL) {
		return 0;
	}

	char *pEncPt, *pBinPt = str;
	for(pEncPt = str; *pEncPt != '\0'; pEncPt++) {
		switch (*pEncPt) {
			case '+': {
				*pBinPt++ = ' ';
				break;
			}
			case '%': {
				*pBinPt++ = _q_x2c(*(pEncPt + 1), *(pEncPt + 2));
				pEncPt += 2;
				break;
			}
			default: {
				*pBinPt++ = *pEncPt;
				break;
			}
		}
	}
	*pBinPt = '\0';

	return (pBinPt - str);
}

/**
 * Encode data using BASE64 algorithm.
 *
 * @param bin	a pointer of input data.
 * @param size	the length of input data.
 *
 * @return	a malloced string pointer of BASE64 encoded string in case of successful, otherwise returns NULL
 *
 * @code
 *   const char *text = strdup("hello 'qDecoder' world");
 *
 *   char *encstr = qBase64Encode(text, strlen(text));
 *   if(encstr == NULL) return -1;
 *
 *   printf("Original: %s\n", text);
 *   printf("Encoded : %s\n", encstr);
 *
 *   size_t decsize = qBase64Decode(encstr);
 *
 *   printf("Decoded : %s (%zu bytes)\n", encstr, decsize);
 *   free(encstr);
 *
 *   --[output]--
 *   Original: hello 'qDecoder' world
 *   Encoded:  hello%20%27qDecoder%27%20world
 *   Decoded:  hello 'qDecoder' world (22 bytes)
 * @endcode
 */
char *qBase64Encode(const void *bin, size_t size) {
	const char B64CHARTBL[64] = {
		'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P', /* 00-0F */
		'Q','R','S','T','U','V','W','X','Y','Z','a','b','c','d','e','f', /* 10-1F */
		'g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v', /* 20-2F */
		'w','x','y','z','0','1','2','3','4','5','6','7','8','9','+','/'  /* 30-3F */
	};

	if(size == 0) {
		return strdup("");
	}

	// malloc for encoded string
	char *pszB64 = (char*)malloc((4 * ((size / 3) + ((size % 3 == 0) ? 0 : 1)) + 1));
	if(pszB64 == NULL) {
		return NULL;
	}

	char *pszB64Pt = pszB64;
	unsigned char *pBinPt, *pBinEnd = (unsigned char*)(bin + size - 1);
	unsigned char szIn[3] = {0,0,0};
	int nOffset;
	for(pBinPt = (unsigned char*)bin, nOffset = 0; pBinPt <= pBinEnd; pBinPt++, nOffset++) {
		int nIdxOfThree = nOffset % 3;
		szIn[nIdxOfThree] = *pBinPt;
		if(nIdxOfThree < 2 && pBinPt < pBinEnd) continue;

		*pszB64Pt++ = B64CHARTBL[((szIn[0] & 0xFC) >> 2)];
		*pszB64Pt++ = B64CHARTBL[(((szIn[0] & 0x03) << 4) | ((szIn[1] & 0xF0) >> 4))];
		*pszB64Pt++ = (nIdxOfThree >= 1) ? B64CHARTBL[(((szIn[1] & 0x0F) << 2) | ((szIn[2] & 0xC0) >> 6))] : '=';
		*pszB64Pt++ = (nIdxOfThree >= 2) ? B64CHARTBL[(szIn[2] & 0x3F)] : '=';

		memset((void*)szIn, 0, sizeof(szIn));
	}
	*pszB64Pt = '\0';

	return pszB64;
}

/**
 * Decode BASE64 encoded string.
 *
 * @param str	a pointer of URL encoded string.
 *
 * @return	the length of bytes stored in str in case of successful, otherwise returns NULL
 *
 * @note
 * This modify str directly. And the 'str' is always terminated by NULL character.
 */
size_t qBase64Decode(char *str) {
	const char B64MAPTBL[256] = {
		64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64, /* 00-0F */
		64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64, /* 10-1F */
		64,64,64,64,64,64,64,64,64,64,64,62,64,64,64,63, /* 20-2F */
		52,53,54,55,56,57,58,59,60,61,64,64,64,64,64,64, /* 30-3F */
		64, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14, /* 40-4F */
		15,16,17,18,19,20,21,22,23,24,25,64,64,64,64,64, /* 50-5F */
		64,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40, /* 60-6F */
		41,42,43,44,45,46,47,48,49,50,51,64,64,64,64,64, /* 70-7F */
		64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64, /* 80-8F */
		64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64, /* 90-9F */
		64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64, /* A0-AF */
		64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64, /* B0-BF */
		64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64, /* C0-CF */
		64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64, /* D0-DF */
		64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64, /* E0-EF */
		64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64  /* F0-FF */
	};

	char *pEncPt, *pBinPt = str;
	int nIdxOfFour = 0;
	char cLastByte = 0;
	for(pEncPt = str; *pEncPt != '\0'; pEncPt++) {
		char cByte = B64MAPTBL[(int)(*pEncPt)];
		if(cByte == 64) continue;

		if(nIdxOfFour == 0) {
			nIdxOfFour++;
		} else if(nIdxOfFour == 1) {
			// 00876543 0021????
			//*pBinPt++ = ( ((cLastByte << 2) & 0xFC) | ((cByte >> 4) & 0x03) );
			*pBinPt++ = ( (cLastByte << 2) | (cByte >> 4) );
			nIdxOfFour++;
		} else if(nIdxOfFour == 2) {
			// 00??8765 004321??
			*pBinPt++ = ( (cLastByte << 4) | (cByte >> 2) );
			nIdxOfFour++;
		} else {
			// 00????87 00654321
			*pBinPt++ = ( (cLastByte << 6) | cByte );
			nIdxOfFour = 0;
		}

		cLastByte = cByte;
	}
	*pBinPt = '\0';

	return (pBinPt - str);
}
