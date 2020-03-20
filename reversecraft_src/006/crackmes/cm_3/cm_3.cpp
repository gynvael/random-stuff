#include <stdio.h>
#include <string.h>
#include <stdlib.h>

char EncryptedPassword[] = "<?php $pass = 'HASLO_AllSoulsNight'; $k = 0x3; for($i = 0; $i < strlen($pass); $i++, $k += 0x3) echo chr(ord($pass[$i]) ^ $k); ?>";

int
main(void)
{
  char Haslo[32];

  char *DecryptedHaslo = (char*)malloc(64);

  int i, k = 0x3;

  printf("Podaj haslo: ");
  scanf("%31s", Haslo);

  for(i = 0; i < 19; i++, k += 0x3)
    DecryptedHaslo[i] = EncryptedPassword[i] ^ k;

  DecryptedHaslo[i] = 0;

  if(strcmp(Haslo, DecryptedHaslo) == 0)
  {
    puts("Gratulacje!");
  }
  else
  {
    puts("Nope ;<");
  }

  return 0;
}


