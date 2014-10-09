// gendata.c
// ... make random data for known schemas
// ... usage: ./gendata #tuples
// ... written by John Shepherd Sept 2012


#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <string.h>

// Schema: R(x:integer,y:text) S(a:integer,b:text,c:references(R(x)))

#define insertR "insert into R(x,y) values (%d,'%s');\n"
#define insertS "insert into S(a,b,c) values (%d,'%s',%d);\n"

#define STRLEN  100

int uniform_random(int, int);
int normal_random(int, int);
char *random_string(int, char *);

int main(int argc, char **argv)
{
	int i, j, m, n, Ntuples;
	char str[STRLEN];
	int x, a; char *y, *b;
    int seed = 0;

	if (argc < 2 || sscanf(argv[1],"%d",&Ntuples) != 1) {
		fprintf(stderr, "Usage: %s #tuples [seed]\n", argv[0]);
		exit(1);
	}
	if (argc > 2 && sscanf(argv[2],"%d",&seed) != 1) {
		fprintf(stderr, "Usage: %s #tuples [seed]\n", argv[0]);
		exit(1);
	}
	srandom(seed);  // use e.g. time() if you want it "more random"
	x = 99999;
	for (i = 0; i < Ntuples; i++) {
		x++;
		n = uniform_random(10,STRLEN-1);
		y = random_string(n, str);
		printf(insertR,x,y);
		m = uniform_random(0,5);
		for (j = 0; j < m; j++) {
			a = normal_random(1,100000);
			n = uniform_random(10,STRLEN-1);
			b = random_string(n, str);
			printf(insertS, a, b, x);
		}
	}
	return 0;
}

// random_string(len, buf)
// ... generate a random string of length len in buffer buf
// ... assumes that buf is big enough to hold strings of length len

char *random_string(int len, char *buf)
{
	int i, n;
	char*cs= "!@#$%^&*?abcdefghijklmnopqrstuvwxyz0123456789";

	n = strlen(cs);
	for (i = 0; i < len; i++) {
		buf[i] = cs[uniform_random(0,n-1)];
	}
	buf[i] = '\0';
	return buf;
}

// uniform_random(lo,hi)
// ... generate a uniformly-distributed random number in range lo..hi

int uniform_random(int lo, int hi)
{
	assert(hi > lo);
	long x = random();
	return lo + (x % (hi-lo+1));
}

// normal_random(lo,hi)
// ... generate a normally-distributed random number in range lo..hi

int normal_random(int lo, int hi)
{
	int r;  float u,v,x;
	assert(hi > lo);
	do {
		u = (random() & 0x7FFFFFFF) / (float)0x7FFFFFFF;
		v = (random() & 0x7FFFFFFF) / (float)0x7FFFFFFF;
		x = sqrt(-2*log(u))*cos(2*M_PI*v);
		x = (x+1)/2;
		// printf("(%0.3f,%0.3f,%0.3f) ",u,v,x);
		r = lo + (int)(roundf(x*(hi-lo+1)));
	} while (r < lo || r > hi);
	return r;
}
