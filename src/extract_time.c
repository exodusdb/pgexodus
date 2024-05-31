#include "pgexodus.h"

//time extraction as interval so that we can handle times like 25:00 which fall into the following day
#if 1
PG_FUNCTION_INFO_V1(exodus_extract_time);

Datum
exodus_extract_time(PG_FUNCTION_ARGS)
{

	//PG_GETARG_TEXT_P(n) gives you a pointer to the data structure of parameter n
	//VARDATA() gives you a pointer to the data region of a struct.
	//VARSIZE() gives you the total size of the structure
	//VARHDRSZ

	char intstr[21];
	int4 picktime;
	Interval *output;

#include "getinputstartlength.cpp"

	//return NULL for zero length string
	if (outlen==0)
		PG_RETURN_NULL();

	//prepare a c str
	if (outlen>=20)
	{
		//ignore bad data
		//elog(ERROR, "pgexodus exodus_extract_time cannot convert more than 20 characters to an integer");
		PG_RETURN_NULL();
	}
	intstr[20]='\0';
	memcpy(intstr,			// destination
		   (void *) (VARDATA(input)+outstart),	// starting from
		   (size_t)outlen);						// how many bytes
	intstr[outlen]='\0';

	//convert the c str to an int
	//this will error if not a valid integer
	//picktime=pg_atoi(intstr,4,'.');
	picktime=atoi(intstr);

    //#define SIZEOFINTERVAL 12
    #define SIZEOFINTERVAL (int)(sizeof(Interval*))
	//prepare a new output
	//text	   *output = (text *) palloc(VARSIZE(input));
	output = (Interval *) palloc(VARHDRSZ+SIZEOFINTERVAL);

	output->month=0;
	output->day=0;
	output->time=picktime;
#	ifdef HAVE_INT64_TIMESTAMP
		output->time=output->time*1000000;
#	endif

	PG_RETURN_INTERVAL_P(output);
}

#else
//returning time as time - not used because cant handle times like 25:00
PG_FUNCTION_INFO_V1(exodus_extract_time);

Datum
exodus_extract_time(PG_FUNCTION_ARGS)
{

	//PG_GETARG_TEXT_P(n) gives you a pointer to the data structure of parameter n
	//VARDATA() gives you a pointer to the data region of a struct.
	//VARSIZE() gives you the total size of the structure
	//VARHDRSZ

	int64 picktime;

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
		//elog(ERROR, "pgexodus exodus_extract_time cannot convert more than 20 characters to an integer time");
		PG_RETURN_NULL();
	}

	memcpy(intstr,			// destination
		   (void *) (VARDATA(input)+outstart),	// starting from
		   (size_t)outlen);						// how many bytes
	intstr[outlen]='\0';

	//convert the c str to an int
	//picktime=outlen;
	//this will error if not a valid integer
	//picktime=pg_atoi(intstr,4,'.');
	picktime=atoi(intstr);
	picktime*=1000000;
	//picktime=1000000;//1 second in microseconds
	//pick date 0 is 0-86399 (seconds)
	//pg date 0 is 0-86399999999 (microseconds)
	PG_RETURN_INT64(picktime);
//	PG_RETURN_TIMEADT(picktime);

}
#endif

