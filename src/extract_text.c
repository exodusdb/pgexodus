#include "pgexodus.h"

PG_FUNCTION_INFO_V1(exodus_extract_text);

Datum
exodus_extract_text(PG_FUNCTION_ARGS)
{

	//PG_GETARG_TEXT_P(n) gives you a pointer to the data structure of parameter n
	//VARDATA() gives you a pointer to the data region of a struct.
	//VARSIZE() gives you the total size of the structure
	//VARHDRSZ

	text* output;

#include "getinputstartlength.cpp"
/*
	if (PG_ARGISNULL(0))
	{
		outstart=0;
		outlen=0;
	}
	else
	{
		//get a pointer to the first parameter (0)
		input = PG_GETARG_TEXT_P(0);
		fieldno = PG_GETARG_INT32(1);
		valueno = PG_GETARG_INT32(2);
		subvalueno = PG_GETARG_INT32(3);
		extract(VARDATA(input), VARSIZE(input)-VARHDRSZ, fieldno, valueno, subvalueno, &outstart, &outlen);
	}
*/
//PG_RETURN_NULL();
	//prepare a new output
	//text	   *output = (text *) palloc(VARSIZE(input));
	output = (text *) palloc(VARHDRSZ+(size_t)outlen);

	//set the complete size of the output
	//SET_VARSIZE(output,VARSIZE(input));
	SET_VARSIZE(output,VARHDRSZ+(size_t)outlen);

	//copy the input to the output
	memcpy((void *) VARDATA(output),			// destination
		   (void *) (VARDATA(input)+outstart),	// starting from
		   (size_t)outlen);						// how many bytes

	PG_RETURN_TEXT_P(output);

}

PG_FUNCTION_INFO_V1(exodus_extract_text2);

Datum
exodus_extract_text2(PG_FUNCTION_ARGS)
{

	//PG_GETARG_TEXT_P(n) gives you a pointer to the data structure of parameter n
	//VARDATA() gives you a pointer to the data region of a struct.
	//VARSIZE() gives you the total size of the structure
	//VARHDRSZ

	text *output;
#include "getinputstartlength.cpp"

	//return NULL for zero length string
	if (outlen==0)
		PG_RETURN_NULL();

	//prepare a new output
	//text	   *output = (text *) palloc(VARSIZE(input));
	output = (text *) palloc(VARHDRSZ+(size_t)outlen);

	//set the complete size of the output
	SET_VARSIZE(output,VARSIZE(input));
	SET_VARSIZE(output,VARHDRSZ+(size_t)outlen);

	//copy the input to the output
	memcpy((void *) VARDATA(output),			// destination
		   (void *) (VARDATA(input)+outstart),	// starting from
		   (size_t)outlen);						// how many bytes

	PG_RETURN_TEXT_P(output);

}
