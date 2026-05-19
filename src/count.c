//#include "postgres.h"
//#include "fmgr.h"
//#include "utils/bytea.h"  // for bytea functions if needed
#include "pgexodus.h"

PG_FUNCTION_INFO_V1(exodus_count);

Datum
exodus_count(PG_FUNCTION_ARGS)
{
    // Handle NULL inputs
    if (PG_ARGISNULL(0) || PG_ARGISNULL(1))
        PG_RETURN_INT32(0);

    text *txt_data = PG_GETARG_TEXT_PP(0);   // input string
    text *txt_find = PG_GETARG_TEXT_PP(1);   // substring to count

    int32 len_data = VARSIZE_ANY_EXHDR(txt_data);
    int32 len_find = VARSIZE_ANY_EXHDR(txt_find);

    char *data = VARDATA_ANY(txt_data);
    char *find = VARDATA_ANY(txt_find);

    // If find string is empty, standard SQL behavior is to return 0 (or sometimes 1 + length, but 0 is safer)
    if (len_find <= 0)
        PG_RETURN_INT32(0);

    // If data is shorter than find string, no match possible
    if (len_data < len_find)
        PG_RETURN_INT32(0);

    // === Fast path for single byte (e.g. \x1D) ===
    if (len_find == 1)
    {
        int32 count = 0;
        char c = *find;
        for (int32 i = 0; i < len_data; ++i)
        {
            if (data[i] == c)
                count++;
        }
        PG_RETURN_INT32(count);
    }

    int count = 0;
    int max_start = len_data - len_find;  // last possible start position

    for (int i = 0; i <= max_start; ++i)
    {
        if (memcmp(data + i, find, len_find) == 0)
        {
            count++;
            // Comment the next line if you want OVERLAPPING matches (e.g. "aaa".count("aa") = 2)
            i += len_find - 1;  // skip ahead to allow overlap
            // For non-overlapping (standard str.count() in Python, strpos loop), keep i++:
            // just continue with i++ (default loop behavior)
        }
    }

    PG_RETURN_INT32(count);
}
