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
