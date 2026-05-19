#include "pgexodus.h"

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

PG_FUNCTION_INFO_V1(exodus_extract_number);

Datum
exodus_extract_number(PG_FUNCTION_ARGS)
{

	//PG_GETARG_TEXT_PP(n) gives you a pointer to the data structure of parameter n
	//VARDATA_ANY() gives you a pointer to the data region of a struct.
	//VARSIZE() gives you the total size of the structure
	//VARHDRSZ

	double doublenum;
	char doublestr[21];

	#include "getinputstartlength.c"

	// Return NULL for zero length string
	if (outlen==0)
		PG_RETURN_FLOAT8(0);

	// Prepare a c str
	if (outlen>=20)
	{
		// Ignore bad data
		// elog(ERROR, "pgexodus exodus_extract_number cannot convert more than 20 characters to an integer");
		PG_RETURN_NULL();
	}
	doublestr[20]='\0';
	memcpy(doublestr,                              // destination
		   (void *) (VARDATA_ANY(input)+outstart), // starting from
		   (size_t)outlen);                        // how many bytes
	doublestr[outlen]='\0';

	// Convert the c str to an double
	doublenum=atof(doublestr);

	PG_RETURN_FLOAT8(doublenum);
}
