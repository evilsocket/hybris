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
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT T_NOT
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>       /* for strcmp, memcmp, etc */
#include <sys/types.h>    /* ULTRIX needs this for in.h */
#include <netinet/in.h>   /* for reading/writing hashtables */
#include <assert.h>
#include "hashtable.h"     /* all the types */

/* round num up to a multiple of wordsize.  (LOG_WORD_SIZE-3 is in bytes) */
#define WORD_ROUND(num)         ( ((num-1) | ((1<<(LOG_WORD_SIZE-3))-1)) + 1 )
/* Useful operations we do to keys: compare them, copy them, free them */
#define KEY_CMP(ht, key1, key2)      ( strcmp((char *)key1, (char *)key2) )
#define COPY_KEY(ht, keyTo, keyFrom) (keyTo) = (ulong)malloc( WORD_ROUND(strlen((char *)(keyFrom))+1) );  \
									 strcpy((char *)(keyTo), (char *)(keyFrom))

#define FREE_KEY(ht, key) ht_free((char *)(key), WORD_ROUND(strlen((char *)(key))+1))

   /* the following are useful for bitmaps */
   /* Format is like this (if 1 word = 4 bits):  3210 7654 ba98 fedc ... */
typedef ulong          HTBitmapPart;  /* this has to be unsigned, for >> */
typedef HTBitmapPart   HTBitmap[1<<LOG_BM_WORDS];
typedef ulong          HTOffset; /* something big enough to hold offsets */

#define BM_BYTES(cBuckets)   /* we must ensure it's a multiple of word size */\
   ( (((cBuckets) + 8*sizeof(ulong)-1) >> LOG_WORD_SIZE) << (LOG_WORD_SIZE-3) )
#define MOD2(i, logmod)      ( (i) & ((1<<(logmod))-1) )
#define DIV_NUM_ENTRIES(i)   ( (i) >> LOG_WORD_SIZE )
#define MOD_NUM_ENTRIES(i)   ( MOD2(i, LOG_WORD_SIZE) )
#define MODBIT(i)            ( ((ulong)1) << MOD_NUM_ENTRIES(i) )

#define TEST_BITMAP(bm, i)   ( (bm)[DIV_NUM_ENTRIES(i)] & MODBIT(i) ? 1 : 0 )
#define SET_BITMAP(bm, i)    (bm)[DIV_NUM_ENTRIES(i)] |= MODBIT(i)
#define CLEAR_BITMAP(bm, i)  (bm)[DIV_NUM_ENTRIES(i)] &= ~MODBIT(i)

/* ht_free() -- keep track of memory use
 *    frees memory using free, but updates count of how much memory
 *    is being used.
 */
__force_inline static void ht_free(void *ptr, ulong size)
{
   if ( size > 0 )         /* some systems seem to not like freeing NULL */
      free(ptr);
}

/* ======================================================================== */
/*                            TABLE ROUTINES                                */
/*                         --------------------                             */

/*  The idea is that a hashtable with (logically) t buckets is divided
 *  into t/M groups of M buckets each.  (M is a constant set in
 *  LOG_BM_WORDS for efficiency.)  Each group is stored sparsely.
 *  Thus, inserting into the table causes some array to grow, which is
 *  slow but still constant time.  Lookup involves doing a
 *  logical-position-to-sparse-position lookup, which is also slow but
 *  constant time.  The larger M is, the slower these operations are
 *  but the less overhead (slightly).
 *
 *  To store the sparse array, we store a bitmap B, where B[i] = 1 iff
 *  bucket i is non-empty.  Then to look up bucket i we really look up
 *  array[# of 1s before i in B].  This is constant time for fixed M.
 *
 *  Terminology: the position of an item in the overall table (from
 *  1 .. t) is called its "location."  The logical position in a group
 *  (from 1 .. M ) is called its "position."  The actual location in
 *  the array (from 1 .. # of non-empty buckets in the group) is
 *  called its "offset."
 *
 *  The following operations are supported:
 *     o Allocate an array with t buckets, all empty
 *     o Free a array (but not whatever was stored in the buckets)
 *     o Tell whether or not a bucket is empty
 *     o Return a bucket with a given location
 *     o Set the value of a bucket at a given location
 *     o Iterate through all the buckets in the array
 *     o Read and write an occupancy bitmap to disk
 *     o Return how much memory is being allocated by the array structure
 */

