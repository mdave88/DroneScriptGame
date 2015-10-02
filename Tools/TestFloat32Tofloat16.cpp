#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

// float compression/decompression : http://cboard.cprogramming.com/c-programming/92250-storing-float-16-bits.html
int csh = 4;		//8;
int cand = 15;		//255;
float cdiv = 16;	//256.0f;

short float32Tofloat16(float num)
{
	short i, f;

	if(fabs(num) > 2047.999f)
	{
		printf("Error: number out of range (num=%f)\n", num);
	}

	i = (short)num;
	f = (short)(fabs(num * cdiv)) & cand;

	return (i << csh) | f;
}

float float16Tofloat32(int n)
{
	float s = 1.0f;
	if(n < 0)
	{
		s = -1.0f;
		n = -n;
	}

	return s * ((float)(n >> csh) + ((n & cand) / cdiv));
}

int sign() {
	return (rand() % 2 == 0) ? 1 : -1;
}

float MAX(const float a, const float b) {
	return (a < b) ? b : a;
}

int main(int argc, char **argv) {
	float f, g, h;
	short a, b, c;
	float as, bs, cs;

	float sumError = 0, sumMaxError = 0;
	float maxError = 0;
	int numCrossers = 0;

	srand(time(nullptr));

	int size = 1000000;

	for(int i = 0; i < size; i++) {
		f = sign() * (rand() % 1000) / (float)(rand() % 10 + 1);
		g = sign() * (rand() % 1000) / (float)(rand() % 10 + 1);
		h = sign() * (rand() % 1000) / (float)(rand() % 10 + 1);

		a = float32Tofloat16(f);
		b = float32Tofloat16(g);
		c = float32Tofloat16(h);

		//printf("%04x, %04x, %04x\n", a, b, c);
		//printf("fgh: %4.4f, %4.4f, %4.4f\n", f, g, h);
		//printf("abc: %4.4f, %4.4f, %4.4f\n", float16Tofloat32(a), float16Tofloat32(b), float16Tofloat32(c));

		as = float16Tofloat32(a);
		bs = float16Tofloat32(b);
		cs = float16Tofloat32(c);

		float errorMargin = 1.5f;

		if(fabs(as - f) > errorMargin ||
			fabs(bs - g) > errorMargin ||
			fabs(cs - h) > errorMargin
			)
		{
			using namespace std;
			float currentMax = MAX(fabs(as - f), MAX(fabs(bs - g), fabs(cs - h)));
			maxError = MAX(maxError, currentMax);
			sumMaxError += currentMax;

			//printf("err: %4.4f, %4.4f, %4.4f\n", fabs(as - f), fabs(bs - g), fabs(cs - h));
			//printf("ATTENTION %4.4f\n", currentMax);

			numCrossers++;
		}

		sumError += fabs(as - f) + fabs(bs - g) + fabs(cs - h);

		//printf("\n");
	}

	printf("numCrossers: %d/%d -> %4.1f\n", numCrossers, size, (float)numCrossers / size * 100);
	printf("sumError:    %4.2f\n", sumError);
	printf("sumMaxError: %4.2f\n", sumMaxError);
	printf("avgMaxError: %4.2f\n", sumMaxError / numCrossers);
	printf("avgError:    %4.4f\n", sumError / (size * 3));
	printf("maxError:    %4.4f\n", maxError);

	return 0;
}
