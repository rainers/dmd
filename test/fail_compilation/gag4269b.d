﻿// REQUIRED_ARGS: -c -o-
/*
TEST_OUTPUT:
---
fail_compilation/gag4269b.d(10): Error: undefined identifier Y, did you mean variable y?
---
*/

static if(is(typeof(X2.init))) {}
struct X2 { Y y; }
