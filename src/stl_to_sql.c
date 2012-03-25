/*
 *   Implement the callback functions that will allow 
 *   SQLite to manage SQL queries towards the registered 
 *   virtual tables.
 *
 *   Copyright 2012 Marios Fragkoulis
 *
 *   Licensed under the Apache License, Version 2.0
 *   (the "License");you may not use this file except in
 *   compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in
 *   writing, software distributed under the License is
 *   distributed on an "AS IS" BASIS.
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 *   express or implied.
 *   See the License for the specific language governing
 *   permissions and limitations under the License.
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include "stl_to_sql.h"
#include "stl_search.h"

// Constructs the SQL CREATE query.
void create(sqlite3 *db, 
	    int argc, 
	    const char * const * as, 
	    char *q) { 
  int i;
  q[0] = '\0';
  strcat(q, "CREATE TABLE ");
  strcat(q, as[2]);
  strcat(q, "(");
  for (i = 3; i < argc; i++) {
    strcat(q, as[i]);
    if (i+1 < argc) strcat(q, ",");
    else strcat(q, ");");
  }
  q[strlen(q)] = '\0';
#ifdef DEBUG
  printf("Query is: %s with length %i \n", q, 
	 (int)strlen(q));
#endif
}

/* Creates/connects a virtual table to the provided 
 * database.
 * isCreate for activating e.g. extra storage utilised. 
 * Not used so far.
 */
int init_vtable(int iscreate, 
		sqlite3 *db, 
		void *paux, 
		int argc, 
		const char * const * argv, 
		sqlite3_vtab **ppVtab,
		char **pzErr) {
  stlTable *stl;
  int nDb, nName, nByte, nCol, nString, i;
  char *temp;
  nDb = (int)strlen(argv[1]) + 1;
  nName = (int)strlen(argv[2]) + 1;
  nString=0;
  // explore fts3 way
  for (i = 3; i < argc; i++){
    nString += (int)strlen(argv[i]) + 1;
  }
  nCol = argc - 3;
  assert(nCol > 0);
  nByte = sizeof(stlTable) + nCol * sizeof(char *) + 
    nDb + nName + nString;
  stl = (stlTable *)sqlite3_malloc(nByte);
  if (stl == 0) {
    return SQLITE_NOMEM;
  }
  memset(stl, 0, nByte);
  stl->zErr = NULL;
  stl->db = db;
  stl->nColumn = nCol;
  stl->azColumn = (char **)&stl[1];
  temp = (char *)&stl->azColumn[nCol];

  stl->zName = temp;
  memcpy(temp, argv[2], nName);
  temp += nName;
  stl->zDb = temp;
  memcpy(temp, argv[1], nDb);
  temp += nDb;

  int n;
  for (i = 3; i < argc; i++){
    n = (int)strlen(argv[i]) + 1;
    memcpy(temp, argv[i], n);
    stl->azColumn[i-3] = temp;
    temp += n;
    assert(temp <= &((char *)stl)[nByte]);
  }

  char query[arrange_size(argc, argv)];
  create(db, argc, argv, query);
#ifdef DEBUG
  printf("Query is: %s \n", query);
#endif
  if (!(*pzErr)) {
    int output = sqlite3_declare_vtab(db, query);
    if (output == 1) {
      *pzErr = sqlite3_mprintf("Error while declaring the virtual table.\n");
      printf("%s \n", *pzErr);
      return SQLITE_ERROR;
    } else if (output == 0) {
      *ppVtab = &stl->vtab;
      register_vt(stl);
#ifdef DEBUG
      printf("Virtual table declared successfully.\n");
#endif
      return SQLITE_OK;
    }
  }else{
    *pzErr = sqlite3_mprintf("Unknown error");
    printf("%s \n", *pzErr);
    return SQLITE_ERROR;
  } 
}

//xConnect
int connect_vtable(sqlite3 *db, 
		   void *paux, 
		   int argc,
		   const char * const * argv, 
		   sqlite3_vtab **ppVtab,
		   char **pzErr) {
#ifdef DEBUG
  printf("Connecting vtable %s \n\n", argv[2]);
#endif
  return init_vtable(0, db, paux, argc, argv, 
		     ppVtab, pzErr);
}

