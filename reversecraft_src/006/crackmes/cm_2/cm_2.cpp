#include <stdio.h>
#include <string.h>

char EncryptedPassword[] = "<?php $pass = 'HASLO_CodeUrDreams'; $k = 0x1; for($i = 0; $i < strlen($pass); $i++, $k += 0x1) echo chr(ord($pass[$i]) ^ $k); ?>";

int
main(void)
{
  char DecryptedHaslo[32], Haslo[32];

  int i, k = 0x1;

  for(i = 0; i < 18; i++, k += 0x1)
    DecryptedHaslo[i] = EncryptedPassword[i] ^ k;

  DecryptedHaslo[i] = 0;

  printf("Podaj haslo: ");
  scanf("%31s", Haslo);

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