typedef struct dense_bin {      /* needs to be a struct for C typing reasons */
   hash_item_t *rgBuckets;      /* A bin is an array of buckets */
}
dense_bin_t;

typedef struct dense_iterator {
   long pos;               /* the actual iterator */
   dense_bin_t *bin;       /* state info, to avoid args for NextBucket() */
   ulong cBuckets;
}
dense_iterator_t;

#define DENSE_IS_EMPTY(bin, i)     ( (bin)[i].data == EMPTY )
#define DENSE_SET_EMPTY(bin, i)    (bin)[i].data = EMPTY      /* fks-hash.h */
#define DENSE_SET_OCCUPIED(bin, i) (bin)[i].data = 1          /* not EMPTY */

__force_inline static void DenseTableClear(dense_bin_t *bin, ulong cBuckets)
{
   while ( cBuckets-- )
      DENSE_SET_EMPTY(bin->rgBuckets, cBuckets);
}

__force_inline static ulong DenseTableAllocate(dense_bin_t **pbin, ulong cBuckets)
{
   *pbin = (dense_bin_t *) malloc(sizeof(*pbin));
   (*pbin)->rgBuckets = (hash_item_t *) malloc(sizeof(*(*pbin)->rgBuckets)
						  * cBuckets);
   DenseTableClear(*pbin, cBuckets);
   return cBuckets;
}

__force_inline static dense_bin_t *DenseTableFree(dense_bin_t *bin, ulong cBuckets)
{
   ht_free(bin->rgBuckets, sizeof(*bin->rgBuckets) * cBuckets);
   ht_free(bin, sizeof(*bin));
   return NULL;
}

__force_inline static int DenseTableIsEmpty(dense_bin_t *bin, ulong location)
{
   return DENSE_IS_EMPTY(bin->rgBuckets, location);
}

__force_inline static hash_item_t *DenseTableFind(dense_bin_t *bin, ulong location)
{
   if ( DenseTableIsEmpty(bin, location) )
      return NULL;
   return bin->rgBuckets + location;
}

__force_inline static hash_item_t *DenseTableInsert(dense_bin_t *bin, hash_item_t *bckInsert, ulong location, int *pfOverwrite)
{
   hash_item_t *bckPlace;

   bckPlace = DenseTableFind(bin, location);
   if ( bckPlace )                /* means something is already there */
   {
      if ( *pfOverwrite )
	 *bckPlace = *bckInsert;
      *pfOverwrite = 1;           /* set to 1 to indicate someone was there */
      return bckPlace;
   }
   else
   {
      bin->rgBuckets[location] = *bckInsert;
      *pfOverwrite = 0;
      return bin->rgBuckets + location;
   }
}

__force_inline static hash_item_t *DenseTableNextBucket(dense_iterator_t *iter)
{
	hash_item_t *buckets = iter->bin->rgBuckets;
   for ( ++iter->pos; iter->pos < iter->cBuckets; ++iter->pos )
      if ( !DENSE_IS_EMPTY( buckets, iter->pos ) )
	 	return buckets + iter->pos;
   return NULL;                        /* all remaining groups were empty */
}

__force_inline static hash_item_t *DenseTableFirstBucket(dense_iterator_t *iter, dense_bin_t *bin, ulong cBuckets)
{
   iter->bin = bin;                    /* set it up for NextBucket() */
   iter->cBuckets = cBuckets;
   iter->pos = -1;                     /* thus the next bucket will be 0 */
   return DenseTableNextBucket(iter);
}

__force_inline static ulong DenseTableMemory(ulong cBuckets, ulong cOccupied)
{
   return cBuckets * sizeof(hash_item_t);
}


/* ======================================================================== */
/*                          HASHING ROUTINES                                */
/*                       ----------------------                             */

