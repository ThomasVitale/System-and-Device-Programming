#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char* argv[]) {
  int n, t, i;
  pid_t pid1, pid2;
  int continua=1;

  if (argc != 3) {
    fprintf(stderr, "Errore numero argomenti.\n");
    return -1;
  }

  n = atoi(argv[1]);
  t = atoi(argv[2]);

  for(i=0; i<n && continua; i++) {
    pid1 = fork();
    if(pid1 > 0) {
      pid2 = fork();
      if (pid2 > 0) {
	  	continua = 0;
	  }
    }
  }
  
  if(i==n && continua) {
    sleep(t);
    fprintf(stdout, "%s %d %s", "Terminazione foglia processo: ", getpid(), "\n");
  }

  return 0;
}
