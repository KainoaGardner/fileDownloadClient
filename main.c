#include "ansi-colors.h"
#include <ctype.h>
#include <libsocket/libinetsocket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

void printFileSize(double bytes) {
  if (bytes > 1000000) {
    printf("%*.2f MB\n", 8, bytes / 1000000);
  } else if (bytes > 1000) {
    printf("%*.2f KB\n", 8, bytes / 1000);
  } else {
    printf("%*.2f B\n", 9, bytes);
  }
}

void printListColor(char *fileName) {
  if (strstr(fileName, "txt") != NULL) {
    printf("%s", WHT);
  } else if (strstr(fileName, "jpg") != NULL) {
    printf("%s", GRN);
  } else if (strstr(fileName, "mp3") != NULL) {
    printf("%s", BLU);
  } else if (strstr(fileName, "exe") != NULL) {
    printf("%s", YEL);
  }

  else {
    printf("%s", RED);
  }
}

void listFiles(FILE *s) {
  /* fprintf(s, "LIST\n"); */
  fprintf(s, "LIST\n");
  char line[1000];

  // flush OK
  fgets(line, 100, s);

  printf("%-20s", "Filename");
  printf("%*s\n", 11, "Size");
  printf("-------------------------------\n");

  while (fgets(line, 100, s) != NULL) {
    char *nl = strchr(line, '\n');
    if (nl)
      *nl = '\0';

    if (strcmp(line, ".") == 0) {
      break;
    }

    char *fileSize = strtok(line, " ");
    char *fileName = strtok(NULL, " ");

    printListColor(fileName);
    printf("%-20s", fileName);
    printFileSize((double)(atoi(fileSize)));
    printf("%s", CRESET);

    /* printf("%s\n", fileSize); */

    /* printf("%s\n", line); */

    // get array of password variations
  }

  printf("\n");
}

void downloadFile(FILE *s, char *fileName) {
  fprintf(s, "SIZE %s\n", fileName);
  char response[100];

  fgets(response, 100, s);
  char *fSize = strtok(response, " ");
  int fileSize = atoi(strtok(NULL, " "));

  fprintf(s, "GET %s\n", fileName);
  fgets(response, 100, s);
  if (strstr(response, "-ERR")) {
    printf("Invalid Filename %s\n", fileName);
    return;
  }

  FILE *outputFile = fopen(fileName, "w");
  if (!outputFile) {
    return;
  }

  int bufferSize = 1000;
  char line[bufferSize];
  int transferred = 0;

  while (transferred < fileSize) {
    int remain = fileSize - transferred;
    int bytes_wanted;
    if (remain < bufferSize) {
      bytes_wanted = remain;
    } else {
      bytes_wanted = bufferSize;
    }

    int bytes_received = fread(line, 1, bytes_wanted, s);
    fwrite(line, 1, bytes_received, outputFile);
    transferred = transferred + bytes_received;
  }

  printf("Downloaded %s\n", fileName);

  fclose(outputFile);
}

void downloadSingleFile(FILE *s) {
  char fileName[100];
  printf("Enter File Name to Download: ");
  scanf("%s", fileName);

  FILE *checkFile = fopen(fileName, "r");
  if (checkFile) {
    char overwrite[10];
    printf("Do you want to ovewrite %s (Y/N)?: ", fileName);
    scanf("%s", overwrite);

    fclose(checkFile);
    if (strcmp(overwrite, "Y") != 0) {
      return;
    }
  }

  downloadFile(s, fileName);
}

void downloadAllFiles(FILE *s) {
  fprintf(s, "LIST\n");
  char line[1000];

  // flush OK
  fgets(line, 100, s);

  while (fgets(line, 100, s) != NULL) {
    char *nl = strchr(line, '\n');
    if (nl)
      *nl = '\0';

    if (strcmp(line, ".") == 0) {
      break;
    }

    char *fileSize = strtok(line, " ");
    char *fileName = strtok(NULL, " ");
  }
}

int main(int argc, char *argv[]) {
  char host[100];

  int validHost = 0;
  while (validHost == 0) {

    printf("Which server (newark/london): ");
    scanf("%s", host);

    if (strcmp(host, "newark") != 0 && strcmp(host, "london") != 0) {
      printf("Invalid Server: %s\n", host);
    } else {
      validHost = 1;
    }
  }

  strcat(host, ".cs.sierracollege.edu");
  int fd = create_inet_stream_socket(host, "3456", LIBSOCKET_IPv4, 0);

  if (fd < 0) {
    printf("Couldn't connect to server\n");
    exit(1);
  }

  FILE *s = fdopen(fd, "r+");
  char line[100];
  fgets(line, 100, s);

  int optionInput;
  while (optionInput != 3) {
    printf("-----------------------\n");
    printf("0) List Files\n");
    printf("1) Download One File\n");
    printf("2) Download All Files\n");
    printf("3) Quit\n");
    printf("What would you like to do: ");
    scanf("%d", &optionInput);
    printf("-----------------------\n");
    printf("\n");

    switch (optionInput) {
    case 0:
      listFiles(s);
      break;
    case 1:
      downloadSingleFile(s);
      break;
    case 2:
      downloadAllFiles(s);
      break;
    case 3:
      printf("Quitting\n");
      break;
    default:
      printf("Invalid Option %d", optionInput);
    }
  }

  fclose(s);
  close(fd);

  return 0;
}