/*  Implements a simple quadratic hashing scheme.  We have a single hash
 *  table of size t and a single hash function h(x).  When inserting an
 *  item, first we try h(x) % t.  If it's occupied, we try h(x) +
 *  i*(i-1)/2 % t for increasing values of i until we hit a not-occupied
 *  space.  To make this dynamic, we double the size of the hash table as
 *  soon as more than half the cells are occupied.  When deleting, we can
 *  choose to shrink the hashtable when less than a quarter of the
 *  cells are occupied, or we can choose never to shrink the hashtable.
 *  For lookup, we check h(x) + i*(i-1)/2 % t (starting with i=0) until
 *  we get a match or we hit an empty space.  Note that as a result,
 *  we can't make a cell empty on deletion, or lookups may end prematurely.
 *  Instead we mark the cell as "deleted."  We thus steal the value
 *  DELETED as a possible "data" value.  As long as data are pointers,
 *  that's ok.
 *     The hash increment we use, i(i-1)/2, is not the standard quadratic
 *  hash increment, which is i^2.  i(i-1)/2 covers the entire bucket space
 *  when the hashtable size is a power of two, as it is for us.  In fact,
 *  the first n probes cover n distinct buckets; then it repeats.  This
 *  guarantees insertion will always succeed.
 *     If you linear hashing, set JUMP in chash.h.  You can also change
 *  various other parameters there.
 */

/*************************************************************************\
| Hash()                                                                  |
|     The hash function I use is due to Bob Jenkins (see                  |
|     http://burtleburtle.net/bob/hash/evahash.html                       |
|     According to http://burtleburtle.net/bob/c/lookup2.c,               |
|     his implementation is public domain.)                               |
|     It takes 36 instructions, in 18 cycles if you're lucky.             |
|        hashing depends on the fact the hashtable size is always a       |
|     power of 2.  cBuckets is probably ht->cBuckets.                     |
\*************************************************************************/

#if LOG_WORD_SIZE == 5                      /* 32 bit words */

#define mix(a,b,c) \
{ \
  a -= b; a -= c; a ^= (c>>13); \
  b -= c; b -= a; b ^= (a<<8); \
  c -= a; c -= b; c ^= (b>>13); \
  a -= b; a -= c; a ^= (c>>12);  \
  b -= c; b -= a; b ^= (a<<16); \
  c -= a; c -= b; c ^= (b>>5); \
  a -= b; a -= c; a ^= (c>>3);  \
  b -= c; b -= a; b ^= (a<<10); \
  c -= a; c -= b; c ^= (b>>15); \
}
#ifdef WORD_HASH                 /* play with this on little-endian machines */
#define WORD_AT(ptr)    ( *(ulong *)(ptr) )
#else
#define WORD_AT(ptr)    ( (ptr)[0] + ((ulong)(ptr)[1]<<8) + \
			  ((ulong)(ptr)[2]<<16) + ((ulong)(ptr)[3]<<24) )
#endif

#elif LOG_WORD_SIZE == 6        /* 64 bit words */

#define mix(a,b,c) \
{ \
  a -= b; a -= c; a ^= (c>>43); \
  b -= c; b -= a; b ^= (a<<9); \
  c -= a; c -= b; c ^= (b>>8); \
  a -= b; a -= c; a ^= (c>>38); \
  b -= c; b -= a; b ^= (a<<23); \
  c -= a; c -= b; c ^= (b>>5); \
  a -= b; a -= c; a ^= (c>>35); \
  b -= c; b -= a; b ^= (a<<49); \
  c -= a; c -= b; c ^= (b>>11); \
  a -= b; a -= c; a ^= (c>>12); \
  b -= c; b -= a; b ^= (a<<18); \
  c -= a; c -= b; c ^= (b>>22); \
}
#ifdef WORD_HASH                 /* alpha is little-endian, btw */
#define WORD_AT(ptr)    ( *(ulong *)(ptr) )
#else
#define WORD_AT(ptr)    ( (ptr)[0] + ((ulong)(ptr)[1]<<8) + \
			  ((ulong)(ptr)[2]<<16) + ((ulong)(ptr)[3]<<24) + \
			  ((ulong)(ptr)[4]<<32) + ((ulong)(ptr)[5]<<40) + \
			  ((ulong)(ptr)[6]<<48) + ((ulong)(ptr)[7]<<56) )
