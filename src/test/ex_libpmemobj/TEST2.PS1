# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2015-2019, Intel Corporation

#
# src/test/ex_libpmemobj/TEST2 -- unit test for libpmemobj examples
#

. ..\unittest\unittest.PS1

require_test_type medium
require_build_type debug nondebug
require_no_unicode

setup

echo @"
i 1234
i 4321
p
n 5
p
q
"@ | &$Env:EXAMPLES_DIR\ex_pmemobj_mapcli hashmap_atomic $DIR\testfile1 555 > out$Env:UNITTEST_NUM.log 2>&1

check_exit_code

check

pass