// xCreate
int create_vtable(sqlite3 *db, 
		  void *paux, 
		  int argc,
		  const char * const * argv, 
		  sqlite3_vtab **ppVtab,
		  char **pzErr) {
#ifdef DEBUG
  printf("Creating vtable %s \n\n", argv[2]);
#endif
  return init_vtable(1, db, paux, argc, argv, ppVtab, pzErr);
}

// SQL update. Not provided.
int update_vtable(sqlite3_vtab *pVtab, 
		  int argc, 
		  sqlite3_value **argv, 
		  sqlite_int64 *pRowid) {
  return SQLITE_OK;
}

// xDestroy
int destroy_vtable(sqlite3_vtab *ppVtab) {
  stlTable *st = (stlTable *)ppVtab;
#ifdef DEBUG
  printf("Destroying vtable %s \n\n", st->zName);
#endif
  int result;
  // Need to destroy additional structures. So far none.
  result = disconnect_vtable(ppVtab);
  return result;
}

// xDisconnect. Called when closing a database connection.
int disconnect_vtable(sqlite3_vtab *ppVtab) {
  stlTable *s=(stlTable *)ppVtab;
#ifdef DEBUG
  printf("Disconnecting vtable %s \n\n", s->zName);
#endif
  sqlite3_free(s);
  return SQLITE_OK;
}

/* Helper function for structuring an SQL WHERE 
 * constraint.
 */
void eval_constraint(int sqlite3_op, 
		     char iCol, 
		     int *j, 
		     char *nidxStr, 
		     int nidxLen) {
  char op;
  switch (sqlite3_op) {
  case SQLITE_INDEX_CONSTRAINT_LT:
    op='A'; 
    break;
  case SQLITE_INDEX_CONSTRAINT_LE:  
    op='B'; 
    break;
  case SQLITE_INDEX_CONSTRAINT_EQ:  
    op='C'; 
    break;
  case SQLITE_INDEX_CONSTRAINT_GE:  
    op='D'; 
    break;
  case SQLITE_INDEX_CONSTRAINT_GT:  
    op='E'; 
    break;
    //    case SQLITE_INDEX_CONSTRAINT_MATCH: nidxStr[i]="F"; break;
  }
	
  assert(*j < nidxLen - 2);
  nidxStr[(*j)++] = op;
  nidxStr[(*j)++] = iCol;
}

/* xBestindex. Defines the query plan for an SQL query. 
 * Might be called multiple times with alternate plans.
 */
int best_index_vtable(sqlite3_vtab *pVtab, 
		     sqlite3_index_info *pInfo) {
  stlTable *st=(stlTable *)pVtab;
  /* No constraint no setting up. */
  if (pInfo->nConstraint > 0) {
    char iCol;
    int nCol;
    int nidxLen = pInfo->nConstraint*2 + 1;
    char nidxStr[nidxLen];
    memset(nidxStr, 0, sizeof(nidxStr));

    assert(pInfo->idxStr == 0);
    int i, j=0;
    if (!st->embedded) {
      for (i = 0; i < pInfo->nConstraint; i++){
	struct sqlite3_index_constraint *pCons = 
	  &pInfo->aConstraint[i];
	if (pCons->usable == 0) 
	  continue;
	iCol = pCons->iColumn - 1 + 'a';
	eval_constraint(pCons->op, iCol, &j, 
			nidxStr, nidxLen);
	pInfo->aConstraintUsage[i].argvIndex = i+1;
	pInfo->aConstraintUsage[i].omit = 1;
      }
    } else {
      if (st->zErr != NULL) {
	sqlite3_free(st->zErr);
	st->zErr = NULL;
#ifdef DEBUG
	printf("zErr freed for %s\n", st->zName);
#endif
      }
      for (i = 0; i < pInfo->nConstraint; i++) {
	struct sqlite3_index_constraint *pCons = 
	  &pInfo->aConstraint[i];
	if( pCons->usable==0 ) 
	  continue;
	iCol = pCons->iColumn - 1 + 'a';
	nCol = pCons->iColumn;
	if (!equals_base(st->azColumn[nCol])) 
	  continue;
	eval_constraint(pCons->op, iCol, &j, 
			nidxStr, nidxLen);
	pInfo->aConstraintUsage[i].argvIndex = 1;
	pInfo->aConstraintUsage[i].omit = 1;
      }
      if (j == 0) {
	st->zErr = sqlite3_mprintf("Query VT with no usable BASE constraint.Abort.\n");
#ifdef DEBUG
	printf("j=0: NO BASE for %s\n", st->zName);
#endif
	return SQLITE_OK;
      }
      int counter = 2;
      for (i = 0; i < pInfo->nConstraint; i++) {
	struct sqlite3_index_constraint *pCons = 
	  &pInfo->aConstraint[i];
	if (pCons->usable == 0) 
	  continue;
	iCol = pCons->iColumn - 1 + 'a';
	nCol = pCons->iColumn;
	if (equals_base(st->azColumn[nCol])) 
	  continue;
	eval_constraint(pCons->op, iCol, &j, 
			nidxStr, nidxLen);
	pInfo->aConstraintUsage[i].argvIndex = counter++;
	pInfo->aConstraintUsage[i].omit = 1;
      }
    }
    pInfo->needToFreeIdxStr = 1;
    if ((j>0) && 0 == (pInfo->idxStr = 
		       sqlite3_mprintf("%s", nidxStr)))
      return SQLITE_NOMEM;
  }
  return SQLITE_OK;
}

