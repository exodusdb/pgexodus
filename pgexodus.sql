DO $$
BEGIN
   RAISE INFO '
Creating extension pgexodus using /usr/share/postgresql/NN/extension/pgexodus--1.0.sql origin exodus/pgexodus/pgexodus.sql
─────────────────────────────────────────────────────────────────────────────────';
END
$$;
-- =================================
-- Create the @EXTENSION_NAME@ @EXTENSION_VERSION@ functions
-- =================================
--
-- Creates the following - in whatever database the extension is being created in.
--
-- 1. Schema exodus
-- 2. Role exodus
-- 3. Functions exodus.extract_text etc.
--
-- and tests that the functions work.

-- https://www.postgresql.org/docs/current/extend-extensions.html

\echo Use "CREATE EXTENSION @EXTENSION_NAME@" to load this file because its objects should be owned by it so that they can all be created and deleted together.\quit

-- Create schema
----------------

	CREATE SCHEMA IF NOT EXISTS exodus;

-- A function to create a role if not exists
--------------------------------------------

	CREATE OR REPLACE FUNCTION exodus.create_role_if_not_exists(role_name TEXT) RETURNS VOID AS $$
	BEGIN
		IF NOT EXISTS (SELECT 1 FROM pg_roles WHERE rolname = role_name) THEN
			EXECUTE 'CREATE ROLE ' || role_name;
		END IF;
	END;
	$$ LANGUAGE plpgsql;

-- Role Exodus
--------------

	SELECT exodus.create_role_if_not_exists('exodus');
	DROP FUNCTION exodus.create_role_if_not_exists(role_name TEXT);

	ALTER SCHEMA exodus OWNER TO exodus;

-- Drop any existing functions - why?
------------------------------

	DROP FUNCTION IF EXISTS exodus.extract_text(text, int4, int4, int4);
	DROP FUNCTION IF EXISTS exodus.extract_date(text, int4, int4, int4);
	DROP FUNCTION IF EXISTS exodus.extract_time(text, int4, int4, int4);
	DROP FUNCTION IF EXISTS exodus.extract_datetime(text, int4, int4, int4);
	DROP FUNCTION IF EXISTS exodus.extract_number(text, int4, int4, int4);
	DROP FUNCTION IF EXISTS exodus.count(text, text);
	DROP FUNCTION IF EXISTS exodus.toBool(numeric);
	DROP FUNCTION IF EXISTS exodus.toBool(text);

-- Create the C functions - obsolete
------------------------------------

--	CREATE OR REPLACE FUNCTION exodus.extract_text(data text, fn int4, vn int4, sn int4)     RETURNS text      AS '@EXTENSION_NAME@', 'exodus_extract_text'     LANGUAGE C IMMUTABLE;
--	-- returns zero for zero length strings or NULLS
--	CREATE OR REPLACE FUNCTION exodus.extract_number(data text, fn int4, vn int4, sn int4)   RETURNS float8    AS '@EXTENSION_NAME@', 'exodus_extract_number'   LANGUAGE C IMMUTABLE;
--	CREATE OR REPLACE FUNCTION exodus.count(data text, countchar text)                       RETURNS integer   AS '@EXTENSION_NAME@', 'exodus_count'            LANGUAGE C IMMUTABLE;
--	-- Remaining functions are STRICT therefore never get called with NULLS -- also return NULL if passed zero length strings
--	CREATE OR REPLACE FUNCTION exodus.extract_date(data text, fn int4, vn int4, sn int4)     RETURNS date      AS '@EXTENSION_NAME@', 'exodus_extract_date'     LANGUAGE C IMMUTABLE STRICT;
--	CREATE OR REPLACE FUNCTION exodus.extract_time(data text, fn int4, vn int4, sn int4)     RETURNS interval  AS '@EXTENSION_NAME@', 'exodus_extract_time'     LANGUAGE C IMMUTABLE STRICT;
--	CREATE OR REPLACE FUNCTION exodus.extract_datetime(data text, fn int4, vn int4, sn int4) RETURNS timestamp AS '@EXTENSION_NAME@', 'exodus_extract_datetime' LANGUAGE C IMMUTABLE STRICT;

-- Create the functions
-----------------------

-- extract_text
-- ------------
-- NO STRICT - CAN BE CALLED WITH NULL
-- RETURNS "" FOR NULL INPUT

CREATE OR REPLACE FUNCTION exodus.extract_text(data text, fn integer, vn integer, sn integer)
	RETURNS text
	LANGUAGE plperl
	IMMUTABLE
