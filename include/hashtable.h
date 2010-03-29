/* Copyright (c) 1998 - 2005, Google Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _HHASHTABLE_H_
#   define _HHASHTABLE_H_


#include <sys/types.h>         /* includes definition of "ulong", we hope */
#define ulong u_long

#ifndef LOG_WORD_SIZE                  /* 5 for 32 bit words, 6 for 64 */
#	if defined (__LP64__) || defined (_LP64)
#		define LOG_WORD_SIZE          6       /* log_2(sizeof(ulong)) [in bits] */
#else
#	define LOG_WORD_SIZE          5       /* log_2(sizeof(ulong)) [in bits] */
#	endif
#endif

   /* The following gives a speed/time tradeoff: how many buckets are  *
    * in each bin.  0 gives 32 buckets/bin, which is a good number.    */
#ifndef LOG_BM_WORDS
#	define LOG_BM_WORDS        0      /* each group has 2^L_B_W * 32 buckets */
#endif

   /* The following are all parameters that affect performance. */
#ifndef JUMP
#	define JUMP(key, offset)   ( ++(offset) )  /* ( 1 ) for linear hashing */
#endif

#ifndef FAST_DELETE
#	define FAST_DELETE         1      /* if it's 1, we never shrink the ht */
#endif
#ifndef SAMEKEY_OVERWRITE
#	define SAMEKEY_OVERWRITE   1      /* overwrite item with our key on insert? */
#endif
#ifndef OCCUPANCY_PCT
#	define OCCUPANCY_PCT       0.5    /* large PCT means smaller and slower */
#endif
#ifndef MIN_HASH_SIZE
#	define MIN_HASH_SIZE       512    /* ht size when first created */
#endif
   /* When deleting a bucket, we can't just empty it (future hashes  *
    * may fail); instead we set the data field to DELETED.  Thus you *
    * should set DELETED to a data value you never use.  Better yet, *
    * if you don't need to delete, define INSERT_ONLY.               */
#ifndef INSERT_ONLY
#	define DELETED                   -2UL
#	define IS_BCK_DELETED(bck)       ( (bck) && (bck)->data == DELETED )
#	define SET_BCK_DELETED(ht, bck)  do { (bck)->data = DELETED;                \
                                       FREE_KEY(ht, (bck)->key); } while ( 0 )
#else
#	define IS_BCK_DELETED(bck)       0
#	define SET_BCK_DELETED(ht, bck)  \
									 do { fprintf(stderr, "Deletion not supported for insert-only hashtable\n");\
									 exit(2); } while ( 0 )
#endif

   /* We need the following only for dense buckets (Dense##x above).  *
    * If you need to, set this to a value you'll never use for data.  */
#define EMPTY -3UL                /* steal more of the bck->data space */


   /* This is what an item is.  Either can be cast to a pointer. */
typedef struct {
   ulong data;        /* 4 bytes for data: either a pointer or an integer */
   ulong key;         /* 4 bytes for the key: either a pointer or an int */
} hash_item_t;

struct dense_bin;                            /* defined in chash.c, I hope */
struct dense_iterator;
typedef struct dense_iterator dense_iterator_t;
typedef struct dense_bin dense_bin_t;

   /* for STORES_PTR to work ok, cchKey MUST BE DEFINED 1st, cItems 2nd! */
typedef struct hash_table {
   ulong cchKey;        /* the length of the key, or if it's \0 terminated */
   ulong cItems;        /* number of items currently in the hashtable */
   ulong cDeletedItems; /* # of buckets holding DELETE in the hashtable */
   ulong cBuckets;      /* size of the table */
   dense_bin_t *table;        /* The actual contents of the hashtable */
   int fSaveKeys;       /* 1 if we copy keys locally; 2 if keys in one block */
   int cDeltaGoalSize;  /* # of coming inserts (or deletes, if <0) we expect */
   hash_item_t *posLastFind; /* position of last Find() command */
   dense_iterator_t *iter; /* used in First/NextBucket */

   FILE *fpData;        /* if non-NULL, what item->data points into */
   char * (*dataRead)(FILE *, int);   /* how to load data from disk */
   hash_item_t bckData;      /* holds data after being loaded from disk */
} hash_table_t;

   /* Small keys are stored and passed directly, but large keys are
    * stored and passed as pointers.  To make it easier to remember
    * what to pass, we provide two functions:
    *   PTR_KEY: give it a pointer to your data, and it returns
    *            something appropriate to send to Hash() functions or
    *            be stored in a data field.
    *   KEY_PTR: give it something returned by a Hash() routine, and
    *            it returns a (char *) pointer to the actual data.
    */
#define HashKeySize(ht)   ( ((ulong *)(ht))[0] )  /* this is how we inline */
#define HashSize(ht)      ( ((ulong *)(ht))[1] )  /* ...a la C++ :-) */

#define STORES_PTR(ht)    ( HashKeySize(ht) == 0 || \
			    HashKeySize(ht) > sizeof(ulong) )
#define KEY_PTR(ht, key)  ( STORES_PTR(ht) ? (char *)(key) : (char *)&(key) )

#ifdef DONT_HAVE_TO_WORRY_ABOUT_BUS_ERRORS
#	define PTR_KEY( ht, ptr )  ( STORES_PTR(ht) ? (ulong)(ptr) : *(ulong *)(ptr) )
#else
#	define PTR_KEY( ht, ptr )  ( STORES_PTR(ht) ? (ulong)(ptr) : ht_copy((char *)ptr))
#endif

/* for PTR_KEY, not for users */
unsigned long ht_copy				 ( char *pul );
hash_table_t *ht_alloc				 ( int cchKey, int fSaveKeys );
void 		  ht_clear 				 ( hash_table_t *ht );
void 		  ht_free  				 ( hash_table_t *ht );
/* search functions */
hash_item_t  *ht_find				 ( hash_table_t *ht, ulong key );
hash_item_t  *ht_find_last			 ( hash_table_t *ht );
hash_item_t  *ht_find_or_insert      ( hash_table_t *ht, ulong key, ulong dataInsert );
hash_item_t  *ht_find_or_insert_item ( hash_table_t *ht, hash_item_t *pItem );
/* insert functions */
hash_item_t  *ht_insert				 ( hash_table_t *ht, ulong key, ulong data );
hash_item_t  *ht_insert_item		 ( hash_table_t *ht, hash_item_t *pItem );
/* delete functions */
int 		  ht_delete				 ( hash_table_t *ht, ulong key );
int 		  ht_delete_last		 ( hash_table_t *ht );
/* bucket functions */
hash_item_t  *ht_first_bucket 		 ( hash_table_t *ht );
hash_item_t  *ht_next_bucket		 ( hash_table_t *ht );
/* set eventually predicted hash table size */
int 		  ht_set_delta_goal_size ( hash_table_t *ht, int delta );

#endif
