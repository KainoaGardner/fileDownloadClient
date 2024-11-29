#include "ansi-colors.h"
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

// choose color for each file type
void printListColor(char *fileName) {
  if (strstr(fileName, "txt") != NULL) {
    printf("%s", WHT);
  } else if (strstr(fileName, "jpg") != NULL) {
    printf("%s", GRN);
  } else if (strstr(fileName, "mp3") != NULL) {
    printf("%s", BLU);
  } else if (strstr(fileName, "exe") != NULL) {
    printf("%s", MAG);
  } else {
    printf("%s", RED);
  }
}

void printTime(double time, int fileSize) {
  // print total time
  if (time > 1000) {
    printf("Total time %0.2fs\n", time / 1000);
  } else {
    printf("Total time %0.2fms\n", time);
  }

  // print size per secound
  if (fileSize > 1000000) {
    printf("Download Speed %*.2fMB/s\n", 8,
           (fileSize / 1000000.0) / (time / 1000));
  } else {
    printf("Download Speed %*.2fKB/s\n", 8,
           (fileSize / 1000.0) / (time / 1000));
  }
}

void listFiles(FILE *s) {
  // call list
  fprintf(s, "LIST\n");
  char line[1000];

  // flush OK
  fgets(line, 100, s);

  // format print
  printf("%-20s", "Filename");
  printf("%*s\n", 11, "Size");
  printf("-------------------------------\n");

  while (fgets(line, 100, s) != NULL) {
    // remove newline
    char *nl = strchr(line, '\n');
    if (nl)
      *nl = '\0';

    if (strcmp(line, ".") == 0) {
      break;
    }

    // split string
    char *fileSize = strtok(line, " ");
    char *fileName = strtok(NULL, " ");

    printListColor(fileName);
    printf("%-20s", fileName);
    printFileSize((double)(atoi(fileSize)));
    printf("%s", CRESET);
  }

  printf("\n");
}

void downloadFile(FILE *s, char *fileName) {
  // get start time
  struct timeval now;
  gettimeofday(&now, NULL);
  double sentTime = now.tv_sec + now.tv_usec / 1000000.0;

  // call size
  fprintf(s, "SIZE %s\n", fileName);
  char response[100];
  fgets(response, 100, s);

  // split response string to get size
  char *fSize = strtok(response, " ");
  int fileSize = atoi(strtok(NULL, " "));

  // call get
  fprintf(s, "GET %s\n", fileName);
  fgets(response, 100, s);

  // return if err
  if (strstr(response, "-ERR")) {
    printf("Invalid Filename %s\n", fileName);
    return;
  }

  // create outputFile
  FILE *outputFile = fopen(fileName, "w");
  if (!outputFile) {
    return;
  }

  int bufferSize = 1000;
  char line[bufferSize];
  int transferred = 0;
  double transferredCount = 0;
  int count = 0;

  printf("Downloading: [");

  // transferred 1000 bytes at a time
  while (transferred < fileSize) {
    // needed to print progress bar
    printf("‎");

    int remain = fileSize - transferred;
    int bytes_wanted;
    if (remain < bufferSize) {
      bytes_wanted = remain;
    } else {
      bytes_wanted = bufferSize;
    }

    int bytes_received = fread(line, 1, bytes_wanted, s);

    // progress bar every 10% of filesize
    transferredCount += bytes_received;
    if (transferredCount / fileSize >= 0.10) {
      while (transferredCount / fileSize >= 0.10) {
        printf("⬛");
        transferredCount -= fileSize * 0.1;
      }
      transferredCount = 0;
    }

    // write to outputfile
    fwrite(line, 1, bytes_received, outputFile);

    transferred = transferred + bytes_received;
  }

  printf("]\n");
  printf("Downloaded %s\n", fileName);

  // get end time
  gettimeofday(&now, NULL);
  double receivedTime = now.tv_sec + now.tv_usec / 1000000.0;
  double time = (receivedTime - sentTime) * 1000;
  printTime(time, fileSize);
  printf("\n");

  fclose(outputFile);
}

void downloadSingleFile(FILE *s) {
  char fileName[100];
  printf("Enter File Name to Download: ");
  scanf("%s", fileName);

  // if already have file in directory ask if to replace
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

  // connect to host
  int fd = create_inet_stream_socket(host, "3456", LIBSOCKET_IPv4, 0);

  if (fd < 0) {
    printf("Couldn't connect to server\n");
    exit(1);
  }

  // open server socket
  FILE *s = fdopen(fd, "r+");
  char line[100];
  fgets(line, 100, s);

  // get user option
  int optionInput;
  char flush[10];
  while (optionInput != 2) {
    printf("-----------------------\n");
    printf("0) List Files\n");
    printf("1) Download One File\n");
    printf("2) Quit\n");
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
      printf("Quitting\n");
      break;
    default:
      printf("Invalid Option\n");
      scanf("%s", flush);
    }
  }

  fclose(s);
  close(fd);

  return 0;
}