#endif

#else                            /* neither 32 or 64 bit words */
#error This hash function can only hash 32 or 64 bit words.  Sorry.
#endif

__force_inline static ulong Hash(hash_table_t *ht, char *key, ulong cBuckets)
{
   ulong a, b, c, cchKey, cchKeyOrig,
	     sizeof_ulong_3 = 3 * sizeof(ulong),
	     sizeof_ulong_2 = 2 * sizeof(ulong);

   cchKeyOrig = strlen(key);
   /*
    * The golden ratio; an arbitrary value.
    *
    * BUG!
    *
    * If it's not appropriately high, may cause hash collisions!
    */
   a = b = c = 0xfe3779b9;

   for( cchKey = cchKeyOrig; cchKey >= sizeof_ulong_3; cchKey -= sizeof_ulong_3, key += sizeof_ulong_3 ) {
      a += WORD_AT(key);
      b += WORD_AT(key + sizeof(ulong) );
      c += WORD_AT(key + sizeof_ulong_2 );

      mix(a,b,c);
   }

   c += cchKeyOrig;
   switch ( cchKey ) {           /* deal with rest.  Cases fall through */
#if LOG_WORD_SIZE == 5
      case 11: c += (ulong)key[10]<<24;
      case 10: c += (ulong)key[9]<<16;
      case 9 : c += (ulong)key[8]<<8;
               /* the first byte of c is reserved for the length */
      case 8 : b += WORD_AT(key+4);  a+= WORD_AT(key);  break;
      case 7 : b += (ulong)key[6]<<16;
      case 6 : b += (ulong)key[5]<<8;
      case 5 : b += key[4];
      case 4 : a += WORD_AT(key);  break;
      case 3 : a += (ulong)key[2]<<16;
      case 2 : a += (ulong)key[1]<<8;
      case 1 : a += key[0];
   /* case 0 : nothing left to add */
#elif LOG_WORD_SIZE == 6
      case 23: c += (ulong)key[22]<<56;
      case 22: c += (ulong)key[21]<<48;
      case 21: c += (ulong)key[20]<<40;
      case 20: c += (ulong)key[19]<<32;
      case 19: c += (ulong)key[18]<<24;
      case 18: c += (ulong)key[17]<<16;
      case 17: c += (ulong)key[16]<<8;
               /* the first byte of c is reserved for the length */
      case 16: b += WORD_AT(key+8);  a+= WORD_AT(key);  break;
      case 15: b += (ulong)key[14]<<48;
      case 14: b += (ulong)key[13]<<40;
      case 13: b += (ulong)key[12]<<32;
      case 12: b += (ulong)key[11]<<24;
      case 11: b += (ulong)key[10]<<16;
      case 10: b += (ulong)key[ 9]<<8;
      case  9: b += (ulong)key[ 8];
      case  8: a += WORD_AT(key);  break;
      case  7: a += (ulong)key[ 6]<<48;
      case  6: a += (ulong)key[ 5]<<40;
      case  5: a += (ulong)key[ 4]<<32;
      case  4: a += (ulong)key[ 3]<<24;
      case  3: a += (ulong)key[ 2]<<16;
      case  2: a += (ulong)key[ 1]<<8;
      case  1: a += (ulong)key[ 0];
   /* case 0: nothing left to add */
#endif
   }
   mix(a,b,c);
   return c & (cBuckets-1);
}


/*************************************************************************\
| Rehash()                                                                |
|     You give me a hashtable, a new size, and a bucket to follow, and    |
|     I resize the hashtable's bin to be the new size, rehashing          |
|     everything in it.  I keep particular track of the bucket you pass   |
|     in, and RETURN a pointer to where the item in the bucket got to.    |
|     (If you pass in NULL, I return an arbitrary pointer.)               |
\*************************************************************************/

