#include <libsocket/libinetsocket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  char serverHost[100];

  int validHost = 0;
  while (validHost == 0) {

    printf("Which server (newark/london): ");
    scanf("%s", serverHost);

    if (strcmp(serverHost, "newark") != 0 &&
        strcmp(serverHost, "london") != 0) {
      printf("Invalid Server: %s\n", serverHost);
    } else {
      validHost = 1;
    }
  }

  strcat(serverHost, ".cs.sierracollege.edu");

  return 0;
}
