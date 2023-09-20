#include "pgexodus.h"

PG_FUNCTION_INFO_V1(exodus_count);

Datum
exodus_count(PG_FUNCTION_ARGS)
{
	text *arg1 = PG_GETARG_TEXT_P(0);
	text *arg2 = PG_GETARG_TEXT_P(1);

	char* instring=VARDATA(arg1);
	int nn=(int)VARSIZE(arg1);

	//only count the 1st char of the sep at the moment
	int sepchar=*VARDATA(arg2);
	int count=0;
	for (int ii=0;ii<nn;++ii)
	{
		if (instring[ii] == sepchar)
			count++;
	}

	PG_RETURN_INT32(count);
}
