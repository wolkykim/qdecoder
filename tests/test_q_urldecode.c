/******************************************************************************
 * qLibc
 *
 * Copyright (c) 2010-2015 Seungyoung Kim.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *****************************************************************************/

#include "qunit.h"
#include "qdecoder.h"
#include "internal.h"
#include <ctype.h>

void test_urldecode(const char *v1, const char *v2);

QUNIT_START("Test internal.c/_q_urldecode");

TEST("Test plain strings")
{
    test_urldecode("Hello World", "Hello World");
}

TEST("Test urlencoded strings")
{
    test_urldecode("Hello%20World%21%40%23", "Hello World!@#");
    test_urldecode("Hello World!@#", "Hello World!@#");
    test_urldecode("%60%7E%21%40%23%24%25%5E%26%2A%28%29%2D%5F%3D%2B%5B%7B%5D%7D%5C%7C%3B%3A%27%22%2C%3C%2E%3E%2F%3F", \
    "`~!@#$%^&*()-_=+[{]}\\|;:'\",<.>/?");
}

TEST("Test improper encodings")
{
    // ending with '%' character
    test_urldecode("Hello%20World%21%40%", "Hello World!@%");

    // incomplete ending with '%2'
    test_urldecode("Hello%20World%21%40%2", "Hello World!@%2");

    // improper encoding - '%%'
    test_urldecode("Hello%20World%21%40%%", "Hello World!@%%");

    // improper encoding - '%%%'
    test_urldecode("Hello%20World%21%40%%%", "Hello World!@%%%");

    // improper encoding - '%%%%'
    test_urldecode("Hello%20World%21%40%%%%", "Hello World!@%%%%");

    // improper encoding - '%%%61%%'
    test_urldecode("Hello%20World%21%40%%%61%%", "Hello World!@%%a%%");

    // non hexadecimal digits - '%1Q'
    test_urldecode("Hello%20World%21%40%1Q", "Hello World!@%1Q");
}

QUNIT_END();

void test_urldecode(const char *v1, const char *v2)
{
    char *v = strdup(v1);
    _q_urldecode(v);
    ASSERT_EQUAL_STR(v, v2);
    free(v);
}
