ExtremeLibrary is a **pure C** library containing helpful (I hope) utility routines for C programmers.
Now the "Dynamic strings" part of the library is released.

## Description ##

To create new dynamic string from C style string use: 
```C
str *pStr = elstrCreateFromCStr("Parrot Peter picked a pack of pickled peppers");
```
To append C style string to existing dynamic string: 
```C
elstrAppendCStr(pStr, "!!!");
```

## Changelog ##

- **v1.0.0**, *18 May 2014*
  - Initial release.

### Library usage ###

Just add source files to your project.

### Documentation ###

Source code contains documentation comments so it's easy to build docs via [Doxygen](www.doxygen.org).
