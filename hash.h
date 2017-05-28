/*
 *  hash structures
 */


#define HASHSIZE   51

typedef struct _hash HASH;


struct _hash {
  struct _hash  *next;
  char           ipstr[16];
  unsigned long  count;
};
