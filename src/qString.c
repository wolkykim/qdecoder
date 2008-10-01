/**************************************************************************
 * qDecoder - Web Application Interface for C/C++   http://www.qDecoder.org
 *
 * Copyright (C) 2007 Seung-young Kim.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *************************************************************************/

/**
 * @file qString.c Advanced String API
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/time.h>
#include "qDecoder.h"
#include "qInternal.h"

/**
 * Remove white spaces(including CR, LF) from head and tail of the string.
 *
 * @param str		source string
 *
 * @return		a pointer of source string if rsuccessful, otherewise returns NULL
 *
 * @note This modify source string directly.
 */
char *qStrTrim(char *str) {
	int i, j;

	if (str == NULL) return NULL;
	for (j = 0; str[j] == ' ' || str[j] == '\t' || str[j] == '\r' || str[j] == '\n'; j++);
	for (i = 0; str[j] != '\0'; i++, j++) str[i] = str[j];
	for (i--; (i >= 0) && (str[i] == ' ' || str[i] == '\t' || str[i] == '\r' || str[i] == '\n'); i--);
	str[i+1] = '\0';

	return str;
}

/**
 * Remove tailing white spaces(including CR, LF) of the string.
 *
 * @param str		source string
 *
 * @return		a pointer of source string if rsuccessful, otherewise returns NULL
 *
 * @note This modify source string directly.
 */
char *qStrTrimTail(char *str) {
	int i;

	if (str == NULL)return NULL;
	for (i = strlen(str) - 1; (i >= 0) && (str[i] == ' ' || str[i] == '\t' || str[i] == '\r' || str[i] == '\n'); i--);
	str[i+1] = '\0';

	return str;
}

/**
 * Replace string or tokens as word from source string with given mode.
 *
 * @param mode		replacing mode
 * @param srcstr	source string
 * @param tokstr	token or string
 * @param word		target word to be replaced
 *
 * @return		a pointer of malloced or source string depending on the mode if rsuccessful, otherewise returns NULL
 *
 * @note
 * The mode argument has two separated character. First character
 * is used to decide replacing method and can be 't' or 's'.
 * The character 't' and 's' stand on [t]oken and [s]tring.
 *
 * When 't' is given each character of the token string(third argument)
 * will be compared with source string individually. If matched one
 * is found. the character will be replaced with given work.
 *
 * If 's' is given instead of 't'. Token string will be analyzed
 * only one chunk word. So the replacement will be occured when
 * the case of whole word matched.
 *
 * Second character is used to decide returning memory type and
 * can be 'n' or 'r' which are stand on [n]ew and [r]eplace.
 *
 * When 'n' is given the result will be placed into new array so
 * you should free the return string after using. Instead of this,
 * you can also use 'r' character to modify source string directly.
 * In this case, given source string should have enough space. Be
 * sure that untouchable value can not be used for source string.
 *
 * So there are four associatable modes such like below.
 *
 * Mode "tn" : [t]oken replacing & putting the result into [n]ew array.
 * Mode "tr" : [t]oken replacing & [r]eplace source string directly.
 * Mode "sn" : [s]tring replacing & putting the result into [n]ew array.
 * Mode "sr" : [s]tring replacing & [r]eplace source string directly.
 *
 * @code
 *   char srcstr[256], *retstr;
 *   char mode[4][2+1] = {"tn", "tr", "sn", "sr"};
 *
 *   for(i = 0; i < 4; i++) {
 *     strcpy(srcstr, "Welcome to the qDecoder project.");
 *     printf("before %s : srcstr = %s\n", mode[i], srcstr);
 *
 *     retstr = qStrReplace(mode[i], srcstr, "the", "_");
 *     printf("after  %s : srcstr = %s\n", mode[i], srcstr);
 *     printf("            retstr = %s\n\n", retstr);
 *     if(mode[i][1] == 'n') free(retstr);
 *   }
 *
 *   --[Result]--
 *   before tn : srcstr = Welcome to the qDecoder project.
 *   after  tn : srcstr = Welcome to the qDecoder project.
 *               retstr = W_lcom_ _o ___ qD_cod_r proj_c_.
 *
 *   before tr : srcstr = Welcome to the qDecoder project.
 *   after  tr : srcstr = W_lcom_ _o ___ qD_cod_r proj_c_.
 *               retstr = W_lcom_ _o ___ qD_cod_r proj_c_.
 *
 *   before sn : srcstr = Welcome to the qDecoder project.
 *   after  sn : srcstr = Welcome to the qDecoder project.
 *               retstr = Welcome to _ qDecoder project.
 *
 *   before sr : srcstr = Welcome to the qDecoder project.
 *   after  sr : srcstr = Welcome to _ qDecoder project.
 *               retstr = Welcome to _ qDecoder project.
 * @endcode
 */
