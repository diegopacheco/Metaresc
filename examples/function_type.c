
#include <stdio.h>
#include <stdlib.h>

#include <metaresc.h>

TYPEDEF_FUNC (ATTRIBUTES (__attribute__ ((aligned (sizeof (int64_t)))) , "function type sample"), int, func_t, (char *, int, int *, double))

int main ()
{
  char * serialized = MR_SAVE_CINIT (mr_td_t, mr_get_td_by_name ("func_t"));
  if (NULL == serialized)
    printf ("Serialization failed\n");
  else
    {
      printf ("%s\n", serialized);
      MR_FREE (serialized);
    }
  return (EXIT_SUCCESS);
}
