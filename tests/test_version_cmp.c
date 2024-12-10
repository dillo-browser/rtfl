// Used for "../configure.ac".

#include <ctype.h>
#include <stdlib.h>

static int version_cmp (const char *v1, const char *v2)
{
   const char *s1 = v1, *s2 = v2;
   while (*s1 && *s2) {
      if (isdigit (*s1) && isdigit (*s2)) {
         char buf1[10], buf2[10];
         int n1 = 0, n2 = 0;

         while (isdigit (*s1)) {
            if (n1 < 9) buf1[n1++] = *s1;
            s1++;
         }

         while (isdigit (*s2)) {
            if (n2 < 9) buf2[n2++] = *s2;
            s2++;
         }

         buf1[n1] = buf2[n2] = 0;
         int c = atoi (buf1) - atoi (buf2);
         if (c != 0)
            return c;
      } else {
         if (*s1 != *s2)
            return *s1 - *s2;
         s1++;
         s2++;
      }
   }

   return *s1 - *s2;
}

#include <stdio.h>

int main (int argc, char *argv[])
{
   printf ("%d, %d, %d %d\n",
           version_cmp ("0.1", "0.001"),
           version_cmp ("0.1", "0.002"),
           version_cmp ("0.1a", "0.1"),
           version_cmp ("2.38.0", "2.38.1"));
   return 0;
}
