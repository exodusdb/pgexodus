// Make sure PG MAGIC BLOCK is present in the .so file
//
//  nm pgexodus.so|grep "T Pg_magic_func"
//
// Otherwise when trying to load functions:
//
//  ERROR:  incompatible library "/usr/lib/postgresql/12/lib/pgexodus.so": missing magic block

/*

* PostgreSQL example C functions.
*
* This file must be built as a shared library or dll and
* placed into the PostgreSQL `lib' directory. On Windows
* it must link to postgres.lib .
*
* postgresql/include/server must be on your header search path.
* With MSVC++ on win32 so must postgresql/include/server/port/win32_msvc .
* With MinGW use postgresql/include/server/port/win32 .
*/

/* TODO replace atof used in extract_number.c and extract_datetime.c with something faster and not accepting leading spaces etc.

double atof (const char* str);

Convert string to double
Parses the C string str, interpreting its content as a floating point number and returns its value as a double.

The function first discards as many whitespace characters (as in isspace) as necessary until the first non-whitespace character is found.
Then, starting from this character, takes as many characters as possible that are valid following a syntax resembling that of 
floating point literals (see below), and interprets them as a numerical value.
The rest of the string after the last valid character is ignored and has no effect on the behavior of this function.

    C99/C11 (C++11)

A valid floating point number for atof using the "C" locale is formed by an optional sign character (+ or -), followed by one of:
- A sequence of digits, optionally containing a decimal-point character (.),
  optionally followed by an exponent part (an e or E character followed by an optional sign and a sequence of digits).
- A 0x or 0X prefix, then a sequence of hexadecimal digits (as in isxdigit)
  optionally containing a period which separates the whole and fractional number parts.
  Optionally followed by a power of 2 exponent (a p or P character followed by an optional sign and a sequence of hexadecimal digits).
- INF or INFINITY (ignoring case).
- NAN or NANsequence (ignoring case), where sequence is a sequence of characters, where each character is either an alphanumeric character (as in isalnum) or the underscore character (_).

If the first sequence of non-whitespace characters in str does not form a valid floating-point number as just defined,
or if no such sequence exists because either str is empty or contains only whitespace characters,
no conversion is performed and the function returns 0.0.

Parameters

str
    C-string beginning with the representation of a floating-point number.

Return Value

On success, the function returns the converted floating point number as a double value.
If no valid conversion could be performed, the function returns zero (0.0).
If the converted value would be out of the range of representable values by a double,
it causes undefined behavior. See strtod for a more robust cross-platform alternative when this is a possibility.
*/

///* Ensure that Pg_module_function and friends are declared __declspec(dllexport) */
//#ifndef BUILDING_MODULE
//#define BUILDING_MODULE
//#endif

#include <stdio.h>
#include <string.h>

#include <postgres.h>
#if __has_include(<varatt.h>)
//VARSIZE etc. was split out from postgres.h in Postgres V16+
#include <varatt.h>
#endif
#include <fmgr.h>

#include <utils/timestamp.h> //for PG_RETURN_TIMESTAMP
#include <utils/date.h> //for PG_RETURN_TIME_ADT

#ifndef int4
#define int4 int32
#endif
//int4 pg_atoi(char*,int4,int4);

//TODO check all pallocs for success

//see http://www.postgresql.org/docs/8.3/interactive/xfunc-c.html

//text_extract2 is like text_extract but returns NULL for empty strings

/* DEBUG USING SOMETHING LIKE THIS
		elog(ERROR, "Debug point xxx");
		//elog(WARNING, "Debug point xxx");
		elog(DEBUG, "Debug point xxx");
		PG_RETURN_NULL();
*/

// Call extract.c on data, fn, vn, sn and return the start and length of the desired field/value/subvalue
//
#define GETINPUTSTARTLENGTH\
	text *input;\
	int32 outstart;\
	int32 outlen;\
	int32 fieldno;\
	int32 valueno;\
	int32 subvalueno;\
	if (PG_ARGISNULL(0))\
	{\
		outstart=0;\
		outlen=0;\
		input=0; /* evade warning: 'input' may be used unitialized */\
	}\
	else\
	{\
		/*get a pointer to the first parameter (0)*/\
		input = PG_GETARG_TEXT_P(0);\
		fieldno = PG_GETARG_INT32(1);\
		valueno = PG_GETARG_INT32(2);\
		subvalueno = PG_GETARG_INT32(3);\
		extract(VARDATA(input), (int)VARSIZE(input)-VARHDRSZ, (int)fieldno, (int)valueno, (int)subvalueno, &outstart, &outlen);\
	}\

void extract(char * instring, int inlength, int fieldno, int valueno, int subvalueno, int* outstart, int* outlength);
