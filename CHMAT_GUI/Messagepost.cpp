#include <stdio.h>
#include "Messagepost.h"

#define _CRT_SECURE_NO_WARNINGS
void Message::SetWindow(HWND c){Editwin=c;return;}
Message::Message(){messagebuf[0]=0;precision=4;}
void Message::Clear(){
	memset(messagebuf,0,1024);SendMessage (Editwin, EM_SETSEL, (WPARAM)0, 
		(LPARAM)-1);SendMessage (Editwin, EM_REPLACESEL, 0, (LPARAM)  messagebuf);
UpdateWindow(Editwin);}//SetWindowText(Editwin,NULL);}
void Message::AppendText()
{
	int textlen=GetWindowTextLength(Editwin);
	SendMessage (Editwin, EM_SETSEL, (WPARAM)textlen, (LPARAM)textlen);
	SendMessage (Editwin, EM_REPLACESEL, 0, (LPARAM)  messagebuf);
	messagebuf[0]=0;
	return;
}
int Message::Post(double a)
{
	char temp[256];	
	sprintf(temp,"%.3f",a);
	strcat(messagebuf,temp);
	AppendText();
	return(0);
}
int Message::Post(float a)
{
	return(Post((double)a));
}
int Message::Post(int a)
{
	char temp[16];
	itoa(a,temp,10);
	strcat(messagebuf,temp);
	AppendText();
	return(0);
}
int Message::Post(long a)
{	
	return(Post((int)a));
}
int Message::Post(unsigned int a)
{	
	return(Post((int)a));
}
int Message::Post(unsigned long a)
{
	return(Post((int)a));
}
int Message::Post(char *a)
{	
	int k,l,m;
	l=(int)strlen(messagebuf);	
	m=(int)strlen(a);
	for(k=0;k<m;k++)
	{
		if(a[k]==10)
		{
			messagebuf[l]=13;l++;messagebuf[l]=10;l++;
		}
		else if(a[k]==13)continue;
		else
		{
			messagebuf[l]=a[k];l++;
		}
	}
	messagebuf[l]=0;
	AppendText();
	return(0);
}
int Message::Post(char c)
{	
	int k,l,m;
	l=(int)strlen(messagebuf);	
	messagebuf[l]=c;
	messagebuf[l+1]=0;
	AppendText();
	return(0);
}

int Message::Post(char *a,int i)
{	
	/*int j,k,l,m,n;
	char intcontainer[16];
	itoa(i,intcontainer,10);
	n=strlen(intcontainer);
	l=(int)strlen(messagebuf);	
	m=(int)strlen(a);
	for(k=0;k<m;k++)
	{
		if(a[k]==10)
		{
			messagebuf[l]=13;l++;messagebuf[l]=10;l++;
		}
		else if(a[k]==13)continue;
		else if(a[k]=='%' && a[k+1]=='d')
		{
			k++;
			for(j=0;j<n;j++)
			{
				messagebuf[l]=intcontainer[j];
				l++;
			}
		}
		else
		{
			messagebuf[l]=a[k];l++;
		}
	}
	messagebuf[l]=0;*/
	sprintf(messagebuf,a,i);
	AppendText();
	return(0);
}

