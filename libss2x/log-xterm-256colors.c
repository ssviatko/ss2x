#include <stdio.h>

int main(int argc, char **argv)
{
	int i, j, color;
	for (i = 0; i < 8; ++i) {
		for (j = 0; j < 32; ++j) {
			color = ((i * 32) + j);
			printf("\033[38;5;%dm(%03d)", color, color);
		}
		printf("\n");
	}
	printf("\n");
	for (i = 0; i < 8; ++i) {
		for (j = 0; j < 32; ++j) {
			color = ((i * 32) + j);
			printf("\033[48;5;%dm\033[38;5;0m(%03d)", color, color);
		}
		printf("\033[0m\n");
	}
	printf("\n");
	for (i = 0; i < 8; ++i) {
		for (j = 0; j < 32; ++j) {
			color = ((i * 32) + j);
			printf("\033[48;5;%dm\033[38;5;15m(%03d)", color, color);
		}
		printf("\033[0m\n");
	}
	printf("\n");
	return 0;
}