__force_inline static hash_item_t *Rehash(hash_table_t *ht, ulong cNewBuckets, hash_item_t *bckWatch)
{
   dense_bin_t *tableNew;
   ulong iBucketFirst;
   hash_item_t *bck, *bckNew(NULL);
   ulong offset;         /* the i in h(x) + i*(i-1)/2 */
   int fOverwrite(0);    /* not an issue: there can be no collisions */

   cNewBuckets = DenseTableAllocate(&tableNew, cNewBuckets);

  /* Since we RETURN the new position of bckWatch, we want  *
   * to make sure it doesn't get moved due to some table    *
   * rehashing that comes after it's inserted.  Thus, we    *
   * have to put it in last.  This makes the loop weird.    */
   for( bck = ht_first_bucket(ht); ;bck = ht_next_bucket(ht) ) {
	   /* we're done iterating, so look at bckWatch */
	   if( bck == NULL ){
		   bck = bckWatch;
		   /* I guess bckWatch wasn't specified */
		   if( bck == NULL )
			   break;
      }
      else if( bck == bckWatch )
    	  /* ignore if we see it during the iteration */
    	  continue;

      offset = 0;                              /* a new i for a new bucket */
      for( iBucketFirst = Hash(ht, (char *)(bck->key), cNewBuckets);
           !DenseTableIsEmpty(tableNew, iBucketFirst);
           iBucketFirst = (iBucketFirst + JUMP(KEY_PTR(ht,bck->key), offset)) & (cNewBuckets-1) );

      bckNew = DenseTableInsert(tableNew, bck, iBucketFirst, &fOverwrite);
      /* we're done with the last thing to do */
      if( bck == bckWatch )
    	  break;
   }

   DenseTableFree(ht->table, ht->cBuckets);

   ht->table 		 = tableNew;
   ht->cBuckets 	 = cNewBuckets;
   ht->cDeletedItems = 0;

   /* new position of bckWatch, which was inserted last */
   return bckNew;
}

/*************************************************************************\
| Find()                                                                  |
|     Does the quadratic searching stuff.  RETURNS NULL if we don't       |
|     find an object with the given key, and a pointer to the Item        |
|     holding the key, if we do.  Also sets posLastFind.  If piEmpty is   |
|     non-NULL, we set it to the first open bucket we pass; helpful for   |
|     doing a later insert if the search fails, for instance.             |
\*************************************************************************/

__force_inline static hash_item_t *Find(hash_table_t *ht, ulong key, ulong *piEmpty)
{
   ulong iBucketFirst;
   hash_item_t *item;
   ulong offset = 0;              /* the i in h(x) + i*(i-1)/2 */
   int fFoundEmpty = 0;           /* set when we pass over an empty bucket */

   ht->posLastFind = NULL;        /* set up for failure: a new find starts */

   iBucketFirst = Hash(ht, (char *)(key), ht->cBuckets);

   /* now try all i > 0 */
   while( 1 ) {
      item = DenseTableFind( ht->table, iBucketFirst );
      /* it's not in the table */
      if( item == NULL ) {
		 if( piEmpty && !fFoundEmpty ) *piEmpty = iBucketFirst;
		 return NULL;
      }
      /* item found */
	  else {
		   ht->posLastFind = item;
		   return item;               /* we found it! */
	  }

      iBucketFirst = ((iBucketFirst + JUMP((char *)(key), offset)) & (ht->cBuckets-1));
   }
}

/*************************************************************************\
| Insert()                                                                |
|     If an item with the key already exists in the hashtable, RETURNS    |
|     a pointer to the item (replacing its data if fOverwrite is 1).      |
|     If not, we find the first place-to-insert (which Find() is nice     |
|     enough to set for us) and insert the item there, RETURNing a        |
|     pointer to the item.  We might grow the hashtable if it's getting   |
|     full.  Note we include buckets holding DELETED when determining     |
|     fullness, because they slow down searching.                         |
\*************************************************************************/

