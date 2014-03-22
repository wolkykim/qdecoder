/******************************************************************************
 * qDecoder - http://www.qdecoder.org
 *
 * Copyright (c) 2000-2012 Seungyoung Kim.
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
 ******************************************************************************
 * $Id: session.c 639 2012-05-07 23:44:19Z seungyoung.kim $
 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "qdecoder.h"

int main(void)
{
    qentry_t *req = qcgireq_parse(NULL, 0);

    // fetch queries
    time_t expire = (time_t)req->getint(req, "expire");
    const char *mode = req->getstr(req, "mode", false);
    const char *name   = req->getstr(req, "name", false);
    const char *value  = req->getstr(req, "value", false);

    // start session.
    qentry_t *sess = qcgisess_init(req, NULL);

    // Mose case, you don't need to set timeout. this is just example
    if (expire > 0) qcgisess_settimeout(sess, expire);

    switch (mode[0]) {
        case 's': {
            req->putstr(sess, name, value, true);
            break;
        }
        case 'r': {
            req->remove(sess, name);
            break;
        }
        case 'd': {
            qcgisess_destroy(sess);
            qcgires_setcontenttype(req, "text/html");
            printf("Session reinitialized.\n");
            return 0;
            break;
        }
    }
    // screen out
    qcgires_setcontenttype(req, "text/plain");
    req->print(sess, stdout, true);

    // save session & free allocated memories
    qcgisess_save(sess);
    sess->free(sess);
    req->free(req);
    return 0;
}
