// PERMUTE_ARGS:
// REQUIRED_ARGS: -D -w -o- -c -Ddtest_results/compilable -o-
// POST_SCRIPT: compilable/extra-files/ddocAny-postscript.sh 9475

module ddoc9475;

/// foo
void foo() { }

/// Example
unittest
{
    // comment 1
    foreach (i; 0 .. 10)
    {
        // comment 2
        documentedFunction();
    }
}

/// bar
void bar() { }

/// Example
unittest
{
    // bar comment
}