__force_inline static ulong NextPow2(ulong x)    /* returns next power of 2 > x, or 2^31 */
{
   if ( ((x << 1) >> 1) != x )    /* next power of 2 overflows */
      x >>= 1;                    /* so we return highest power of 2 we can */
   while ( (x & (x-1)) != 0 )     /* blacks out all but the top bit */
      x &= (x-1);
   return x << 1;                 /* makes it the *next* power of 2 */
}

__force_inline static hash_item_t *Insert(hash_table_t *ht, ulong key, ulong data, int fOverwrite, int skip_search = 0 )
{
   hash_item_t *item, bckInsert;
   ulong iEmpty;                  /* first empty bucket key probes */


   if( skip_search == 0 ){
	   item = Find(ht, key, &iEmpty);
	   if ( item && fOverwrite ) {
		  item->data = data;       /* key already matches */
		  return item;
	   }
   }

   ht->posLastFind = NULL;        /* last operation is insert, not find */

   COPY_KEY(ht, bckInsert.key, key);    /* make our own copy of the key */
   bckInsert.data = data;               /* oh, and the data too */
   item = DenseTableInsert(ht->table, &bckInsert, iEmpty, &fOverwrite);
   if ( fOverwrite )                    /* we overwrote a deleted bucket */
      ht->cDeletedItems--;
   ++ht->cItems;                        /* insert couldn't have overwritten */
   if ( ht->cDeltaGoalSize > 0 )  /* closer to our goal size */
      ht->cDeltaGoalSize--;
   if ( ht->cItems + ht->cDeletedItems >= ht->cBuckets * OCCUPANCY_PCT || ht->cDeltaGoalSize < 0 ) /* we must've overestimated # of deletes */
      item = Rehash( ht,
                     NextPow2((ulong)(((ht->cDeltaGoalSize > 0 ?
				     ht->cDeltaGoalSize : 0)
                     + ht->cItems) / OCCUPANCY_PCT)),
		    item);
   return item;
}

/* ======================================================================== */
/*                          USER-VISIBLE API                                */
/*                       ----------------------                             */

/*************************************************************************\
| ht_alloc()                                                     |
| ht_clear()                                                        |
| ht_free()                                                         |
|     Allocate() allocates a hash table and sets up size parameters.      |
|     Free() frees it.  Clear() deletes all the items from the hash       |
|     table, but frees not.                                               |
|        cchKey is < 0 if the keys you send me are meant to be pointers   |
|     to \0-terminated strings.  Then -cchKey is the maximum key size.    |
|     If cchKey < one word (ulong), the keys you send me are the keys     |
|     themselves; else the keys you send me are pointers to the data.     |
|        If fSaveKeys is 1, we copy any keys given to us to insert.  We   |
|     also free these keys when freeing the hash table.  If it's 0, the   |
|     user is responsible for key space management.                       |
|        ht_alloc() RETURNS a hash table; the others TAKE one.   |
\*************************************************************************/

hash_table_t *ht_alloc(int cchKey, int fSaveKeys)
{
   hash_table_t *ht;

   ht = (hash_table_t *) malloc(sizeof(hash_table_t));   /* set everything to 0 */
   ht->cBuckets = DenseTableAllocate(&ht->table, MIN_HASH_SIZE);
   ht->cchKey = cchKey <= 0 ? 0 : cchKey;
   ht->cItems = 0;
   ht->cDeletedItems = 0;
   ht->fSaveKeys = fSaveKeys;
   ht->cDeltaGoalSize = 0;
   ht->iter = (dense_iterator_t *)malloc( sizeof(dense_iterator_t) );

   return ht;
}

void ht_clear(hash_table_t *ht)
{
   hash_item_t *bck;

   if ( ht->fSaveKeys )       /* need to free keys */
      for ( bck = ht_first_bucket(ht); bck; bck = ht_next_bucket(ht) )
      {
	 FREE_KEY(ht, bck->key);
	 if ( ht->fSaveKeys == 2 )  /* this means key stored in one block */
	    break;                  /* ...so only free once */
      }
   DenseTableFree(ht->table, ht->cBuckets);
   ht->cBuckets = DenseTableAllocate(&ht->table, MIN_HASH_SIZE);

   ht->cItems = 0;
   ht->cDeletedItems = 0;
   ht->cDeltaGoalSize = 0;
   ht->posLastFind = NULL;
}