int Message::Post(char *a,int i,int e)
{	
	int j,k,l,m,n,count=0;
	char intcontainer[16];	
	l=(int)strlen(messagebuf);	
	m=(int)strlen(a);
	for(k=0;k<m;k++)
	{
		if(a[k]==10)
		{
			messagebuf[l]=13;l++;messagebuf[l]=10;l++;
		}
		else if(a[k]==13)continue;
		else if(a[k]=='%' && a[k+1]=='d')
		{
			k++;
			if(count==0)
			{
				itoa(i,intcontainer,10);
				count++;
			}
			else if(count==1)
			{
				itoa(e,intcontainer,10);
				count++;
			}
			n=strlen(intcontainer);
			for(j=0;j<n;j++)
			{
				messagebuf[l]=intcontainer[j];
				l++;
			}
		}
		else
		{
			messagebuf[l]=a[k];l++;
		}
	}
	messagebuf[l]=0;
	AppendText();	
	return(0);
}
int Message::Post(char *a,unsigned int i)
{
	return(Post(a,(int)i));	
}
int Message::Post(char *a,unsigned int i,unsigned int e)
{
	return(Post(a,(int)i,(int)e));
}
int Message::Post(char *a,unsigned long i)
{
	return(Post(a,(int)i));	
}
int Message::Post(char *a,unsigned long i,unsigned long e)
{
	return(Post(a,(int)i,(int)e));
}
int Message::Post(char *a,double f)
{	
	int j,k,l,m,n,s;
	char fcontainer[16];
	
	l=(int)strlen(messagebuf);	
	m=(int)strlen(a);
	for(k=0;k<m;k++)
	{
		if(a[k]==10)
		{
			messagebuf[l]=13;l++;messagebuf[l]=10;l++;
		}
		else if(a[k]==13)continue;
		else if(a[k]=='%' && a[k+1]=='f')
		{
			k++;
			_gcvt(f,precision,fcontainer);
			n=strlen(fcontainer);
			for(j=0;j<n;j++)
			{
				messagebuf[l]=fcontainer[j];
				l++;
			}
		}
		else
		{
			messagebuf[l]=a[k];l++;
		}
	}
	messagebuf[l]=0;
	AppendText();	
	return(0);
}
int Message::Post(char *a,double f,int i)
{	
	int j,k,l,m,n,s;
	char fcontainer[16];
	char intcontainer[16];
	_itoa_s(i,intcontainer,16,10);	
	l=(int)strlen(messagebuf);	
	m=(int)strlen(a);
	for(k=0;k<m;k++)
	{
		if(a[k]==10)
		{
			messagebuf[l]=13;l++;messagebuf[l]=10;l++;
		}
		else if(a[k]==13)continue;
		else if(a[k]=='%' && a[k+1]=='f')
		{
			k++;
			_gcvt(f,precision,fcontainer);
			n=strlen(fcontainer);
			for(j=0;j<n;j++)
			{
				messagebuf[l]=fcontainer[j];
				l++;
			}
		}
		else if(a[k]=='%' && a[k+1]=='d')
		{
			k++;
			n=strlen(intcontainer);	
			for(j=0;j<n;j++)
			{
				messagebuf[l]=intcontainer[j];
				l++;
			}
		}
		else
		{
			messagebuf[l]=a[k];l++;
		}
	}
	messagebuf[l]=0;
	AppendText();
	return(0);
}
int Message::Post(char *a,char *b)
{	
	int k,l,m;
	l=(int)strlen(messagebuf);
	m=(int)strlen(a);
	for(k=0;k<m;k++)
	{
		if(a[k]==10)
		{
			messagebuf[l]=13;
			l++;
			messagebuf[l]=10;
			l++;
			//messagebuf[l+k+n]=0;
		}
		else if(a[k]==13)continue;
		else
		{
			messagebuf[l]=a[k];
			l++;
		}
	}
	m=(int)strlen(b);
	for(k=0;k<m;k++)
	{
		if(b[k]==10)
		{
			messagebuf[l]=13;
			l++;
			messagebuf[l]=10;
			l++;
		}
		else if(b[k]==13)continue;
		else
		{
			messagebuf[l]=b[k];
			l++;
		}
	}
	messagebuf[l]=0;
	AppendText();	
	return(0);
}

int Message::Post(char *a,char *b,char *c)
{	
	int k,l,m;
	l=(int)strlen(messagebuf);
	m=(int)strlen(a);
	for(k=0;k<m;k++)
	{
		if(a[k]==10)
		{
			messagebuf[l]=13;l++;messagebuf[l]=10;l++;
		}
		else if(a[k]==13)continue;
		else
		{
			messagebuf[l]=a[k];l++;
		}
	}
	m=(int)strlen(b);
	for(k=0;k<m;k++)
	{
		if(b[k]==10)
		{
			messagebuf[l]=13;l++;messagebuf[l]=10;l++;
		}
		else if(b[k]==13)continue;
		else
		{
			messagebuf[l]=b[k];l++;
		}
	}
	m=(int)strlen(c);
	for(k=0;k<m;k++)
	{
		if(c[k]==10)
		{
			messagebuf[l]=13;l++;messagebuf[l]=10;l++;
		}
		else if(b[k]==13)continue;
		else
		{
			messagebuf[l]=c[k];l++;
		}
	}
	messagebuf[l]=0;
	AppendText();	
	return(0);
}