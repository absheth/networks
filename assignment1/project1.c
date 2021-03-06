/*
Name: Akash Sheth

*/
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
// --------------------------------------------------------------------------------

// Constants
#define DOT_COUNT 3
#define RETURN_FAIL -1
#define RETURN_SUCCESS 0
#define ERROR_MESSAGE "ERROR"
#define UNSIGNED_INTEGER_RANGE 4294967295
const char *error_message = ERROR_MESSAGE;
// --------------------------------------------------------------------------------

// Method declarations
int validate_input(int conversion_type, char str[], int endian_type);
void convertIPToDecimal(char str[], int endian_type);
void convertDecimalToIP(char str[], int endian_type);
long binary_to_decimal(int *p_binary_string, int p_length);
void *decimal_to_binary(unsigned int p_number, int *p_binary_string, int p_index);

// --------------------------------------------------------------------------------

int main(int argc, char const *argv[]) {
  /* Inputs */
  int conversion_type;
  char conversion_str[100];
  int endian_type;
  if (argc > 4) {
    printf("%s", error_message);
    return 0;
  }
  conversion_type = atoi(argv[1]);
  strcpy(conversion_str, argv[2]);
  endian_type = atoi(argv[3]);
  int check_return = validate_input(conversion_type, conversion_str, endian_type);
  // printf("Return from validate_input --> %d\n", check_return);
  if (check_return != RETURN_SUCCESS) {
    printf("%s", error_message);
  } else {
    if (conversion_type == 0) {
      convertIPToDecimal(conversion_str, endian_type);
    } else {
      convertDecimalToIP(conversion_str, endian_type);
    }
  }


  // printf("\n");
  return 0;
}
// --------------------------------------------------------------------------------

/* This method validates the input from command line. */
int validate_input(int p_conversion_type, char p_str[], int p_endian_type) {

  // Validating endian type
  if (p_endian_type != 1 && p_endian_type != 0) {
    return RETURN_FAIL;
  }

  // Validating the IP address or
  // the internal decimal format based on the conversion type.
  if (p_conversion_type == 0) {
    struct sockaddr_in sa;
    int x = inet_pton(AF_INET, p_str, &(sa.sin_addr));
    if (x != 1) {
      return RETURN_FAIL;
    }

  } else if (p_conversion_type == 1) {

    char *ret = strchr(p_str, '.');
    if (ret != NULL) {
      return RETURN_FAIL;
    }

    char *ret1 = strchr(p_str, '-');
    if (ret1 != NULL) {
      return RETURN_FAIL;
    }
    unsigned long long int decimal_number= 0;
    int i = 0;
    for(i = 0; i < strlen(p_str); i++) {
      decimal_number += (p_str[i]-'0')*pow(10,strlen(p_str)-i-1);
    }
    if (decimal_number > UNSIGNED_INTEGER_RANGE) {
      return RETURN_FAIL;
    }
    
  } else {
    return RETURN_FAIL; // Input error;
  }
  return RETURN_SUCCESS;
}
// --------------------------------------------------------------------------------

void convertIPToDecimal(char p_str[], int p_endian_type) {
  int binary_string[32];

  // Pre-populate with zeros
  int length = sizeof(binary_string)/sizeof(binary_string[0]);
  int i = 0;
  for (i = 0; i < length; i++) {
    binary_string[i] = 0;
  }
  /*printf("\n");

  printf("str         --> %s\n", p_str);
  printf("endian type --> ");
  p_endian_type == 1 ? printf("Big Endian --> ") : printf("Little Endian --> ");
  printf("%d \n", p_endian_type);
  printf("-------------------------\n");
  printf("\n");*/
  int l_count = 2;
  int index = 0;

  if (p_endian_type == 0) {
    char tokens[4] = "000";
    int x = 0;
    // printf("Length-> %lu\n", strlen(p_str) );
    for (x = strlen(p_str)-1; x>=0; x--) {
      // printf("char --> %c\n", p_str[x]);
      if (p_str[x] == '.') {
        decimal_to_binary(atoi(tokens), binary_string, index);
        l_count = 2;
        strcpy(tokens, "000");
        index = index + 8;
      } else {
        tokens[l_count--] = p_str[x];
        // printf("%s\n", tokens);
      }

      if (x == 0) {
        // tokens[l_count] = p_str[i]; // THINK
        decimal_to_binary(atoi(tokens), binary_string, index);
        break;
      }

    }
    printf("%lu", binary_to_decimal(binary_string, length));
  } else {
    //Spliting
    char *tokens = strtok(p_str, ".");
    while (tokens != NULL)
    {
      decimal_to_binary(atoi(tokens), binary_string, index);
      tokens = strtok(NULL, ".");
      index = index + 8;
    }
    printf("%lu", binary_to_decimal(binary_string, length));

  }
  //int j = 0;
  // printf("Binary--> ");
  /*or (j = 0; j < length; j++) {
    printf("%d", binary_string[j]);
  }*/
  //printf("\n-------------------------\n");
}
// --------------------------------------------------------------------------------