AS $$
	my ($data, $fn, $vn, $sn) = @_;
	return '' if !defined $data;

	return $data if !defined $fn || $fn < 1;

	# Field level
	my $pos = 0;
	for (1 .. $fn-1) {
		$pos = index($data, "\x1E", $pos) + 1;
		return '' if $pos == 0;
	}
	my $end = index($data, "\x1E", $pos);
	my $field = ($end == -1) ? substr($data, $pos) : substr($data, $pos, $end - $pos);

	return $field if $vn < 1;

	# Value level
	$pos = 0;
	for (1 .. $vn-1) {
		$pos = index($field, "\x1D", $pos) + 1;
		return '' if $pos == 0;
		$field = substr($field, $pos);
	}
	$end = index($field, "\x1D", 0);
	my $val = ($end == -1) ? $field : substr($field, 0, $end);

	return $val if $sn < 1;

	# Subvalue level
	$pos = 0;
	for (1 .. $sn-1) {
		$pos = index($val, "\x1C", $pos) + 1;
		return '' if $pos == 0;
		$val = substr($val, $pos);
	}
	$end = index($val, "\x1C", 0);
	return ($end == -1) ? $val : substr($val, 0, $end);
$$;

-- count
-- ------------
-- NOT STRICT - CAN BE CALLED WITH NULLS - RETURNS 0
-- NEVER RETURNS NULL
-- plperl FOR PERFORMANCE

CREATE OR REPLACE FUNCTION exodus.count(data text, find text)
	RETURNS integer
	LANGUAGE plperl
	IMMUTABLE
AS $$
	my ($data, $find) = @_;
	return 0 if !defined $data || $data eq '' || !defined $find || $find eq '';

	my $flen = length($find);
	my $count = 0;
	my $pos = 0;

	if ($flen == 1) {
		my $c = substr($find, 0, 1);
		while (1) {
			$pos = index($data, $c, $pos);
			last if $pos == -1;
			$count++;
			$pos += 1;
		}
	} else {
		while (1) {
			$pos = index($data, $find, $pos);
			last if $pos == -1;
			$count++;
			$pos += $flen;
		}
	}
	return $count;
$$;

-- extract_number
-- --------------
-- NO STRICT - CAN BE CALLED WITH NULL
-- RETURNS 0 FOR NULL OR "" INPUT
-- CAN RETURN 0 FOR NON-NUMERIC

CREATE OR REPLACE FUNCTION exodus.extract_number(data text, fn integer, vn integer, sn integer)
	RETURNS double precision
	LANGUAGE plpgsql
	IMMUTABLE
AS $$
DECLARE
    ans text;
BEGIN
    ans := exodus.extract_text(data, fn, vn, sn);
    IF ans = '' THEN
        RETURN 0;
    END IF;

    BEGIN
        RETURN ans::float8;
    EXCEPTION WHEN others THEN
        IF current_setting('exodus.allow_non_numeric', true) = 'false' THEN
            RAISE EXCEPTION 'non-numeric value: "%"', ans;
        ELSE
            RAISE WARNING 'non-numeric value: "%"', ans;
            RETURN 0;
        END IF;
    END;
END;
$$;

-- extract_date
-- ------------
-- STRICT - CANNOT BE CALLED WITH NULL
-- RETURNS NULL FOR "" INPUT
-- CAN RETURN EPOC START FOR NON-NUMERIC

CREATE OR REPLACE FUNCTION exodus.extract_date(data text, fn integer, vn integer, sn integer)
	RETURNS date
	LANGUAGE plpgsql
	IMMUTABLE
	STRICT
	AS $$
DECLARE
    ans text;
BEGIN
    ans := exodus.extract_text(data, fn, vn, sn);
    IF ans = '' THEN
        RETURN NULL;
    END IF;

    BEGIN
        RETURN '1967-12-31'::date + trunc(ans::float8)::int;
    EXCEPTION WHEN others THEN
        IF current_setting('exodus.allow_non_numeric', true) = 'false' THEN
            RAISE EXCEPTION 'non-numeric value for date: "%"', ans;
        ELSE
--            RAISE WARNING 'non-numeric value for date: "%"', ans;
--            RETURN '1967-12-31'::date;
			RETURN NULL;
        END IF;
    END;
END;
$$;