/* xFilter. Filters an SQL query. Calls the search 
 * family of callbacks at stl_search.cpp.
 */
int filter_vtable(sqlite3_vtab_cursor *cur, 
		  int idxNum, 
		  const char *idxStr,
		  int argc, 
		  sqlite3_value **argv) {
  stlTableCursor *stc=(stlTableCursor *)cur;
  int i, j = 0, re = 0;
  char *constr = (char *)sqlite3_malloc(sizeof(char) * 3);
  memset(constr, 0, sizeof(constr));
  /* Initialize size of resultset data structure. */
  stc->size = 0;
  stc->current = -1;      /* Initial cursor position. */

  /* In case of a join, xfilter will be called many times, 
   *  x times for x eligible rows of the paired table. 
   * In this case isEof will be set to terminate at row 
   * level and has to be reset to allow matching all 
   * eligible rows.
   */
  stc->isEof = 0;         
  /* First_constr is used to signal that the current 
   * constr encountered is the first (value 1) 
   * or not (value 0).
   */
  stc->first_constr = 1;  
  if (argc == 0) {        /* Empty where clause. */
    if ((re = search(cur, NULL, NULL)) != 0 ) {
      sqlite3_free(constr);
      return re;
    }
  } else {
    for (i=0; i < argc; i++) {
      constr[0] = idxStr[j++];
      constr[1] = idxStr[j++];
      constr[2] = '\0';
      if ((re = search(cur, constr, argv[i])) != 0) {
	sqlite3_free(constr);
	return re;
      }
    }
  }
  sqlite3_free(constr);
  return next_vtable(cur);
}

//xNext. Advances the cursor to next record of resultset.
int next_vtable(sqlite3_vtab_cursor *cur) {
  stlTable *st = (stlTable *)cur->pVtab;
  stlTableCursor *stc = (stlTableCursor *)cur;
  stc->current++;
#ifdef DEBUG
  printf("Table %s, now stc->current: %i \n\n", 
	 st->zName, stc->current);
#endif
  if (stc->current >= stc->size)
    stc->isEof = 1;
  return SQLITE_OK;
}


/* xOpen. Opens the virtual table struct to be used 
 * in an SQL query. Triggered by the FROM clause. 
 * Initialises cursor.
 */
