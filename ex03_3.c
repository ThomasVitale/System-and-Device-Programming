#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char* argv[]) {
  int n, *vett, i;
  pid_t pid;
  int ret;
  if (argc != 2) {
    fprintf(stderr, "Errore numero argomenti.\n");
    return -1;
  }

  n = atoi(argv[1]);

  vett = (int*)malloc(n*sizeof(int));

  for (i=0; i<n; i++) {
    fprintf(stdout, "Inserire un intero:\n");
    fscanf(stdin, "%d", &vett[i]);
  }

  for(i=0; i<n-1; i++) {
    pid = fork();
    if(pid > 0) {
      wait(&ret);
      break;
    }
  }
  
  if (pid != 0)
  fprintf(stdout, "%d%s", vett[i], "\n");
 
  return 0;
}
