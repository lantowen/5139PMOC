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
//Datum		email_recv(PG_FUNCTION_ARGS);
//Datum		email_send(PG_FUNCTION_ARGS);
static int Word_Validate(char* word) {
	int i;
	i = strlen(word);
	if((word[0] >= 65 &&  word[0] <= 90)||(word[0] >= 97 &&  word[0] <= 122))
			if((word[i-1] >= 48 &&  word[i-1] <= 57)||(word[i-1] >= 65 &&  word[i-1] <= 90)
				||(word[i-1] >= 97 &&  word[i-1] <= 122))
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
	snprintf(result, 300, "(%s,%s)", email->x, email->y);
	PG_RETURN_CSTRING(result);
}

/*
PG_FUNCTION_INFO_V1(email_recv);

Datum
email_recv(PG_FUNCTION_ARGS)
{
	StringInfo	buf = (StringInfo) PG_GETARG_POINTER(0);
	Email    *result;

	result = (Email *) palloc(sizeof(Email));
	result->x = pq_getmsgfloat8(buf);
	result->y = pq_getmsgfloat8(buf);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(email_send);

Datum
email_send(PG_FUNCTION_ARGS)
{
	Email    *email = (Email *) PG_GETARG_POINTER(0);
	StringInfoData buf;

	pq_begintypsend(&buf);
	pq_sendfloat8(&buf, email->x);
	pq_sendfloat8(&buf, email->y);
	PG_RETURN_BYTEA_P(pq_endtypsend(&buf));
}
*/
