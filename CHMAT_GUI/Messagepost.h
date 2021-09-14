#ifndef MESSAGEPOST_H
#define MESSAGEPOST_H
#include <windows.h>

class Message
{
	char messagebuf[4096];
	int precision;
	HWND Editwin;
public:
	virtual ~Message(){}
	Message();
	void SetWindow(HWND c);
	void AppendText();
	int Post(double a);
	int Post(float a);
	int Post(int a);
	int Post(long a);
	int Post(unsigned int a);
	int Post(unsigned long a);
	int Post(char *a);
	int Post(char a);
	int Post(char *a,int i);
	int Post(char *a,int i,int e);
	int Post(char *a,unsigned int i);
	int Post(char *a,unsigned int i,unsigned int e);
	int Post(char *a,unsigned long i);
	int Post(char *a,unsigned long i,unsigned long e);
	int Post(char *a,double f);
	int Post(char *a,double f,int i);
	int Post(char *a,char *b);
	int Post(char *a,char *b,char *c);
	void Clear();
};

#endif