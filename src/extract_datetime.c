#include "pgexodus.h"

// See extract_number.c for discussion about atof

PG_FUNCTION_INFO_V1(exodus_extract_datetime);

Datum
exodus_extract_datetime(PG_FUNCTION_ARGS)
{

	//PG_GETARG_TEXT_PP(n) gives you a pointer to the data structure of parameter n
	//VARDATA_ANY() gives you a pointer to the data region of a struct.
	//VARSIZE() gives you the total size of the structure
	//VARHDRSZ

	Timestamp pickdatetime;
	char datetimestr[21];

	#include "getinputstartlength.c"

	// Return NULL for zero length string
	if (outlen==0)
		PG_RETURN_NULL();

	// Prepare a c str
	if (outlen>=20)
	{
		// Ignore bad data
		// elog(ERROR, "pgexodus exodus_extract_datetime cannot convert more than 20 characters to an integer");
		PG_RETURN_NULL();
	}
	datetimestr[20]='\0';
	memcpy(datetimestr,                            // destination
		   (void *) (VARDATA_ANY(input)+outstart), // starting from
		   (size_t)outlen);                        // how many bytes
	datetimestr[outlen]='\0';

	// Convert the c str to an double

#ifdef HAVE_INT64_TIMESTAMP
    // Number of microseconds before or after midnight 1/1/2000?
	pickdatetime=(int64)(atof(datetimestr)-11689)*86400000000LL;
#else
    // Number of seconds before or after midnight 1/1/2000
	pickdatetime=(atof(datetimestr)-11689)*86400;
#endif

	PG_RETURN_TIMESTAMP(pickdatetime);
}

