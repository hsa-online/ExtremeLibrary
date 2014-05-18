ExtremeLibrary is a **pure C** library containing helpful (I hope) utility routines for C programmers.
Now the "Dynamic strings" part of the library is released.

## Description ##

To create new dynamic string from C style string use: 
```C
str *pStr = elstrCreateFromCStr("Parrot Peter picked a pack of pickled peppers");
```
Now do some more things with our string: 
```C
elstrAppendCStr(pStr, "!!!");
elstrPrependCStr(pStr, "*** ");
elstrAppendCStr(pStr, " ***");
elstrInsertCStr(pStr, 11, "\"");
elstrInsertCStr(pStr, 17, "\"");
```
And check what we got:
```C
printf("\"%s\"\n", elstrGetRawBuf(pStr));
```

```
"*** Parrot "Peter" picked a pack of pickled peppers!!! ***"
```

Many other functions are supported (Delete, Format, Trim, Split and etc. etc.).

Good idea to not forget to destroy our string:
```C
elstrDestroy(pStr);
```

Also If dynamic string is created using any elstrCreateXX routine except the 
elstrCreateFixed() it becomes "variable size" dynamic string. This means that 
the buffer size is allocated "when necessary" and may grow.
 
Fixed strings use buffers preallocated externally and are unable to grow. 
These strings are extremely helpful when data locality is important so they
may work much faster in some circumstances. 

## Changelog ##

- **v1.0.0**, *18 May 2014*
  - Initial release.

### Library usage ###

Just add source files to your project.

### Documentation ###

Source code contains documentation comments so it's easy to build docs via [Doxygen](www.doxygen.org).