char *qStrReplace(const char *mode, char *srcstr, const char *tokstr, const char *word) {
	if (mode == NULL || strlen(mode) != 2 || srcstr == NULL || tokstr == NULL || word == NULL) {
		DEBUG("Unknown mode \"%s\".", mode);
		return NULL;
	}

	char *newstr, *newp, *srcp, *tokenp, *retp;
	newstr = newp = srcp = tokenp = retp = NULL;

	char method = mode[0], memuse = mode[1];
	int maxstrlen, tokstrlen;

	/* Put replaced string into malloced 'newstr' */
	if (method == 't') { /* Token replace */
		maxstrlen = strlen(srcstr) * ( (strlen(word) > 0) ? strlen(word) : 1 );
		newstr = (char*)malloc(maxstrlen + 1);

		for (srcp = (char*)srcstr, newp = newstr; *srcp; srcp++) {
			for (tokenp = (char*)tokstr; *tokenp; tokenp++) {
				if (*srcp == *tokenp) {
					char *wordp;
					for (wordp = (char*)word; *wordp; wordp++) *newp++ = *wordp;
					break;
				}
			}
			if (!*tokenp) *newp++ = *srcp;
		}
		*newp = '\0';
	} else if (method == 's') { /* String replace */
		if (strlen(word) > strlen(tokstr)) maxstrlen = ((strlen(srcstr) / strlen(tokstr)) * strlen(word)) + (strlen(srcstr) % strlen(tokstr));
		else maxstrlen = strlen(srcstr);
		newstr = (char*)malloc(maxstrlen + 1);
		tokstrlen = strlen(tokstr);

		for (srcp = srcstr, newp = newstr; *srcp; srcp++) {
			if (!strncmp(srcp, tokstr, tokstrlen)) {
				char *wordp;
				for (wordp = (char*)word; *wordp; wordp++) *newp++ = *wordp;
				srcp += tokstrlen - 1;
			} else *newp++ = *srcp;
		}
		*newp = '\0';
	} else {
		DEBUG("Unknown mode \"%s\".", mode);
		return NULL;
	}

	/* decide whether newing the memory or replacing into exist one */
	if (memuse == 'n') retp = newstr;
	else if (memuse == 'r') {
		strcpy(srcstr, newstr);
		free(newstr);
		retp = srcstr;
	} else {
		DEBUG("Unknown mode \"%s\".", mode);
		free(newstr);
		return NULL;
	}

	return retp;
}

/**
 * Copies at most len characters from src into dst then append '\0'.
 *
 * @param dst		a pointer of the string to be copied
 * @param dstsize	size of dst
 * @param src		a pointer of source string
 * @param nbytes	bytes to copy
 *
 * @return		always returns a pointer of dst
 *
 * @note The dst string will be always terminated by '\0'. (bytes that follow a null byte are not copied)
 */
char *qStrCpy(char *dst, size_t dstsize, const char *src, size_t nbytes) {
	if(dst == NULL || dstsize == 0 || src == NULL || nbytes == 0) return dst;

	if(nbytes >= dstsize) nbytes = dstsize - 1;
	strncpy(dst, src, nbytes);
	dst[nbytes] = '\0';

	return dst;
}

/**
 * Convert character to bigger character.
 *
 * @param str		a pointer of source string
 *
 * @return		always returns a pointer of str
 *
 * @note This modify str directly.
 */
char *qStrUpper(char *str) {
	char *cp;

	if (!str) return NULL;
	for (cp = str; *cp; cp++) if (*cp >= 'a' && *cp <= 'z') *cp -= 32;
	return str;
}

/**
 * Convert character to lower character.
 *
 * @param str		a pointer of source string
 *
 * @return		always returns a pointer of str
 *
 * @note This modify str directly.
 */
