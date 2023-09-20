#include "pgexodus.h"

PG_FUNCTION_INFO_V1(exodus_extract_bytea);

Datum
exodus_extract_bytea(PG_FUNCTION_ARGS)
{


	/*very similar to GETINPUTSTARTLENGTH macro but bytea instead of text*/

	int32 outstart;
	int32 outlen;
	int32 fieldno;
	int32 valueno;
	int32 subvalueno;

	bytea* input;
	bytea* output;

	//get a pointer to the first parameter (0)
	input = PG_GETARG_BYTEA_P(0);

	fieldno = PG_GETARG_INT32(1);

	valueno = PG_GETARG_INT32(2);

	subvalueno = PG_GETARG_INT32(3);

	extract(VARDATA(input), (int)VARSIZE(input)-VARHDRSZ, fieldno, valueno, subvalueno, &outstart, &outlen);


	//prepare a new output
	//bytea	   *output = (bytea *) palloc(VARSIZE(input));
	output = (bytea *) palloc(VARHDRSZ+(size_t)outlen);

	//set the complete size of the output
	SET_VARSIZE(output,VARSIZE(input));
	SET_VARSIZE(output,VARHDRSZ+(size_t)outlen);

	//copy the input to the output
	memcpy((void *) VARDATA(output),			// destination
		   (void *) (VARDATA(input)+outstart),	// starting from
		   (size_t)outlen);						// how many bytes

	PG_RETURN_BYTEA_P(output);

}
