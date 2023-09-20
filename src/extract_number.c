#include "pgexodus.h"

PG_FUNCTION_INFO_V1(exodus_extract_number);

Datum
exodus_extract_number(PG_FUNCTION_ARGS)
{

	//PG_GETARG_TEXT_P(n) gives you a pointer to the data structure of parameter n
	//VARDATA() gives you a pointer to the data region of a struct.
	//VARSIZE() gives you the total size of the structure
	//VARHDRSZ

	double doublenum;
	char doublestr[21];

	GETINPUTSTARTLENGTH

	//return NULL for zero length string
	if (outlen==0)
		PG_RETURN_FLOAT8(0);

	//prepare a c str
	if (outlen>=20)
	{
		//ignore bad data
		//elog(ERROR, "pgexodus exodus_extract_number cannot convert more than 20 characters to an integer");
		PG_RETURN_NULL();
	}
	doublestr[20]='\0';
	memcpy(doublestr,			// destination
		   (void *) (VARDATA(input)+outstart),	// starting from
		   (size_t)outlen);						// how many bytes
	doublestr[outlen]='\0';

	//convert the c str to an double
	doublenum=atof(doublestr);

	PG_RETURN_FLOAT8(doublenum);
}
