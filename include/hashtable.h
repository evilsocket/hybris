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
#include <stdio.h>

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

#ifndef INSERT_ONLY
#	define INSERT_ONLY         1
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
} hash_table_t;

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
hash_item_t  *ht_insert				 ( hash_table_t *ht, ulong key, ulong data, int skip_search = 0 );
hash_item_t  *ht_insert_item		 ( hash_table_t *ht, hash_item_t *pItem );
/* bucket functions */
hash_item_t  *ht_first_bucket 		 ( hash_table_t *ht );
hash_item_t  *ht_next_bucket		 ( hash_table_t *ht );
/* set eventually predicted hash table size */
int 		  ht_set_delta_goal_size ( hash_table_t *ht, int delta );

#endif
