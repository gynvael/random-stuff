#include <stdio.h>
#include <string.h>

int
main(void)
{
  char Haslo[32];
  printf("Podaj haslo: ");
  scanf("%31s", Haslo);

  if(strcmp(Haslo, "NukeThemFromOrbit") == 0)
  {
    puts("Gratulacje!");
  }
  else
  {
    puts("Nope ;<");
  }

  return 0;
}