int open_vtable(sqlite3_vtab *pVtab, 
		sqlite3_vtab_cursor **ppCsr) {
  stlTable *st=(stlTable *)pVtab;
  int arraySize;
#ifdef DEBUG
  printf("Opening vtable %s\n\n", st->zName);
#endif
  sqlite3_vtab_cursor *pCsr;    /* Allocated cursor */

  *ppCsr = pCsr = 
    (sqlite3_vtab_cursor *)sqlite3_malloc(sizeof(stlTableCursor));
  if (!pCsr) {
    return SQLITE_NOMEM;
  }
  stlTableCursor *stc = (stlTableCursor *)pCsr;
  memset(pCsr, 0, sizeof(stlTableCursor));
  /* Keep copy of initial data. Might change in search. 
   * Useful when multiple instances of the VT are open.
   */
  stc->source = st->data;
  /* To allocate space for the resultset.
   * Will need space at most equal to the data structure 
   * size. This is fixed for autonomous structs, variable 
   * for embedded ones (will be taken care of in search).
   */
  if (!st->embedded) {
    pCsr->pVtab = &st->vtab;
    arraySize = get_datastructure_size(pCsr);
    pCsr->pVtab = NULL;
  } else 
    /* Embedded struct. Size will be synced in search when 
     * powered from source.
     */
    arraySize = 1;
  stc->max_size = arraySize;
#ifdef DEBUG
  printf("ppCsr = %lx, pCsr = %lx \n", 
	 (long unsigned int)ppCsr, 
	 (long unsigned int)pCsr);
#endif 
  /* A data structure to hold index positions of resultset
   * so that in the end of the constraint evaluation the 
   * remaining resultset is the wanted one. 
   */
  stc->resultSet = (int *)sqlite3_malloc(sizeof(int) * 
					 arraySize); 
  if (!stc->resultSet) {
    return SQLITE_NOMEM;
  }
  memset(stc->resultSet, -1, sizeof(int) * arraySize);
  return SQLITE_OK;
}

/* xColumn. Calls the retrieve family of functions at 
 * stl_search.cpp. Returns the value of column $n for 
 * record pointed at by $cur.
 */
int column_vtable(sqlite3_vtab_cursor *cur, 
		  sqlite3_context *con, 
		  int n) {
  return retrieve(cur, n, con);
}

/* xRowid. Returns the rowid of record pointed at by $cur 
 * in integer format.
 */
int rowid_vtable(sqlite3_vtab_cursor *cur, 
		 sqlite_int64 *pRowid) {
}

/* xClose. Closes the virtual table after the completion 
 * of a query.
 */
int close_vtable(sqlite3_vtab_cursor *cur) {
  stlTable *st = (stlTable *)cur->pVtab;
  stlTableCursor *stc = (stlTableCursor *)cur;
#ifdef DEBUG
  printf("Closing vtable %s \n\n", st->zName);
#endif
  sqlite3_free(stc->resultSet);
  sqlite3_free(stc);
  return SQLITE_OK;
}

//xEof. Signifies the end of resultset.
int eof_vtable(sqlite3_vtab_cursor *cur) {
  return ((stlTableCursor *)cur)->isEof;
}

/* Fills virtual table module's function pointers with 
 * implemented callback functions.
 */
void fill_module(sqlite3_module *m) {
  m->iVersion = 1;
  m->xCreate = create_vtable;
  m->xConnect = connect_vtable;
  m->xBestIndex = best_index_vtable;
  m->xDisconnect = disconnect_vtable;
  m->xDestroy = destroy_vtable;
  m->xOpen = open_vtable;
  m->xClose = close_vtable;
  m->xEof = eof_vtable;
  m->xFilter = filter_vtable;
  m->xNext = next_vtable;
  m->xColumn = column_vtable;
  m->xRowid = rowid_vtable;
  m->xUpdate = 0;
  m->xFindFunction = 0;
  m->xBegin = 0;
  m->xSync = 0;
  m->xCommit = 0;
  m->xRollback = 0;
  m->xRename = 0;
}

/* Estimates the query length by counting the length of 
 * the parameters passed.
 */
int arrange_size(int argc, const char * const * argv) {
  /* Length of standard keywords of sql queries. */
  int length = 28;
  int i;
  /* + length for all keywords except db_name. + 1 for 
   * following identifier.
   */
  for (i = 0; i < argc; i++) {
    if (i != 1) length += strlen(argv[i]) + 1;
  }
  length += 1;         /*  Sentinel character. */
#ifdef DEBUG
  printf("length is %i \n",length);
#endif
  return length;
}