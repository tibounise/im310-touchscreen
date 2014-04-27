#include <stdio.h>

int main() {
	int x,y;
	char is_touching;

	char input[] = "T1024,1000\x0A";
	sscanf(input,"%c%4d,%4d\x0A",&is_touching,&x,&y);
	printf("X : %d\n", x);
	printf("Y : %d\n", y);
	printf("Is touching : %c\n",is_touching);
}