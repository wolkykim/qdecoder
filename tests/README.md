qDecoder Unit Tests
===================

# How to run unit tests.

```
$ make test
Test internal.c/_q_urldecode
======================================================================
* TEST : Test plain string . OK (1 assertions, 0ms)
* TEST : Test urlencoded string . OK (1 assertions, 0ms)
* TEST : Test urlencoded string exceptions .. OK (2 assertions, 0ms)
======================================================================
PASS - 3/3 tests passed.
```

# How to write unit tests

We need your help in writing unit tests. Please refer qunit.h for your reference.

```C
#include "qunit.h"
#include "qdecoder.h"

QUNIT_START("Test title");

TEST("Test name1") {
    ASSERT_EQUAL_STR("abc", "abc");
    ASSERT_EQUAL_INT(8, 8);
}

TEST("Test name2") {
    ASSERT_EQUAL_PT(NULL == NULL);
}

QUNIT_END();
```
