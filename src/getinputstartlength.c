
	text *input;
	int32 outstart;
	int32 outlen;
	int32 fieldno;
	int32 valueno;
	int32 subvalueno;
	if (PG_ARGISNULL(0))
	{
		outstart=0;
		outlen=0;
		// Evade warning: 'input' may be used uninitialized
		input=0;
	}
	else
	{
		// Get a pointer to the first parameter (0)
		input = PG_GETARG_TEXT_PP(0);
		fieldno = PG_GETARG_INT32(1);
		valueno = PG_GETARG_INT32(2);
		subvalueno = PG_GETARG_INT32(3);

		int32 input_len = VARSIZE_ANY_EXHDR(input);
		extract(VARDATA_ANY(input), input_len, fieldno, valueno, subvalueno, &outstart, &outlen);
	}