-- extract_time
-- ------------
-- STRICT - CANNOT BE CALLED WITH NULL
-- RETURNS NULL FOR "" INPUT
-- CAN RETURN 00:00:00 TIME INTERVAL FOR NON-NUMERIC

CREATE OR REPLACE FUNCTION exodus.extract_time(data text, fn integer, vn integer, sn integer)
	RETURNS interval
	LANGUAGE plpgsql
	IMMUTABLE
	STRICT
	AS $$
DECLARE
    ans text;
BEGIN
    ans := exodus.extract_text(data, fn, vn, sn);
    IF ans = '' THEN
        RETURN NULL;
    END IF;

    BEGIN
        RETURN make_interval(secs => trunc(ans::float8)::int);
    EXCEPTION WHEN others THEN
        IF current_setting('exodus.allow_non_numeric', true) = 'false' THEN
            RAISE EXCEPTION 'non-numeric value for time: "%"', ans;
        ELSE
            RAISE WARNING 'non-numeric value for time: "%"', ans;
            RETURN '00:00:00'::interval;
        END IF;
    END;
END;
$$;

-- extract_datetime
-- ----------------
-- STRICT - CANNOT BE CALLED WITH NULL
-- RETURNS NULL FOR "" INPUT
-- CAN RETURN EPOC START TIMESTAMP FOR NON-NUMERIC

CREATE OR REPLACE FUNCTION exodus.extract_datetime(data text, fn integer, vn integer, sn integer)
	RETURNS timestamp without time zone
	LANGUAGE plpgsql
	IMMUTABLE
	STRICT
	AS $$
DECLARE
    ans text;
BEGIN
    ans := exodus.extract_text(data, fn, vn, sn);
    IF ans = '' THEN
        RETURN NULL;
    END IF;

    BEGIN
--      RETURN to_timestamp((split_part(ans,'.',1)::int-732)*86400 + split_part(ans,'.',2)::int) AT TIME ZONE 'UTC';
        RETURN to_timestamp((trunc(COALESCE(NULLIF(split_part(ans, '.', 1), '')::float8, 0))::int-732)*86400 + trunc(COALESCE(NULLIF(split_part(ans, '.', 2), '')::float8, 0))::int) AT TIME ZONE 'UTC';
    EXCEPTION WHEN others THEN
        IF current_setting('exodus.allow_non_numeric', true) = 'false' THEN
            RAISE EXCEPTION 'non-numeric value for datetime: "%"', ans;
        ELSE
            RAISE WARNING 'non-numeric value for datetime: "%"', ans;
            RETURN '1967-12-31 00:00:00'::timestamp;
        END IF;
    END;
END;
$$;

-- toBool from numeric
-- -------------------
-- STRICT - CANNOT BE CALLED WITH NULL
-- ALWAYS RETURN TRUE OR FALSE

CREATE OR REPLACE FUNCTION exodus.tobool(innum numeric)
	RETURNS boolean
	LANGUAGE plpgsql
	IMMUTABLE
	STRICT
	COST 10
	AS $$
BEGIN
	return $1 != 0;
END;
$$;

-- toBool from text
-- -------------------
-- STRICT - CANNOT BE CALLED WITH NULL
-- ALWAYS RETURN TRUE OR FALSE

CREATE OR REPLACE FUNCTION exodus.tobool(instring text)
	RETURNS boolean
	LANGUAGE plpgsql
	IMMUTABLE
	STRICT
	COST 10
	AS $$
DECLARE
    ch text;
BEGIN
    -- Pickos's '', is numeric 0 and false unlike pgsql
    IF instring = '' THEN RETURN FALSE; END IF;
    
    -- Leading non-numeric is true
    ch := left(instring, 1);
    IF ch < '0' OR ch > '9' THEN
        IF ch NOT IN ('-', '.') THEN
            RETURN TRUE;
        END IF;
    END IF;
    
    -- Trailing non-numeric is true
    ch := right(instring, 1);
    IF ch < '0' OR ch > '9' THEN
        IF ch <> '.' THEN
            RETURN TRUE;
        END IF;
    END IF;
    
    -- Numeric zero is false, anything else is true
    RETURN instring::numeric <> 0;

EXCEPTION WHEN others THEN RETURN TRUE;  -- if not numeric (and not empty) then true
END;
$$;

