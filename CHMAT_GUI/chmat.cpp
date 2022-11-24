#include <stdlib.h>
#include <stdio.h>
#include <windows.h>
#include "chmat.h"

int FillBMPInfo(BITMAPINFO* info, int x, int y, int bits)
{
	info->bmiHeader.biBitCount = bits;
	info->bmiHeader.biClrImportant = 0;
	info->bmiHeader.biClrUsed = 0;
	info->bmiHeader.biCompression = BI_RGB;
	info->bmiHeader.biHeight = y;
	info->bmiHeader.biPlanes = 1;
	info->bmiHeader.biSize = sizeof(BITMAPINFO);
	info->bmiHeader.biSizeImage = x * y * bits / 8;
	info->bmiHeader.biWidth = x;
	info->bmiHeader.biXPelsPerMeter = 0;
	info->bmiHeader.biYPelsPerMeter = 0;
	return(0);
}

CHMAT::CHMAT(int x, int y, int typ)
{
	if (typ == CHMAT_UNSIGNED_CHAR)
		data = calloc(x * y, sizeof(char));
	else if (typ == CHMAT_INT)
	{
		BITMAPINFO binfo;
		FillBMPInfo(&binfo, x, y, 32);
		HBitmap = CreateDIBSection(NULL, &binfo, DIB_RGB_COLORS, (void**)&data, NULL, NULL);
	}
	if (data != NULL)
	{
		X = x;
		Y = y;
		type = typ;
	}
}

CHMAT::~CHMAT()
{
	if (type == CHMAT_INT)
		DeleteObject(HBitmap);
	else free(data);
}

int CHMAT::set(int x, int y, int val)
{
	if (x<1 || x>X) return (-1);
	if (type == CHMAT_UNSIGNED_CHAR)
		((unsigned char*)data)[(y - 1) * X + (x - 1)] = val;
	else if (type == CHMAT_INT)
		((int*)data)[(y - 1) * X + (x - 1)] = val;
	return 0;
}

void CHMAT::print_mat()
{
	int x, y;
	unsigned char* ucd = (unsigned char*)data;
	int* id = (int*)data;
	for (y = 0; y < Y; y++)
	{
		for (x = 0; x < X; x++)
		{
			if (type == CHMAT_UNSIGNED_CHAR)
				printf("%d ", ucd[y * X + x]);
			else if (type == CHMAT_INT)
				printf("%d ", id[y * X + x]);
		}
		printf("\n");
	}
	printf("\n");
}

void CHMAT::operator = (int a)
{
	int x, y;
	unsigned char* ucd = (unsigned char*)data;
	int* id = (int*)data;
	for (y = 0; y < Y; y++)
	{
		for (x = 0; x < X; x++)
		{
			if (type == CHMAT_UNSIGNED_CHAR)
				ucd[y * X + x] = a;
			else if (type == CHMAT_INT)
				id[y * X + x] = a;
		}
	}
}


//Template????
void CHMAT::operator += (int a)
{
	int x, y;
	unsigned char* ucd = (unsigned char*)data;
	int* id = (int*)data;
	for (y = 0; y < Y; y++)
	{
		for (x = 0; x < X; x++)
		{
			if (type == CHMAT_UNSIGNED_CHAR)
				ucd[y * X + x] += a;
			else if (type == CHMAT_INT)
				id[y * X + x] += a;
		}
	}
}

int& CHMAT::operator () (int x, int y)
{
	static int r = 0;
	if (x > X || y > Y); //???????????????????
	int* t = (int*)data;
	return t[X * (y - 1) + (x - 1)];
}