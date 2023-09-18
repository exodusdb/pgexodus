set datestyle = "ISO";
set intervalstyle = "postgres";
select
 exodus.extract_text(E'a1 \x1E b1 \x1D b2a \x1C b2b \x1E cc',2,2,2),
 exodus.extract_date('20000',0,0,0),
 exodus.extract_time('86399',1,0,0),
 exodus.extract_datetime('20000.86399',1,1,0),
 exodus.extract_number('123.456', 1, 0, 0),
 exodus.count('ABBA','A');
