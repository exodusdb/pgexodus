#include "pgexodus.h"

PG_FUNCTION_INFO_V1(exodus_extract_text);

Datum
exodus_extract_text(PG_FUNCTION_ARGS)
{

	//PG_GETARG_TEXT_PP(n) gives you a pointer to the data structure of parameter n
	//VARDATA_ANY() gives you a pointer to the data region of a struct.
	//VARSIZE() gives you the total size of the structure
	//VARHDRSZ

	text* output;

	#include "getinputstartlength.c"

    // Optional optimization: fast path for empty result (common case!)
    if (outlen <= 0) {
        output = (text *) palloc(VARHDRSZ);
        SET_VARSIZE(output, VARHDRSZ);
        PG_RETURN_TEXT_P(output);
    }

	//PG_RETURN_NULL();
	// Prepare a new output
	output = (text *) palloc(VARHDRSZ+(size_t)outlen);

	// Set the complete size of the output
	//SET_VARSIZE(output,VARSIZE(input));
	SET_VARSIZE(output,VARHDRSZ+(size_t)outlen);

	// Copy the input to the output
	memcpy((void *) VARDATA_ANY(output),           // destination
		   (void *) (VARDATA_ANY(input)+outstart), // starting from
		   (size_t)outlen);                        // how many bytes

	PG_RETURN_TEXT_P(output);

}