void ht_free(hash_table_t *ht)
{
   ht_clear(ht);
   if ( ht->iter )    ht_free(ht->iter, sizeof(dense_iterator_t));
   if ( ht->table )   DenseTableFree(ht->table, ht->cBuckets);
   free(ht);
}

/*************************************************************************\
| ht_find()                                                              |
| ht_find_last()                                                          |
|     ht_find(): looks in h(x) + i(i-1)/2 % t as i goes up from 0        |
|     until we either find the key or hit an empty bucket.  RETURNS a     |
|     pointer to the item in the hit bucket, if we find it, else          |
|     RETURNS NULL.                                                       |
|        ht_find_last() returns the item returned by the last             |
|     ht_find(), which may be NULL if the last ht_find() failed.        |
|        LOAD_AND_RETURN reads the data from off disk, if necessary.      |
\*************************************************************************/

hash_item_t *ht_find(hash_table_t *ht, ulong key)
{
   return Find( ht, key, NULL );
}

hash_item_t *ht_find_last(hash_table_t *ht)
{
   return ht->posLastFind;
}

/*************************************************************************\
| ht_find_or_insert()                                                      |
| ht_find_or_insert_item()                                                  |
| ht_insert()                                                            |
| ht_insert_item()                                                        |
| ht_delete()                                                            |
| ht_delete_last()                                                        |
|     Pretty obvious what these guys do.  Some take buckets (items),      |
|     some take keys and data separately.  All things RETURN the bucket   |
|     (a pointer into the hashtable) if appropriate.                      |
\*************************************************************************/

hash_item_t *ht_find_or_insert(hash_table_t *ht, ulong key, ulong dataInsert)
{
      /* This is equivalent to Insert without samekey-overwrite */
   return Insert(ht, key, dataInsert, 0);
}

hash_item_t *ht_find_or_insert_item(hash_table_t *ht, hash_item_t *pItem)
{
   return ht_find_or_insert(ht, pItem->key, pItem->data);
}

hash_item_t *ht_insert(hash_table_t *ht, ulong key, ulong data, int skip_search /*= 0*/ )
{
   return Insert(ht, key, data, SAMEKEY_OVERWRITE, skip_search );
}

hash_item_t *ht_insert_item(hash_table_t *ht, hash_item_t *pItem)
{
   return ht_insert(ht, pItem->key, pItem->data);
}

/*************************************************************************\
| ht_first_bucket()                                                       |
| ht_next_bucket()                                                        |
|     Iterates through the items in the hashtable by iterating through    |
|     the table.  Since we know about deleted buckets and loading data    |
|     off disk, and the table doesn't, our job is to take care of these   |
|     things.  RETURNS a bucket, or NULL after the last bucket.           |
\*************************************************************************/

hash_item_t *ht_first_bucket(hash_table_t *ht)
{
	return DenseTableFirstBucket(ht->iter, ht->table, ht->cBuckets);
}

hash_item_t *ht_next_bucket(hash_table_t *ht)
{
	return DenseTableNextBucket(ht->iter);
}

/*************************************************************************\
| ht_set_delta_goal_size()                                                  |
|     If we're going to insert 100 items, set the delta goal size to      |
|     100 and we take that into account when inserting.  Likewise, if     |
|     we're going to delete 10 items, set it to -100 and we won't         |
|     rehash until all 100 have been done.  It's ok to be wrong, but      |
|     it's efficient to be right.  Returns the delta value.               |
\*************************************************************************/

int ht_set_delta_goal_size(hash_table_t *ht, int delta)
{
   ht->cDeltaGoalSize = delta;
#if FAST_DELETE == 1 || defined INSERT_ONLY
   if ( ht->cDeltaGoalSize < 0 )   /* for fast delete, we never */
      ht->cDeltaGoalSize = 0;      /* ...rehash after deletion  */
#endif
   return ht->cDeltaGoalSize;
}
