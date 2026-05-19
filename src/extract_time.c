#include "pgexodus.h"

// Extract time as postgresql interval
// so that we can handle times like 25:00 which fall into the following day
PG_FUNCTION_INFO_V1(exodus_extract_time);

Datum
exodus_extract_time(PG_FUNCTION_ARGS)
{

	//PG_GETARG_TEXT_PP(n) gives you a pointer to the data structure of parameter n
	//VARDATA_ANY() gives you a pointer to the data region of a struct.
	//VARSIZE() gives you the total size of the structure
	//VARHDRSZ

	char intstr[21];
	int32 picktime;
	Interval *output;

	#include "getinputstartlength.c"

	// Return NULL for zero length string
	if (outlen==0)
		PG_RETURN_NULL();

	// Prepare a c str
	if (outlen>=20)
	{
		// Ignore bad data
		// elog(ERROR, "pgexodus exodus_extract_time cannot convert more than 20 characters to an integer");
		PG_RETURN_NULL();
	}
	intstr[20]='\0';
	memcpy(intstr,                                 // destination
		   (void *) (VARDATA_ANY(input)+outstart), // starting from
		   (size_t)outlen);                        // how many bytes
	intstr[outlen]='\0';

	// Convert the c str to an int32
	// This will error if not a valid integer
	// picktime=pg_atoi(intstr,4,'.');
	picktime=atoi(intstr);

	output = (Interval *) palloc(sizeof(Interval));

	output->month=0;
	output->day=0;
	output->time=picktime;
#	ifdef HAVE_INT64_TIMESTAMP
		output->time=output->time*1000000;
#	endif

	PG_RETURN_INTERVAL_P(output);
}