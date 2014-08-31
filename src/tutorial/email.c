/*
 * src/tutorial/Email.c
 *
 ******************************************************************************
  This file contains routines that can be bound to a Postgres backend and
  called by the backend in the process of processing queries.  The calling
  format for these routines is dictated by Postgres architecture.
******************************************************************************/

#include "postgres.h"

#include "fmgr.h"
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include "libpq/pqformat.h"		/* needed for send/recv functions */

#define DOMAIN_LENGTH 128
#define LOCAL_LENGTH 128
#define DOMAIN 1
#define LOCAL 0
PG_MODULE_MAGIC;

typedef struct Email
{
	char x[DOMAIN_LENGTH];
	char y[LOCAL_LENGTH];
}	Email;

/*
 * Since we use V1 function calling convention, all these functions have
 * the same signature as far as C is concerned.  We provide these prototypes
 * just to forestall warnings when compiled with gcc -Wmissing-prototypes.
 */
Datum		email_in(PG_FUNCTION_ARGS);
Datum		email_out(PG_FUNCTION_ARGS);
Datum		email_recv(PG_FUNCTION_ARGS);
Datum		email_send(PG_FUNCTION_ARGS);
Datum		email_eq(PG_FUNCTION_ARGS);
Datum		email_lt(PG_FUNCTION_ARGS);
Datum		email_le(PG_FUNCTION_ARGS);
Datum		email_neq(PG_FUNCTION_ARGS);
Datum		email_ge(PG_FUNCTION_ARGS);
Datum		email_gt(PG_FUNCTION_ARGS);
Datum		email_cmp(PG_FUNCTION_ARGS);
Datum		email_domain_eq(PG_FUNCTION_ARGS);
Datum		email_domain_neq(PG_FUNCTION_ARGS);
static int Word_Validate(char* word) {
	int i;
	i = strlen(word);
	if((word[0] >= 'A' &&  word[0] <= 'Z')||(word[0] >= 'a' &&  word[0] <= 'z'))
			if((word[i-1] >= '0' &&  word[i-1] <= '9')||(word[i-1] >= 'A' &&  word[i-1] <= 'Z')
				||(word[i-1] >= 'a' &&  word[i-1] <= 'z'))
				for(int a = 1; a < i-1; a++) {
				if((word[a] >= 48 &&  word[a] <= 57)||(word[a] >= 65 &&  word[a] <= 90)
				||(word[a] >= 97 &&  word[a] <= 122)||word[a] == 45)
					;
				else
					return 0;
				}
			else return 0;
	else return 0;
	return 1;
}
static int Email_Validate(char* part, int mode) {
	char str[128];
	int counter = 0;
	int flag = 0;
	strncpy(str , part, 127);
	int a = strlen(str);
	if(str[0] == 46 || str[a-1] == 46)
		return 0;
	for(int i = 0; i < a; i++) {
		if(str[i] == 46)
			if(flag == 1)
				return 0;
			else
				flag = 1;
		else 
			flag = 0;		
	}
	char* pch;
	pch = strtok (str,".");
	if(Word_Validate(pch) == 0)
		return 0;
  	while (true)
  	{
    	pch = strtok (NULL, ".");
    	if(pch == NULL)
    		break;
    	if(Word_Validate(pch) == 0)
			return 0;
    	counter++;
  	}
  	if(counter == 0 && mode == DOMAIN)
  		return 0;
  	
  	return 1;
	
}
static int
email_cmp_internal(Email * a, Email * b)
{
	int i = 0;
	//ereport(NOTICE, (errmsg("called 1: a=%s", a->x)));
	while(a->x[i] != '\0' && b->x[i] != '\0') {
		//ereport(NOTICE, (errmsg("called ")));
		if(a->x[i] > b->x[i])
			return 1;
		else if(a->x[i] < b->x[i])
			return -1;
		i++;
	}
	//ereport(NOTICE, (errmsg("called 1")));
	if(a->x[i] == '\0' && b->x[i] == '\0') 
		return strncmp(a->y, b->y, DOMAIN_LENGTH);
	else if(a->x[i] == '\0') 
		return '@' - b->x[i];
	else
		return a->x[i] - '@';
}

/*****************************************************************************
 * Input/Output functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(email_in);

Datum
email_in(PG_FUNCTION_ARGS)
{
	char	   *str = PG_GETARG_CSTRING(0);
	char*       x, 
			*y;
	char*       pch;
	x = (char *)palloc(sizeof(char)*128);
	y = (char *)palloc(sizeof(char)*128);
	pch = strchr(str,'@');
	if(pch == NULL)
		ereport(ERROR,
				(errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
				 errmsg("not valid email address(no @)\"%s\"",
						str)));
	pch = strchr(pch+1,'@');
	if(pch != NULL)
		ereport(ERROR,
				(errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
				 errmsg("not valid email address(more @s)\"%s\"",
						str)));
	Email    *result;
	result = (Email *) palloc(sizeof(Email));
	pch = strtok(str,"@");
	if(strlen(pch) > 127)
	ereport(ERROR,
				(errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
				 errmsg("Local too long: \"%s\"",
						pch)));
	strncpy(x ,pch, 127);
	pch = strtok(NULL,"@");
	if(strlen(pch) > 127)
	ereport(ERROR,
				(errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
				 errmsg("Domain too long: \"%s\"",
						pch)));
	strncpy(y ,pch, 127);
	int test;
	test = Email_Validate(x, LOCAL);
	if( test == 0)
		ereport(ERROR,
				(errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
				 errmsg("Invalid Local: \"%s\"",
						x)));
	test = Email_Validate(y, DOMAIN);
	if(test == 0)
		ereport(ERROR,
				(errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
				 errmsg("Invalid DOMAIN: \"%s\"",
						y)));
	
	/*
	if (sscanf(str, "%s", x) != 1)
		ereport(ERROR,
				(errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
				 errmsg("invalid input syntax for Email: \"%s\"",
						str)));

	
	strncpy(result->x ,x, 127);
	result->x[127] = '\0';
	//result->x[1] = '\0';
	result->y = 100;
	*/
	int i = 0;
	char c;
	while(x[i]) {
		c = x[i];
		x[i] = tolower(c);
		i++;
	}
	i = 0;
	while(y[i]) {
		c = y[i];
		y[i] = tolower(c);
		i++;
	}
	strncpy(result->x ,x, 127);
	strncpy(result->y ,y, 127);
	pfree(x);
	pfree(y);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(email_out);

