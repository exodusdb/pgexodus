#include <stdio.h>
#include <string.h>

//#warning: 'format' attribute argument not supported: gnu_printf [-Wignored-attributes]
#if __clang_major__ >= 12
#	pragma clang diagnostic ignored "-Wignored-attributes"
#endif
#include <postgres.h>

#if __has_include(<varatt.h>)
//	VARSIZE etc. was split out from postgres.h in Postgres V16+
#	include <varatt.h>
#endif
#include <fmgr.h>

#include <utils/timestamp.h> //for PG_RETURN_TIMESTAMP
#include <utils/date.h> //for PG_RETURN_TIME_ADT

//TODO check all pallocs for success

//text_extract2 is like text_extract but returns NULL for empty strings

void extract(char * instring, int32 inlength, int32 fieldno, int32 valueno, int32 subvalueno, int32* outstart, int32* outlength);

/* DEBUG USING SOMETHING LIKE THIS
		elog(ERROR, "Debug point xxx");
		//elog(WARNING, "Debug point xxx");
		elog(DEBUG, "Debug point xxx");
		PG_RETURN_NULL();
*/

