#include <string.h>
#include <stdlib.h>
#include "hash.h"


/*
 *  hash-functions
 */


inline static unsigned hash_make(char *s)
{
  unsigned hashval;

  for (hashval = 0 ; *s != 0 ; s++) 
    hashval = *s + 31 * hashval;
  
  return hashval % HASHSIZE;
}


HASH *hash_lookup(HASH **hashtab, char *ipstr)
{
  HASH *np;

  for (np = hashtab[hash_make(ipstr)] ; np != NULL ; np = np->next) {
    if (!strcmp(np->ipstr, ipstr))
      return np;
  }
  
  return NULL;
}


HASH *hash_install(HASH **hashtab, char *ipstr)
{
  HASH *np;
  unsigned hashval;

  if ((np = hash_lookup(hashtab, ipstr)) == NULL) {
    np = malloc(sizeof(HASH));
    strcpy(np->ipstr, ipstr);
    np->count = 0;
    hashval = hash_make(ipstr);
    np->next = hashtab[hashval];
    hashtab[hashval] = np;
  } 
  
  return np;
}


HASH **hash_init()
{
  int j;
  HASH **hashtab;

  hashtab = malloc(HASHSIZE * sizeof(HASH));
  for (j = 0 ; j < HASHSIZE ; j++)
    hashtab[j] = NULL;

  return hashtab;
}



HASH **hash_free(HASH **hashtab)
{
  unsigned long i;
  HASH *data;

  if (hashtab == NULL)
    return NULL;

  for (i = 0 ; i < HASHSIZE ; i++) {
    if (hashtab[i] == NULL)
      continue;
    for (data = hashtab[i] ; data != NULL ; ) { 
      hashtab[i] = data->next;
      free(data);
      data = hashtab[i];
    }
  }
  free(hashtab);
  hashtab = NULL;

  return hashtab;
}


HASH **hash_foreach(HASH **hashtab, void (*hashdo)(HASH *hashtab))
{
  unsigned long i;
  HASH *data;

  if (hashtab == NULL)
    return NULL;

  for (i = 0 ; i < HASHSIZE ; i++) {
    if (hashtab[i] == NULL)
      continue;
    for (data = hashtab[i] ; data != NULL ; ) { 
      hashtab[i] = data->next;
      (*hashdo)(data);
      data = hashtab[i];
    }
  }

  return hashtab;
}