Datum
email_out(PG_FUNCTION_ARGS)
{
	Email    *email = (Email *) PG_GETARG_POINTER(0);
	char	   *result;

	result = (char *) palloc(300);
	snprintf(result, 300, "%s@%s", email->x, email->y);
	PG_RETURN_CSTRING(result);
}
PG_FUNCTION_INFO_V1(email_recv);

Datum
email_recv(PG_FUNCTION_ARGS)
{
	StringInfo	buf = (StringInfo) PG_GETARG_POINTER(0);
	Email    *result;
	char* temp_str1 =  NULL,
		*temp_str2 = NULL;
	//ereport(NOTICE, (errmsg("email recv called")));
	result = (Email *) palloc(sizeof(Email));
	temp_str1 = (char *)pq_getmsgstring(buf);

	temp_str2 = (char *)pq_getmsgstring(buf);
	strncpy(result->x , temp_str1,127);

	strncpy(result->y , temp_str2,127);

	
	//pfree(temp_str1);
	//pfree(temp_str2);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(email_send);

Datum
email_send(PG_FUNCTION_ARGS)
{
	Email    *email = (Email *) PG_GETARG_POINTER(0);
	StringInfoData buf;
	//ereport(NOTICE, (errmsg("email send called")));

	pq_begintypsend(&buf);
	pq_sendstring(&buf, email->x);
	//pq_sendtext(&buf, "@",1);
	pq_sendstring(&buf, email->y);
	PG_RETURN_BYTEA_P(pq_endtypsend(&buf));
}

PG_FUNCTION_INFO_V1(email_eq);

Datum
email_eq(PG_FUNCTION_ARGS)
{
	Email    *a = (Email *) PG_GETARG_POINTER(0);
	Email    *b = (Email *) PG_GETARG_POINTER(1);

	PG_RETURN_BOOL(email_cmp_internal(a, b) == 0);
}
PG_FUNCTION_INFO_V1(email_lt);

Datum
email_lt(PG_FUNCTION_ARGS)
{
	Email    *a = (Email *) PG_GETARG_POINTER(0);
	Email    *b = (Email *) PG_GETARG_POINTER(1);

	PG_RETURN_BOOL(email_cmp_internal(a, b) < 0);
}

PG_FUNCTION_INFO_V1(email_le);

Datum
email_le(PG_FUNCTION_ARGS)
{
	Email    *a = (Email *) PG_GETARG_POINTER(0);
	Email    *b = (Email *) PG_GETARG_POINTER(1);

	PG_RETURN_BOOL(email_cmp_internal(a, b) <= 0);
}

PG_FUNCTION_INFO_V1(email_neq);

Datum
email_neq(PG_FUNCTION_ARGS)
{
	Email    *a = (Email *) PG_GETARG_POINTER(0);
	Email    *b = (Email *) PG_GETARG_POINTER(1);

	PG_RETURN_BOOL(email_cmp_internal(a, b) != 0);
}

PG_FUNCTION_INFO_V1(email_ge);

Datum
email_ge(PG_FUNCTION_ARGS)
{
	Email    *a = (Email *) PG_GETARG_POINTER(0);
	Email    *b = (Email *) PG_GETARG_POINTER(1);

	PG_RETURN_BOOL(email_cmp_internal(a, b) >= 0);
}

PG_FUNCTION_INFO_V1(email_gt);

Datum
email_gt(PG_FUNCTION_ARGS)
{
	Email    *a = (Email *) PG_GETARG_POINTER(0);
	Email    *b = (Email *) PG_GETARG_POINTER(1);

	PG_RETURN_BOOL(email_cmp_internal(a, b) > 0);
}

PG_FUNCTION_INFO_V1(email_cmp);

Datum
email_cmp(PG_FUNCTION_ARGS)
{
	Email    *a = (Email *) PG_GETARG_POINTER(0);
	Email    *b = (Email *) PG_GETARG_POINTER(1);

	PG_RETURN_INT32(email_cmp_internal(a, b));
}


PG_FUNCTION_INFO_V1(email_domain_eq);

Datum
email_domain_eq(PG_FUNCTION_ARGS)
{
	Email    *a = (Email *) PG_GETARG_POINTER(0);
	Email    *b = (Email *) PG_GETARG_POINTER(1);

	PG_RETURN_INT32(strncmp(a->y, b->y, DOMAIN_LENGTH) == 0);
}

PG_FUNCTION_INFO_V1(email_domain_neq);

Datum
email_domain_neq(PG_FUNCTION_ARGS)
{
	Email    *a = (Email *) PG_GETARG_POINTER(0);
	Email    *b = (Email *) PG_GETARG_POINTER(1);

	PG_RETURN_INT32(strncmp(a->y, b->y, DOMAIN_LENGTH) != 0);
}