char *qStrLower(char *str) {
	char *cp;

	if (!str) return NULL;
	for (cp = str; *cp; cp++) if (*cp >= 'A' && *cp <= 'Z') *cp += 32;
	return str;
}


/**
 * Find a substring with no case-censitive
 *
 * @param big		a pointer of source string
 * @param small		a pointer of substring
 *
 * @return		a pointer of the first occurrence in the big string if successful, otherwise returns NULL
 */
char *qStrCaseStr(const char *s1, const char *s2) {
	if (s1 == NULL || s2 == NULL) return NULL;

	char *s1p = strdup(s1);
	char *s2p = strdup(s2);
	if (s1p == NULL || s2p == NULL) {
		if(s1p != NULL) free(s1p);
		if(s2p != NULL) free(s2p);
		return NULL;
	}

	qStrUpper(s1p);
	qStrUpper(s2p);

	char *sp = strstr(s1p, s2p);
	if (sp != NULL) sp = (char*)s1 + (sp - s1p);
	free(s1p);
	free(s2p);

	return sp;
}

/**
 * Reverse the order of characters in the string
 *
 * @param str		a pointer of source string
 *
 * @return		always returns a pointer of str
 *
 * @note This modify str directly.
 */
char *qStrRev(char *str) {
	if (str == NULL) return str;

	char *p1, *p2;
	for (p1 = str, p2 = str + (strlen(str) - 1); p2 > p1; p1++, p2--) {
		char t = *p1;
		*p1 = *p2;
		*p2 = t;
	}

	return str;
}

/**
 * Split string into tokens
 *
 * @param str		source string
 * @param delimiters	string that specifies a set of delimiters that may surround the token being extracted
 * @param retstop	stop delimiter character will be stored. it can be NULL if you don't want to know.
 *
 * @return	a pointer to the first byte of a token if successful, otherwise returns NULL.
 *
 * @note
 * The major difference between qStrTok() and standard strtok() is that qStrTok() can returns empty string tokens.
 * If the str is "a:b::d", qStrTok() returns "a", "b", "", "d". But strtok() returns "a","b","d".
 */
char *qStrTok(char *str, const char *delimiters, char *retstop) {
	static char *tokenep;
	char *tokensp;
	int i, j;

	if (str != NULL) tokensp = tokenep = str;
	else tokensp = tokenep;

	for (i = strlen(delimiters); *tokenep; tokenep++) {
		for (j = 0; j < i; j++) {
			if (*tokenep == delimiters[j]) {
				if (retstop != NULL) *retstop = delimiters[j];
				*tokenep = '\0';
				tokenep++;
				return tokensp;
			}
		}
	}

	if (retstop != NULL) *retstop = '\0';
	if (tokensp != tokenep) return tokensp;
	return NULL;
}

/**
 * String Tokenizer
 *
 * @param str		source string
 * @param delimiters	string that specifies a set of delimiters that may surround the token being extracted
 *
 * @return	a pointer to the Q_ENTRY list
 *
 * @note	Tokens will be stored at Q_ENTRY list with key 1, 2, 3, 4, 5...
 *
 * @code
 *   FILE *fp = fopen("/etc/passwd", "r");
 *   char *buf;
 *   while((buf = qFileReadLine(fp)) != NULL) {
 *     qStrTrimTail(buf);
 *      Q_ENTRY *tokens = qStrTokenizer(buf, ":");
 *      printf("%s\n", qEntryGetStr(tokens, "1"));
 *      qEntryFree(tokens);
 *   }
 *   fclose(fp);
 * @endcode
 */
Q_ENTRY *qStrTokenizer(char *str, const char *delimiters) {
	Q_ENTRY *entry = qEntryInit();
	char *token;
	int i;
	for(i = 1, token = qStrTok(str, delimiters, NULL); token != NULL; token = qStrTok(NULL, delimiters, NULL), i++) {
		char key[10+1];
		sprintf(key, "%d", i);
		qEntryPutStr(entry, key, token, false);
	}

	return entry;
}

/**
 * Convert integer to comma string.
 *
 * @param number	integer
 *
 * @return		a pointer of malloced string which contains comma separated number if successful, otherwise returns NULL
 */
