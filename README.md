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

Good idea is to not forget to destroy our string:
```C
elstrDestroy(pStr);
```

Also if dynamic string is created using any elstrCreateXX routine except the 
elstrCreateFixed() it becomes "variable size" dynamic string. This means that 
the buffer size is allocated "when necessary" and may grow.
 
Fixed strings can be created with elstrCreateFixed(). They use buffers preallocated
externally so they can't grow. These strings are extremely helpful when data locality
is important. They may work much faster than their "variable size" counterparts in 
some circumstances. 

### Names of the functions ###

A lot of library functions work both with parameters provided as *dynamic strings* 
and *C style strings*. So to make library functions self-descriptive their names 
include short description of parameter type:

- `CStr` means *C style string* is expected as parameter.

- `ELStr` means *dynamic string* is expected as parameter.

Also some basic handling of multibyte strings is supported:
All functions which names started from `elstrMB` are intended to work with multibyte strings.
For example it's easy to get the length of multibyte string in characters:
```
size_t nMBLength = elstrMBGetLength(pStr);
```
Before calling any `elstrMB`-function it's necessary to set the proper locale:
```
setlocale(LC_ALL, ...
```

It's easy to create N-Grams from multibyte strings:
```
size_t nCountNGrams;
str **pNGrams;
size_t nN = 3;
size_t nMemTotal = elstrMBCreateNGrams(pStr, nN, &pNGrams, 0, &nCountNGrams);
```
Above call creates 3-grams from the dynamic string provided.

N-Grams allowing to compute strings similarity coefficient:
```
float fSimilarity = elstrMBCompareNGrams(pNGrams2, pCountNGrams2, pNGrams, pCountNGrams);
```

### Changelog ###

- **v1.0.0**, *18 May 2014*
  - Initial release.

### Library usage ###

Just add source files to your project.

### Documentation ###

Source code contains documentation comments so it's easy to build docs via [Doxygen](www.doxygen.org).
