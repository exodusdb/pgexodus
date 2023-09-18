DROP FUNCTION IF EXISTS exodus.extract_text(text, int4, int4, int4);
DROP FUNCTION IF EXISTS exodus.extract_date(text, int4, int4, int4);
DROP FUNCTION IF EXISTS exodus.extract_time(text, int4, int4, int4);
DROP FUNCTION IF EXISTS exodus.extract_datetime(text, int4, int4, int4);
DROP FUNCTION IF EXISTS exodus.extract_number(text, int4, int4, int4);
DROP FUNCTION IF EXISTS exodus.count(text, text);

CREATE OR REPLACE FUNCTION exodus.extract_text(data text, fn int4, vn int4, sn int4)     RETURNS text      AS 'pgexodus', 'exodus_extract_text'     LANGUAGE C IMMUTABLE;
-- returns zero for zero length strings or NULLS
CREATE OR REPLACE FUNCTION exodus.extract_number(data text, fn int4, vn int4, sn int4)   RETURNS float8    AS 'pgexodus', 'exodus_extract_number'   LANGUAGE C IMMUTABLE;
CREATE OR REPLACE FUNCTION exodus.count(data text, countchar text)                       RETURNS integer   AS 'pgexodus', 'exodus_count'            LANGUAGE C IMMUTABLE;
-- Remaining functions are STRICT therefore never get called with NULLS -- also return NULL if passed zero length strings
CREATE OR REPLACE FUNCTION exodus.extract_date(data text, fn int4, vn int4, sn int4)     RETURNS date      AS 'pgexodus', 'exodus_extract_date'     LANGUAGE C IMMUTABLE STRICT;
CREATE OR REPLACE FUNCTION exodus.extract_time(data text, fn int4, vn int4, sn int4)     RETURNS interval  AS 'pgexodus', 'exodus_extract_time'     LANGUAGE C IMMUTABLE STRICT;
CREATE OR REPLACE FUNCTION exodus.extract_datetime(data text, fn int4, vn int4, sn int4) RETURNS timestamp AS 'pgexodus', 'exodus_extract_datetime' LANGUAGE C IMMUTABLE STRICT;

CREATE ROLE exodus;
ALTER SCHEMA exodus OWNER TO exodus;

-- GRANT EXECUTE ON FUNCTION exodus.extract_text(text, integer, integer, integer)     TO PUBLIC;
-- GRANT EXECUTE ON FUNCTION exodus.extract_date(text, integer, integer, integer)     TO PUBLIC;
-- GRANT EXECUTE ON FUNCTION exodus.extract_time(text, integer, integer, integer)     TO PUBLIC;
-- GRANT EXECUTE ON FUNCTION exodus.extract_datetime(text, integer, integer, integer) TO PUBLIC;
-- GRANT EXECUTE ON FUNCTION exodus.extract_number(text, integer, integer, integer)   TO PUBLIC;
-- GRANT EXECUTE ON FUNCTION exodus.count(text, text)                                 TO PUBLIC;
ALTER FUNCTION exodus.extract_text(text, integer, integer, integer)     OWNER TO exodus;
ALTER FUNCTION exodus.extract_date(text, integer, integer, integer)     OWNER TO exodus;
ALTER FUNCTION exodus.extract_time(text, integer, integer, integer)     OWNER TO exodus;
ALTER FUNCTION exodus.extract_datetime(text, integer, integer, integer) OWNER TO exodus;
ALTER FUNCTION exodus.extract_number(text, integer, integer, integer)   OWNER TO exodus;
ALTER FUNCTION exodus.count(text, text)                                 OWNER TO exodus;