void convertDecimalToIP(char p_str[], int p_endian_type) {
  int binary_string[32];
  char ip_string[16];
  ip_string[0]='\0';
  // Pre-populate with zeros
  int length = sizeof(binary_string)/sizeof(binary_string[0]);
  int j = 0;
  for (j = 0; j < length; j++) {
    binary_string[j] = 0;
  }
  // printf("%d\n", length);
  // printf("\n");

  // printf("str         --> %s\n", p_str);
  // printf("endian type --> ");
  // p_endian_type == 1 ? printf("Big Endian --> ") : printf("Little Endian --> ");
  // printf("%d \n", p_endian_type);
  // printf("-------------------------\n");
  // printf("\n");

  unsigned int decimal_number = atoi(p_str);
  int l_count = 0;
  while (decimal_number != 0) {
    binary_string[length - l_count - 1] = decimal_number%2;
    decimal_number = decimal_number/2;
    l_count++;
  }

  if (p_endian_type == 0) {
    l_count = length - 1;
    int sum = 0;
    int pointer = 0;
    while (l_count>-1) {
      char temp[4];
      if (l_count%8 == 0) {
        sum = sum + binary_string[l_count]*pow(2, pointer);
        // printf("Sum --> %d\n", sum);
        // printf("pointer --> %d\n", pointer);

        sprintf(temp, "%d", sum);
        strcat(ip_string, temp);
        if (l_count!=0) {
          strcat(ip_string, ".");
        }
        pointer = 0;
        sum = 0;
      } else {
        // printf("pointer --> %d\n", pointer);
        sum = sum + binary_string[l_count]*pow(2, pointer);
        pointer++;
      }
      l_count--;
    }

  } else {
    l_count = 0;
    int sum = 0;
    int index = 0;
    int pointer = 0;
    int traverse_pointer = 0;
    while (l_count < length+1 ) {
      if(l_count % 8 == 0 && l_count!=0) {
        pointer = l_count;
        while(pointer!=index) {
          pointer--;
          sum = sum + binary_string[pointer]*pow(2, traverse_pointer);
          ++traverse_pointer;
        }
        char temp[4];
        if (sum!=0) {
          sprintf(temp, "%d", sum);
          strcat(ip_string, temp);
          if (l_count!=length) {
            strcat(ip_string, ".");
          }
        } else {
          strcat(ip_string, "0");
          if (l_count!=length) {
            strcat(ip_string, ".");
          }
        }
        sum = 0;
        index = l_count;
        traverse_pointer = 0;
        l_count++;
        continue;
      }
      l_count++;
    }
  }
  // printf("-------------------------\n");
  //printf("Binary--> ");
  /*int k = 0;
  for (k = 0; k < length; k++) {
    printf("%d", binary_string[k]);
  }*/
  //printf("\n");
  printf("%s",ip_string);
}
// --------------------------------------------------------------------------------

void *decimal_to_binary(unsigned int p_number, int *p_binary_string, int p_index) {

  int l_remainder = 0;
  int l_index = p_index + 7;
  if (p_number != 0) {
    while (p_number != 0) {
      l_remainder = p_number%2;
      p_binary_string[l_index] = l_remainder;
      p_number = p_number/2;
      l_index--;
    }
  }
  return 0;
}
// --------------------------------------------------------------------------------

long binary_to_decimal(int *p_binary_string, int p_length) {
  long sum = 0;
  int i = 0;
  for (i = 0; i < p_length; i++) {
    sum = sum + p_binary_string[p_length-i-1]*pow(2, i);
  }
  //printf("\n");
  return sum;
}
