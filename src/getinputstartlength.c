
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
		input=0; /* evade warning: 'input' may be used unitialized */
	}
	else
	{
		/*get a pointer to the first parameter (0)*/
		input = PG_GETARG_TEXT_P(0);
		fieldno = PG_GETARG_INT32(1);
		valueno = PG_GETARG_INT32(2);
		subvalueno = PG_GETARG_INT32(3);
		extract(VARDATA(input), (int)VARSIZE(input)-VARHDRSZ, (int)fieldno, (int)valueno, (int)subvalueno, &outstart, &outlen);
	}