-- Change ownership
-------------------

	-- GRANT EXECUTE ON FUNCTION exodus.extract_text(text, integer, integer, integer)     TO PUBLIC;
	-- GRANT EXECUTE ON FUNCTION exodus.extract_date(text, integer, integer, integer)     TO PUBLIC;
	-- GRANT EXECUTE ON FUNCTION exodus.extract_time(text, integer, integer, integer)     TO PUBLIC;
	-- GRANT EXECUTE ON FUNCTION exodus.extract_datetime(text, integer, integer, integer) TO PUBLIC;
	-- GRANT EXECUTE ON FUNCTION exodus.extract_number(text, integer, integer, integer)   TO PUBLIC;
	-- GRANT EXECUTE ON FUNCTION exodus.count(text, text)                                 TO PUBLIC;
	-- GRANT EXECUTE ON FUNCTION exodus.toBool(numeric)                                   TO PUBLIC;
	-- GRANT EXECUTE ON FUNCTION exodus.toBool(text)                                      TO PUBLIC;
	ALTER FUNCTION exodus.extract_text(text, integer, integer, integer)     OWNER TO exodus;
	ALTER FUNCTION exodus.extract_date(text, integer, integer, integer)     OWNER TO exodus;
	ALTER FUNCTION exodus.extract_time(text, integer, integer, integer)     OWNER TO exodus;
	ALTER FUNCTION exodus.extract_datetime(text, integer, integer, integer) OWNER TO exodus;
	ALTER FUNCTION exodus.extract_number(text, integer, integer, integer)   OWNER TO exodus;
	ALTER FUNCTION exodus.count(text, text)                                 OWNER TO exodus;
	ALTER FUNCTION exodus.toBool(numeric)                                   OWNER TO exodus;
	ALTER FUNCTION exodus.toBool(text)                                      OWNER TO exodus;

-- A function to assist in tests
--------------------------------

	CREATE OR REPLACE FUNCTION exodus.assert(result bool, expression text)
	RETURNS VOID AS $$
	BEGIN
		ASSERT result, 'Self test failed. "' || expression || '" should be true.';
		RAISE INFO 'Test: % ', expression;
	END;
	$$ LANGUAGE plpgsql;

	CREATE OR REPLACE FUNCTION exodus.info(expression text)
	RETURNS VOID AS $$
	BEGIN
		RAISE INFO 'Test: % ', expression;
	END;
	$$ LANGUAGE plpgsql;

-- Self test the various functions
----------------------------------

-- text

	select exodus.assert((exodus.extract_text('aaa',1,1,1) = 'aaa'), 'exodus.extract_text(''aaa'',1,1,1) = ''aaa''');
	select exodus.assert((exodus.extract_text(E'a1\x1Eb1\x1Db2a\x1Cb2b\x1Ecc',2,2,2) = 'b2b'),'exodus.extract_text(E''a1\x1Eb1\x1Db2a\x1Cb2b\x1Ecc'',2,2,2) = ''b2b''');
	select exodus.assert((exodus.extract_text('aaa',1,0,0) = 'aaa'), 'exodus.extract_text(''aaa'',1,0,0) = ''aaa''');
	select exodus.assert((exodus.extract_text('aaa',1,1,0) = 'aaa'), 'exodus.extract_text(''aaa'',1,1,0) = ''aaa''');
	select exodus.assert((exodus.extract_text('aaa',0,0,0) = 'aaa'), 'exodus.extract_text(''aaa'',0,0,0) = ''aaa''');

-- date

	select exodus.assert((exodus.extract_date('',1,1,1)         is null),     'exodus.extract_date('''',1,1,1) is null');
	select exodus.assert((exodus.extract_date('0',1,1,1)     = '1967-12-31'), 'exodus.extract_date(''0'',1,1,1) = ''1967-12-31''');
	select exodus.assert((exodus.extract_date('20000',1,1,1) = '2022-10-03'), 'exodus.extract_date(''20000'',1,1,1) = ''2022-10-03''');
	select exodus.assert((exodus.extract_date('20000.9',1,1,1) = '2022-10-03'), 'exodus.extract_date(''20000.9'',1,1,1) = ''2022-10-03''');

