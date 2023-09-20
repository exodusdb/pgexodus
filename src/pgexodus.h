
// Make sure PG MAGIC BLOCK is present in the .so file
//
//  nm pgexodus.so|grep "T Pg_magic_func"
//
// Otherwise when trying to load functions:
//
//  ERROR:  incompatible library "/usr/lib/postgresql/12/lib/pgexodus.so": missing magic block

/*
severely hacked to get around problem building in VS2005
http://www.mail-archive.com/pgsql-general@postgresql.org/msg116145.html
remove w64 compatibility compilation option
and change
static const Pg_magic_struct
to
static Pg_magic_struct
*/

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

/*
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

#if defined(_MSC_VER) || defined(__MINGW32__)
#ifndef _USE_32BIT_TIME_T
#ifndef _WIN64
#define _USE_32BIT_TIME_T
#endif
//to avoid the following warnings that should be removed in postgres 9.1 or 9.2
//c:\program files (x86)\postgresql\9.0\include\server\pg_config_os.h(106): warning C4005: 'EIDRM' : macro redefinition
//          C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\include\errno.h(103) : see previous definition of 'EIDRM'
#pragma warning (disable: 4005)
#endif

//to avoid the following errors in postgres.h below
//error C2011: 'timezone' : 'struct' type redefinition	d:\program files\postgresql\8.3\include\server\pg_config_os.h	188	
//error C2011: 'itimerval' : 'struct' type redefinition	d:\program files\postgresql\8.3\include\server\pg_config_os.h	197	
#ifndef WIN32
#define WIN32
#endif
#endif

/* BUILDING_DLL causes the declarations in Pg's headers to be declared
* __declspec(dllexport) which will break DLL linkage. */
#ifdef BUILDING_DLL
#error Do not define BUILDING_DLL when building extension libraries
#endif

/* Ensure that Pg_module_function and friends are declared __declspec(dllexport) */
#ifndef BUILDING_MODULE
#define BUILDING_MODULE
#endif

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdio.h>
#include <string.h>

#include "postgres.h"
#include "fmgr.h"

/*#include "utils/geo_decls.h"*/
#include <utils/timestamp.h> //for PG_RETURN_TIMESTAMP
#include <utils/date.h> //for PG_RETURN_TIME_ADT

#ifndef int4
#define int4 int32
#endif
int4 pg_atoi(char*,int4,int4);

/*backward compatible to pre 8.3 with no SET_VARSIZE*/
#ifndef SET_VARSIZE
#define SET_VARSIZE(ret,size) VARATT_SIZEP((ret) ) = (size)
#endif

#ifdef _MSC_VER

/*--------------- BEGIN REDEFINITION OF PG MACROS -------------------
*
* These rewritten versions of PG_MODULE_MAGIC and PG_FUNCTION_INFO_V1
* declare the module functions as __declspec(dllexport) when building
* a module. They also provide PGMODULEEXPORT for exporting functions
* in user DLLs.
*/
#undef PG_MODULE_MAGIC
#undef PG_FUNCTION_INFO_V1

/* This might want to go somewhere other than fmgr.h, like
* pg_config_os.h alongside the definition of PGDLLIMPORT
*/
#if defined(_MSC_VER) || defined(__MINGW32__)
#if defined(BUILDING_MODULE)
#define PGMODULEEXPORT __declspec (dllexport)
#else
// Never actually used
#define PGMODULEEXPORT __declspec (dllimport)
#endif
#else
#define PGMODULEEXPORT
#endif

#define PG_MODULE_MAGIC \
PGMODULEEXPORT Pg_magic_struct * \
PG_MAGIC_FUNCTION_NAME(void) \
{ \
static Pg_magic_struct Pg_magic_data = PG_MODULE_MAGIC_DATA; \
return &Pg_magic_data; \
} \
extern int no_such_variable

#define PG_FUNCTION_INFO_V1(funcname) \
PGMODULEEXPORT const Pg_finfo_record * \
CppConcat(pg_finfo_,funcname) (void) \
{ \
static const Pg_finfo_record my_finfo = { 1 }; \
return &my_finfo; \
} \
extern int no_such_variable
#endif
/*--------------- END REDEFINITION OF PG MACROS -------------------*/


//TODO check all pallocs for success

//see http://www.postgresql.org/docs/8.3/interactive/xfunc-c.html

//text_extract2 is like text_extract but returns NULL for empty strings

/* POSTGRES INTERFACE
	PG_ARGISNULL(0) tells you if passed a null
	PG_GETARG_TEXT_P(n) gives you a pointer to the data structure of parameter n
	VARDATA() gives you a pointer to the data region of a struct.
	VARSIZE() gives you the total size of the structure
	VARHDRSZ
	//elog(WARNING, "sizeof picktime8: %d",sizeof(picktime8));
	elog(ERROR, "mvExtractDate cannot convert more than 20 characters to an integer");
	PG_RETURN_NULL();
*/

