#include <stdio.h>
#include <assert.h>

#define HEI_INF( ... ) \
{ \
  printf( "I> " __VA_ARGS__ ); \
  printf( "\n" ); \
}

#define HEI_ERR( ... ) \
{ \
  printf( "E> " __VA_ARGS__ ); \
  printf( "\n" ); \
}

#define HEI_ASSERT( expression ) \
  assert( expression );

