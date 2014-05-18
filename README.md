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

## Changelog ##

- **v1.0.0**, *18 May 2014*
  - Initial release.

### Library usage ###

Just add source files to your project.

### Documentation ###

Source code contains documentation comments so it's easy to build docs via [Doxygen](www.doxygen.org).
