/**
 * Missing declarations and/or definitions in Windows when building against MinGW
 *
 * @file msw_missing.h
 */

#ifndef MSW_MISSING_H
#define MSW_MISSING_H

#include <windows.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>

#define S_IRGRP 0040
#define S_IROTH 0004

int mkstemp (char *tmpl);

#endif  /* MSW_MISSING_H */