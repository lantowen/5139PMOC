create table R (
	x  integer unique,
	y  text
);

create table S (
	a  integer,
	b  text,
	c  integer references R(x)
);
