#include "pgexodus.h"

PG_FUNCTION_INFO_V1(exodus_extract_date);

Datum
exodus_extract_date(PG_FUNCTION_ARGS)
{

	//PG_GETARG_TEXT_P(n) gives you a pointer to the data structure of parameter n
	//VARDATA() gives you a pointer to the data region of a struct.
	//VARSIZE() gives you the total size of the structure
	//VARHDRSZ

	int4 pickdate;

	char intstr[21]="12345";

#include "getinputstartlength.cpp"

	//intstr="12345";
	intstr[20]='\0';

	//return NULL for zero length string
	if (outlen==0)
		PG_RETURN_NULL();

	//prepare a c str
	if (outlen>=20)
	{
		//ignore bad data
		//elog(ERROR, "pgexodus exodus_extract_date cannot convert more than 20 characters to an integer date");
		PG_RETURN_NULL();
	}

	memcpy(intstr,			// destination
		   (void *) (VARDATA(input)+outstart),	// starting from
		   (size_t)outlen);						// how many bytes
	intstr[outlen]='\0';

	//convert the c str to an int
	pickdate=outlen;
	//this will error if not a valid integer
	//pickdate=pg_atoi(intstr,4,'.');
	pickdate=atoi(intstr);

	//pick date 0 is 31/12/1967
	//pg date 0 is 31/12/1999
	PG_RETURN_INT32(pickdate-11689);

}
