#include "pgexodus.h"

PG_FUNCTION_INFO_V1(exodus_extract_datetime);

Datum
exodus_extract_datetime(PG_FUNCTION_ARGS)
{

	//PG_GETARG_TEXT_P(n) gives you a pointer to the data structure of parameter n
	//VARDATA() gives you a pointer to the data region of a struct.
	//VARSIZE() gives you the total size of the structure
	//VARHDRSZ

	Timestamp pickdatetime;
	char datetimestr[21];

#include "getinputstartlength.cpp"

	//return NULL for zero length string
	if (outlen==0)
		PG_RETURN_NULL();

	//prepare a c str
	if (outlen>=20)
	{
		//ignore bad data
		//elog(ERROR, "pgexodus exodus_extract_datetime cannot convert more than 20 characters to an integer");
		PG_RETURN_NULL();
	}
	datetimestr[20]='\0';
	memcpy(datetimestr,			// destination
		   (void *) (VARDATA(input)+outstart),	// starting from
		   (size_t)outlen);						// how many bytes
	datetimestr[outlen]='\0';

	//convert the c str to an double

#ifdef HAVE_INT64_TIMESTAMP
    //number of microseconds before or after midnight 1/1/2000?
	pickdatetime=(long long int)(atof(datetimestr)-11689)*86400000000LL;
#else
    //number of seconds before or after midnight 1/1/2000
	pickdatetime=(atof(datetimestr)-11689)*86400;
#endif

	PG_RETURN_TIMESTAMP(pickdatetime);
}