-- time

	select exodus.assert((exodus.extract_time('',1,1,1)          is null),   'exodus.extract_time('''',1,1,1) is null');
	select exodus.assert((exodus.extract_time('0',1,1,1)      = '00:00:00'), 'exodus.extract_time(''0'',1,1,1) = ''00:00:00''');
	select exodus.assert((exodus.extract_time('10000',1,1,1)  = '02:46:40'), 'exodus.extract_time(''10000'',1,1,1) = ''02:46:40''');
	select exodus.assert((exodus.extract_time('86399',1,1,1)  = '23:59:59'), 'exodus.extract_time(''86399'',1,1,1) = ''23:59:59''');
	select exodus.assert((exodus.extract_time('86400',1,1,1)  = '24:00:00'), 'exodus.extract_time(''86400'',1,1,1) = ''24:00"00''');
	select exodus.assert((exodus.extract_time('100000',1,1,1) = '27:46:40'), 'exodus.extract_time(''100000'',1,1,1) = ''27:46:40''');
	select exodus.assert((exodus.extract_time('3600.5',1,1,1) = '01:00:00'), 'exodus.extract_time(''3600.5'',1,1,1) = ''01:00:00''');

-- datetime

	select exodus.assert((exodus.extract_datetime('',1,1,1)               is null), 'exodus.extract_datetime('''',1,1,1) is null');
	select exodus.assert((exodus.extract_datetime('0',1,1,1)           = '1967-12-31 00:00:00'), 'exodus.extract_datetime(''0'',1,1,1) = ''1967-12-31 00:00:00''');
	select exodus.assert((exodus.extract_datetime('20000.50000',1,1,1) = '2022-10-03 13:53:20'), 'exodus.extract_datetime(''20000.50000'',1,1,1) = ''2022-10-03 13:53:20''');
	select exodus.assert((exodus.extract_datetime('20000'      ,1,1,1) = '2022-10-03 00:00:00'), 'exodus.extract_datetime(''20000''      ,1,1,1) = ''2022-10-03 00:00:00''');
	select exodus.assert((exodus.extract_datetime(    '0.50000',1,1,1) = '1967-12-31 13:53:20'), 'exodus.extract_datetime(    ''0.50000'',1,1,1) = ''1967-12-31 13:53:20''');