char *qStrCommaNumber(int number) {
	char *str, *strp;

	str = strp = (char*)malloc(sizeof(char) * (14+1));
	if(str == NULL) return NULL;

	char buf[10+1], *bufp;
	snprintf(buf, sizeof(buf), "%d", abs(number));

	if (number < 0) *strp++ = '-';
	for (bufp = buf; *bufp != '\0'; strp++, bufp++) {
		*strp = *bufp;
		if ((strlen(bufp)) % 3 == 1 && *(bufp + 1) != '\0') *(++strp) = ',';
	}
	*strp = '\0';

	return str;
}

/**
 * Append formatted string to the end of the source str
 *
 * @param str		a pointer of original string
 * @param format	string format to append
 *
 * @return		a pointer of str
 */
char *qStrCatf(char *str, const char *format, ...) {
	char buf[MAX_LINEBUF];
	va_list arglist;

	va_start(arglist, format);
	vsnprintf(buf, sizeof(buf), format, arglist);
	va_end(arglist);

	return strcat(str, buf);
}

/**
 * Duplicate a substing set
 *
 * @param str		a pointer of original string
 * @param start		substring which is started with this
 * @param end		substring which is ended with this
 *
 * @return		a pointer of malloced string if successful, otherwise returns NULL
 */
char *qStrDupBetween(const char *str, const char *start, const char *end) {
	char *s;
	if ((s = strstr(str, start)) == NULL) return NULL;
	s += strlen(start);

	char *e;
	if ((e = strstr(s, end)) == NULL) return NULL;

	int len = e - s;

	char *buf = (char*)malloc(sizeof(char) * (len + 1));
	qStrCpy(buf, len + 1, s, len);

	return buf;
}

/**
 * Generate unique id
 *
 * @param seed		additional seed string. this can be NULL
 *
 * @return		a pointer of malloced string
 *
 * @note The length of returned string is 32+1 including terminating NULL character.
 */
char *qStrUnique(const char *seed) {
	static int count = 0;

	if(count == 0) {
        	srandom(time(NULL));
        }
	count++;

	long int usec;
#ifdef _WIN32
	usec = 0;
#else
	struct timeval tv;
	gettimeofday(&tv, NULL);
	usec = tv.tv_usec;
#endif

	char szSeed[128];
	snprintf(szSeed, sizeof(szSeed), "%u%d%ld%lu%ld%s", getpid(), count, random(), (unsigned long int)time(NULL), usec, (seed!=NULL?seed:""));
	return qHashMd5Str(szSeed, strlen(szSeed));
}

/**
 * Test for an alpha-numeric string
 *
 * @param str		a pointer of string
 *
 * @return		true for ok, otherwise returns false
 *
 * @code
 *   qCharEncode("한글", "EUC-KR", "UTF-8", 1.5);
 * @endcode
 */
bool qStrIsAlnum(const char *str) {
        for (; *str; str++) {
                if(isalnum(*str) == 0) return false;
        }
        return true;
}

#ifdef __linux__
#include <iconv.h>
/**
 * Convert character encoding
 *
 * @param str		additional seed string. this can be NULL
 * @param fromcode	encoding type of str
 * @param tocode	encoding to convert
 * @param mag		magnification between fromcode and tocode
 *
 * @return		a pointer of malloced converted string if successful, otherwise returns NULL
 *
 * @code
 *   qCharEncode("한글", "EUC-KR", "UTF-8", 1.5);
 * @endcode
 */
char *qStrConvEncoding(const char *str, const char *fromcode, const char *tocode, float mag) {
	if(str == NULL) return NULL;

	char *fromstr = (char*)str;
	size_t fromsize = strlen(fromstr) + 1;

	size_t tosize = sizeof(char) * ((mag * (fromsize - 1)) + 1);
	char *tostr = (char *)malloc(tosize);
	if(tostr == NULL) return NULL;
	char *tostr1 = tostr;

	iconv_t it = iconv_open(tocode, fromcode);
	if(it < 0) {
		DEBUG("iconv_open() failed.");
		return NULL;
	}

	printf("%d %d\n", fromsize, tosize);
	int ret = iconv(it, &fromstr, &fromsize, &tostr, &tosize);

	iconv_close(it);

	printf("%d %d\n", fromsize, tosize);
	if(ret < 0) {
		DEBUG("iconv() failed.");
		free(tostr1);
		return NULL;
	}

	return tostr1;
}
#endif
