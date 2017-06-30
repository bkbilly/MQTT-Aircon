#include <stdio.h>
#include <string.h>

void binaryAddLimit(char* out, char first[], char plus[], int binsize) {
  char result[binsize+1];

  int carrier = 0;
  int i;
  for (i = 0; i < binsize; ++i) {
    if (first[i] == '0' && plus[i] == '0' && carrier == 0) {
        result[i] = '0';
        carrier = 0;
    } else if (first[i] == '1' && plus[i] == '0' && carrier == 0) {
        result[i] = '1';
        carrier = 0;
    } else if (first[i] == '0' && plus[i] == '1' && carrier == 0) {
        result[i] = '1';
        carrier = 0;
    } else if (first[i] == '1' && plus[i] == '1' && carrier == 0) {
        result[i] = '0';
        carrier = 1;
    } else if (first[i] == '0' && plus[i] == '0' && carrier == 1) {
        result[i] = '1';
        carrier = 0;
    } else if (first[i] == '1' && plus[i] == '0' && carrier == 1) {
        result[i] = '0';
        carrier = 1;
    } else if (first[i] == '0' && plus[i] == '1' && carrier == 1) {
        result[i] = '0';
        carrier = 1;
    } else if (first[i] == '1' && plus[i] == '1' && carrier == 1) {
        result[i] = '1';
        carrier = 1;
    }
  }
  result[binsize] = '\0';
  // i = 0;
  // char tmp;
  // while (carrier == 1 && i < binsize) {
  //     if (result[i] == '0') {
  //         tmp = '1';
  //         carrier = 0;
  //     }
  //     else if (result[i] == '1') {
  //         tmp = '0';
  //     }
  //     result[i] = tmp;
  //     i += 1;
  // }
  strcpy(out, result);
}

int main()
{
	char result[10];
	char bintemp[10] = "1111";
	char plus[10] = "1110";
	binaryAddLimit(result, bintemp, plus, 4);
	printf("%s\n", result);

	float f = 18.123456789;
	char c[50]; //size of the number
	sprintf(c, "%d.%d", (int)f, (int)(f*1000)%1000);
	printf("%s", c);
	printf("\n");
}