-- number

	select exodus.assert((exodus.extract_number('',1,1,1)           = 0),          'exodus.extract_number('''',1,1,1) = 0');
	select exodus.assert((exodus.extract_number('0',1,1,1)          = 0),          'exodus.extract_number(''0'',1,1,1) = 0');
	select exodus.assert((exodus.extract_number('100',1,1,1)        = 100),        'exodus.extract_number(''100'',1,1,1) = 100');
	select exodus.assert((exodus.extract_number('12345.6789',1,1,1) = 12345.6789), 'exodus.extract_number(''12345.6789'',1,1,1) = 12345.6789');

	select exodus.assert((exodus.extract_number('-0',1,1,1)          = 0),           'exodus.extract_number(''-0'',1,1,1) = 0');
	select exodus.assert((exodus.extract_number('-100',1,1,1)        = -100),        'exodus.extract_number(''-100'',1,1,1) = -100');
	select exodus.assert((exodus.extract_number('-12345.6789',1,1,1) = -12345.6789), 'exodus.extract_number(''-12345.6789'',1,1,1) = -12345.6789');

-- count

	select exodus.assert((exodus.count('''','a')  = 0), 'exodus.count('''',''a'') = 0');
	select exodus.assert((exodus.count('0','a')   = 0), 'exodus.count(''0'',''a'') = 0');
	select exodus.assert((exodus.count('aaa','a') = 3), 'exodus.count(''aaa'',''a'') = 3');
	select exodus.assert((exodus.count('aaaa','aa') = 2), 'exodus.count(''aaaa'',''aa'') = 2');

-- toBool numeric

	select exodus.assert((exodus.toBool(0)     = false), 'exodus.toBool(0)   = false');
	select exodus.assert((exodus.toBool(0.0)   = false), 'exodus.toBool(0.0) = false');
	select exodus.assert((exodus.toBool(1)     = true),  'exodus.toBool(1)   = true');
	select exodus.assert((exodus.toBool(1.0)   = true),  'exodus.toBool(1.0) = true');
-- PickOS small numbers should be false
--	select exodus.assert((exodus.toBool(0.00001) = false),  'exodus.toBool(0.00001) = false');

-- toBool text

	select exodus.assert((exodus.toBool('')    = false),  'exodus.toBool('''')    = false');
	select exodus.assert((exodus.toBool('0')   = false),  'exodus.toBool(''0'')   = false');
	select exodus.assert((exodus.toBool('00')  = false),  'exodus.toBool(''00'')  = false');
	select exodus.assert((exodus.toBool('.0')  = false),  'exodus.toBool(''.0'')  = false');
	select exodus.assert((exodus.toBool('0.')  = false),  'exodus.toBool(''0.'')  = false');
	select exodus.assert((exodus.toBool('0.0') = false),  'exodus.toBool(''0.0'') = false');
	select exodus.assert((exodus.toBool('-0')  = false),  'exodus.toBool(''-0'')  = false');

	select exodus.assert((exodus.toBool(' ')   = true),  'exodus.toBool('' '')    = true');
	select exodus.assert((exodus.toBool('.')   = true),  'exodus.toBool(''.'')    = true');
	select exodus.assert((exodus.toBool('-')   = true),  'exodus.toBool(''-'')    = true');
	select exodus.assert((exodus.toBool('1')   = true),  'exodus.toBool(''01'')   = true');
	select exodus.assert((exodus.toBool('11')  = true),  'exodus.toBool(''11'')   = true');
	select exodus.assert((exodus.toBool('.1')  = true),  'exodus.toBool(''.1'')   = true');
	select exodus.assert((exodus.toBool('1.')  = true),  'exodus.toBool(''1.'')   = true');
	select exodus.assert((exodus.toBool('1.1') = true),  'exodus.toBool(''1.1'')  = true');
	select exodus.assert((exodus.toBool('-1')  = true),  'exodus.toBool(''-1'') = true');

	select exodus.assert((exodus.toBool('a')   = true),  'exodus.toBool(''a'')    = true');
	select exodus.assert((exodus.toBool('Z')   = true),  'exodus.toBool(''Z'')    = true');

	select exodus.assert((exodus.toBool(' 0')   = true),  'exodus.toBool('' 0'')    = true');
	select exodus.assert((exodus.toBool('0 ')   = true),  'exodus.toBool(''0 '')    = true');
	select exodus.assert((exodus.toBool(E'\t0') = true),  'exodus.toBool(''\t0'') = true');
	select exodus.assert((exodus.toBool(E'0\t') = true),  'exodus.toBool(''0\t'')  = true');
	select exodus.assert((exodus.toBool(E'\n0') = true),  'exodus.toBool(''\n0'') = true');
	select exodus.assert((exodus.toBool(E'0\n') = true),  'exodus.toBool(''0\n'')  = true');
	select exodus.assert((exodus.toBool(E'\r0') = true),  'exodus.toBool(''\r0'') = true');
	select exodus.assert((exodus.toBool(E'0\r') = true),  'exodus.toBool(''0\r'')  = true');

-- bad dates, times and numbers

	select exodus.assert(true, '-- bad dates, times and numbers --');
--	select exodus.assert((exodus.extract_date('x',1,1,1)     = '1967-12-31'), 'exodus.extract_date(''x'',1,1,1) = ''1967-12-31''');
	select exodus.assert((exodus.extract_date('x',1,1,1)     is null       ), 'exodus.extract_date(''x'',1,1,1) is null');
	select exodus.assert((exodus.extract_time('x',1,1,1)     = '00:00:00'),   'exodus.extract_time(''x'',1,1,1) = ''00:00:00''');
	select exodus.assert((exodus.extract_datetime('x',1,1,1) = '1967-12-31 00:00:00'), 'exodus.extract_date(''x'',1,1,1) = ''1967-12-31 00:00:00''');
	select exodus.assert((exodus.extract_number('x',1,1,1)   = 0),            'exodus.extract_number(''x'',1,1,1) = 0');

-- TODO Leading spaces are tolerated but should not be

	select exodus.assert(true, '-- TODO non-numeric dates, times and numbers should raise a non-numeric err --');
	--select exodus.assert((exodus.extract_date(' 1',1,1,1)   = '1967-12-31'), 'exodus.extract_date('' 1'',1,1,1) = ''1967-12-31''');
	--select exodus.assert((exodus.extract_time(' 1',1,1,1)   = '00:00:00'),   'exodus.extract_time('' 1'',1,1,1) = ''00:00:00''');
	--select exodus.assert((exodus.extract_datetime(' 1',1,1,1) = '1967-12-31 00:00:00'), 'exodus.extract_date('' 1'',1,1,1) = ''1967-12-31 00:00:00''');
	--select exodus.assert((exodus.extract_number(' 1',1,1,1) = 0),            'exodus.extract_number('' 1'',1,1,1) = 0');

-- Clean up
-----------

	DROP FUNCTION exodus.assert(result bool, expression text);
DO $$
BEGIN
   RAISE INFO '
Created postgresql extension pgexodus
─────────────────────────────────────';
END
$$;
