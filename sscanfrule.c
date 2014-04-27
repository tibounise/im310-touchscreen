#include <stdio.h>

int main() {
	int x,y;

	char input[] = "T1024,1000\x0A";
	sscanf(input,"T%4d,%4d\x0A",&x,&y);
	printf("X : %d", x);
	printf("\nY : %d", y);
}