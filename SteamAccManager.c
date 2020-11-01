#include <stdio.h>
#include <stdlib.h>

// ARCHIVED VERSION

struct account_data {
  char user[20][40];
  char pass[20][40];
};

const char file[] = "acc.dat";

struct account_data return_data();
int options_func(struct account_data accounts);
void saveaccs(struct account_data accounts);
int invalidCharacter(int error);

void main() {
  struct account_data accounts = return_data();

  if (strcmp(accounts.pass[0], "empty") == 0) {
    printf("Running first time setup...\nEnter account username: ");
    scanf("%s", &accounts.user[1]);

    printf("Enter account password: ");
    scanf("%s", &accounts.pass[1]);

    saveaccs(accounts);
    printf("Account added.\n\n");
  }

  int i = 0;

start:

  // Prints options and returns number of accounts in i
  i = options_func(accounts);

  int inputval;

  int error = scanf("%d", &inputval);
  if (invalidCharacter(error)) {
    goto start;
  }

  // OPTIONS FUNCTION
  if (inputval > i) {
    printf("Invalid selection :( \n\n");
    goto start;
  }

  else if (inputval == 0) {
    int options = 2;
    printf("\nInput 0 to add new account, 1 to delete existing account: ");

    int error = scanf("%d", &options);
    if (invalidCharacter(error)) {
      goto start;
    } else if (options > 1) {
      printf("Invalid selection :( \n\n");
      goto start;
    }

    if (options == 1) {
      int k = 0;

      printf("Enter account number to remove: ");

      int error = scanf("%d", &k);
      if (invalidCharacter(error)) {
        goto start;
      }

      if (k == 0 || k > i) {
        printf("Invalid selection :( \n\n");
        goto start;
      }

      for (k; k < 19; k++) {
        strcpy(accounts.user[k], accounts.user[k + 1]);
        strcpy(accounts.pass[k], accounts.pass[k + 1]);
      }

      saveaccs(accounts);
      printf("Account removed.\n\n");

      goto start;
    }

    else if (options == 0) {
      printf("Enter account username: ");
      scanf("%s", &accounts.user[i][40]);

      printf("Enter account password: ");
      scanf("%s", &accounts.pass[i][40]);

      saveaccs(accounts);
      printf("Login added.\n\n");

      goto start;
    }

    printf("Invalid selection :( \n\n");
    goto start;
  }

  else {
    system("taskkill /F /IM steam.exe");

    char command[120] = "\"C:\\Program Files (x86)\\Steam\\Steam.exe\" -login ";
    strcat(command, accounts.user[inputval]);
    strcat(command, " ");
    strcat(command, accounts.pass[inputval]);

    printf("\nOpening account: %s", accounts.user[inputval]);
    system(command);
  }

  exit(0);
}

struct account_data return_data() {
  struct account_data accounts = {.user = {0}, .pass = {0}};

  char *acc_str = NULL;
  FILE *acc_file = fopen(file, "r");

  // If file doesn't exist
  if (acc_file == NULL) {
    acc_file = fopen(file, "w");
  }

  // Attains length of file stream
  fseek(acc_file, 0, SEEK_END);
  int length = ftell(acc_file);
  fseek(acc_file, 0, SEEK_SET);

  // If file is empty
  if (length == 0) {
    strcpy(accounts.pass[0], "empty");
    return accounts;
  }

  acc_str = malloc((length + 1) * sizeof(char));

  // Reads file stream into string pointer
  int offset = 0;

  while (!feof(acc_file) && offset < length) {
    offset += fread(acc_str + offset, sizeof(char), length - offset, acc_file);
  }

  acc_str[offset] = '\0';

  fclose(acc_file);

  // Loads accounts from acc_str by strtok
  int i = 1;
  char *account_str = strtok(acc_str, "\n");
  while (account_str != NULL) {
    sscanf(account_str, "%s %s\n", &accounts.user[i], &accounts.pass[i]);
    account_str = strtok(NULL, "\n");
    i++;
  }

  return accounts;
}

int options_func(struct account_data accounts) {
  printf("Select account to open: \n");

  int j = 1;
  do {
    printf("%d: %s\n", j, accounts.user[j]);
    j++;
  } while (accounts.user[j][0] != '\0');

  printf("Input 0 for more options: ");

  return j - 1;
}

void saveaccs(struct account_data accounts) {
  int j = 1;
  FILE *acc_file = fopen(file, "w");
  do {
    fprintf(acc_file, "%s %s\n", accounts.user[j], accounts.pass[j]);
    j++;
  } while (accounts.user[j][0] != '\0');
  fflush(acc_file);
}

int invalidCharacter(int error) {
  if (error < 1) {
    char dummy[] = "onerroraddtobufferstring";
    scanf("%s", &dummy);  // eats a few characters off the buffer
    printf("Invalid selection... \n\n");
    return 1;
  }
  return 0;
}