/* DEBUG USING SOMETHING LIKE THIS
		elog(ERROR, "Debug point xxx");
		//elog(WARNING, "Debug point xxx");
		elog(DEBUG, "Debug point xxx");
		PG_RETURN_NULL();
*/

/* POSTGRES SQL FUNCTION DEFINITION CREATION COPY AND PASTE

NB public functions need to be listed in the .def file on windows.

"STRICT" KEYWORD MEANS THAT THE FUNCTION WILL NOT BE CALLED IF ANY OF THE ARGUMENTS ARE NULL
Note that select statements from exodus are (currently) generated with coalesce function
 to turn nulls into "" in order to retrieve non-existent join records.

LOAD 'pgexodus' without the .dll or .so to load or reload a shared library file NB single quotes
but the above will not allow simple unload or updating the DLL without stopping and restarting postgres on win32

-- All functions are defined as IMMUTABLE although exodus_call could theorectically return anything.
-- First three are not STRICT and return "" in case passed a null

-- cut and paste the following SQL to register the functions into postgres --

CREATE OR REPLACE FUNCTION exodus_count(text, text) RETURNS integer
AS 'pgexodus', 'exodus_count' LANGUAGE C IMMUTABLE;

CREATE OR REPLACE FUNCTION exodus_extract_text(text, int4, int4, int4) RETURNS text
AS 'pgexodus', 'exodus_extract_text' LANGUAGE C IMMUTABLE;

CREATE OR REPLACE FUNCTION exodus_extract_sort(text, int4, int4, int4) RETURNS text
AS 'pgexodus', 'exodus_extract_sort' LANGUAGE C IMMUTABLE;

-- Remaining functions are STRICT therefore never get called with NULLS
-- also return NULL if passed zero length strings

CREATE OR REPLACE FUNCTION exodus_extract_text2(text, int4, int4, int4) RETURNS text
AS 'pgexodus', 'exodus_extract_text2' LANGUAGE C IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION exodus_extract_date(text, int4, int4, int4) RETURNS date
AS 'pgexodus', 'exodus_extract_date' LANGUAGE C IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION exodus_extract_time(text, int4, int4, int4) RETURNS time
AS 'pgexodus', 'exodus_extract_time' LANGUAGE C IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION exodus_extract_datetime(text, int4, int4, int4) RETURNS timestamp
AS 'pgexodus', 'exodus_extract_datetime' LANGUAGE C IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION exodus_extract_number(text, int4, int4, int4) RETURNS float8
AS 'pgexodus', 'exodus_extract_number' LANGUAGE C IMMUTABLE STRICT;

-- CREATE OR REPLACE FUNCTION exodus_call(text, text, text, text, text, int4, int4) RETURNS text
-- AS 'pgexodus', 'exodus_call' LANGUAGE C IMMUTABLE;

-- CREATE OR REPLACE FUNCTION exodus_extract_bytea(bytea, int4, int4, int4)
-- RETURNS bytea AS 'pgexodus', 'exodus_extract_bytea' LANGUAGE C IMMUTABLE;

TO REMOVE THE ABOVE DO THE FOLLOWING AS POSTGRES SUPERUSER CONNECTED TO EXODUS DATABASE

drop FUNCTION exodus_count(text, text) cascade;
drop FUNCTION exodus_extract_text(text, int4, int4, int4) cascade;
drop FUNCTION exodus_extract_sort(text, int4, int4, int4) cascade;
drop FUNCTION exodus_extract_text2(text, int4, int4, int4) cascade;
drop FUNCTION exodus_extract_date(text, int4, int4, int4) cascade;
drop FUNCTION exodus_extract_time(text, int4, int4, int4) cascade;
drop FUNCTION exodus_extract_datetime(text, int4, int4, int4) cascade;
drop FUNCTION exodus_extract_number(text, int4, int4, int4) cascade;
-- drop FUNCTION exodus_call(text, text, text, text, text, int4, int4) cascade;
-- drop FUNCTION exodus_extract_bytea(text, int4, int4, int4) cascade;

*/

/* evade the following warnings from GETINPUTSATRTLENGTH macro
  but it isnt clear why the warning doesnt come in all places that the macro is called
pgexodus.c: In function 'exodus_extract_sort': (GETINPUTSTARTLENGTH)
pgexodus.c:679: warning: 'input' may be used uninitialized in this function
pgexodus.c: In function 'exodus_extract_text': (GETINPUTSTARTLENGTH)
pgexodus.c:462: warning: 'input' may be used uninitialized in this function
*/

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

//extern "C" {

//bool callexodus(const char* serverid, const char* request, const int nrequestbytes, const char* response, int& nresponsebytes);
//bool callexodus(const char* serverid, const char* request, const int nrequestbytes, const char* response, int* nresponsebytes);

void extract(char * instring, int inlength, int fieldno, int valueno, int subvalueno, int* outstart, int* outlength);
