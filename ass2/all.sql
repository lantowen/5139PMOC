SELECT avg(s.a), count(*) FROM s;
SELECT avg(s.a), count(*) FROM s where s.b LIKE 'a%';
SELECT count(*) from r join s on r.x = s.c;
