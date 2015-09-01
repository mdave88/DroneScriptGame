
#ifndef POLYTEX_H
#define POLYTEX_H

struct poly3
{
	unsigned int a, b, c;

	poly3(unsigned int x = 0, unsigned int y = 0, unsigned int z = 0) : a(x), b(y), c(z)
	{
	}

	unsigned int at(int i) const
	{
		switch (i)
		{
			case 0:
				return a;
			case 1:
				return b;
			case 2:
				return c;
		}
		return 0;
	}
};

struct texCoord
{
	float u;
	float v;

	texCoord(float u = 0.0f, float v = 0.0f) : u(u), v(v) {}
};

#endif
