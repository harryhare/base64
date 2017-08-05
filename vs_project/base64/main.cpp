#include <iostream>
#include <fstream>
#include <exception>
#include <cstring>
#include <cstdio>
#include <cassert>
using namespace std;


char FILE_SUFFIX_CODE[]=".code";
char FILE_SUFFIX_DECODE[]=".decode";
char ERROR_HINT_INPUT_ILLEGAL[]="input file illegal";
char ERROR_HINT_CHAR_ILLEGAL[]="input file contains illegal character";
char ERROR_HINT_FORMAT_ILLEGAL[]="input file format illegal";
const int READ_LENTH_ONCE=57;//57=76/4*3,this must equal to n*57
const int MAX_LEN=1024;
unsigned char buffer[MAX_LEN];
unsigned char mid[MAX_LEN*2];
unsigned char result[MAX_LEN*2];
char dic[256]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";

class IllegalInput:public exception
{
public:
	virtual const char* what()const throw()
	{
		return ERROR_HINT_INPUT_ILLEGAL;
	}
};
class IllegalChar:public IllegalInput
{
public:
	virtual const char* what()const throw()
	{
		return ERROR_HINT_CHAR_ILLEGAL;
	}
};
IllegalChar illegal_char;
class IllegalFormat:public IllegalInput
{
public:
	virtual const char* what()const throw()
	{
		return ERROR_HINT_FORMAT_ILLEGAL;
	}
};
IllegalFormat illegal_format;
char get_char2(char i)
{
	return dic[(unsigned)i];
}
char get_char(unsigned char i)
{
	char c;
	if(i<=25)
	{
		c=i+'A';
	}
	else if(i<=51)
	{
		c=i+'a'-26;
	}
	else if(i<=61)
	{
		c=i+'0'-52;
	}
	else if(i==62u)
	{
		c='+';
	}
	else if(i==63u)
	{
		c='/';
	}
	else if(i==64)
	{
		c='=';
	}
	else
	{
		c=0;
		cout<<"error"<<__LINE__<<endl;
	}
	return c;
}
unsigned char get_binary(unsigned char c)
{
	unsigned char i;
	if(c>='A' && c<='Z')
	{
		i=c-'A';
	}
	else if(c>='a' && c<='z')
	{
		i=c-'a'+26;
	}
	else if(c>='0' && c<='9')
	{
		i=c-'0'+52;
	}
	else if(c=='+')
	{
		i=62;
	}
	else if(c=='/')
	{
		i=63;
	}
	else if(c=='=')
	{
		i=0;
	}
	else
	{
		i=64;
		throw(illegal_char);
	}
	return i;
}


int binary_to_base64(char *buffer,int len,char*result)
{
	int i,j,k,t;
	for(i=0,j=0;i<len;j+=4,i+=3)
	{
		mid[j]	=	(buffer[i]>>2);
		if(i+1==len)
		{
			mid[j+1]=	((buffer[i]&0x3)<<4);
			mid[j+2]= 64;
			mid[j+3]= 64;
		}
		else if(i+2==len)
		{
			mid[j+1]=	((buffer[i]&0x3)<<4) | ((buffer[i+1]&0xf0u)>>4);
			mid[j+2]=	((buffer[i+1]&0x0f)<<2);
			mid[j+3]=	64;
		}
		else
		{
			mid[j+1]=	((buffer[i]&0x3)<<4) | ((buffer[i+1]&0xf0u)>>4);
			mid[j+2]=	((buffer[i+1]&0x0f)<<2) | ((buffer[i+2]&0xc0u)>>6);
			mid[j+3]=	(buffer[i+2]&0x3f);
		}
	}
	for(k=0,t=0;k<j;)
	{
		result[t]=get_char2(mid[k]);
		t++;
		k++;
		if((k)%76==0)
		{
			result[t]='\n';
			t++;
		}
	}
	return t;
}
int base64_to_binary(char *input,int len,char*result)
{
	int i,j,k,t;

	for(i=0,j=0;i<len;i++)
	{
		/*
		if((i+1)%77==0)
		{
			assert(input[i]=='\n'||input[i]==0);
			i++;
		}
		*/
		if(input[i]==' '||input[i]=='\n'||input[i]=='\a'||input[i]=='\t')
		{
			continue;
		}
		mid[j]=get_binary(input[i]);
		j++;
	}
	if(j%4!=0)
	{
		throw illegal_format;
	}
	for(k=0,t=0;k<j;k+=4,t+=3)
	{
		assert((mid[k]&0xc0u)==0);
		assert((mid[k+1]&0xc0u)==0);
		assert((mid[k+2]&0xc0u)==0);
		assert((mid[k+3]&0xc0u)==0);
		result[t]	=	(mid[k]<<2) | ((mid[k+1]&0x30)>>4);
		result[t+1]	=	((mid[k+1]&0x0f)<<4)|((mid[k+2]&0x3c)>>2);;
		result[t+2] =	((mid[k+2]&0x03)<<6)|(mid[k+3]);
	}
	return t;
}

int main(int argc,char ** argv)
{
	int state=0;//1 code ,2 decode
	ifstream input_file;
	ofstream output_file;
	string file_name;
	input_file.exceptions(std::ifstream::failbit|std::ifstream::badbit);
	output_file.exceptions(std::ofstream::failbit|std::ofstream::badbit);
	try
	{
		if(argc==1 || strcmp(argv[1],"help")==0 || strcmp(argv[1],"-h")==0)
		{
			cout<<"-h           \tHelp"<<endl;
			cout<<"-e [filename]\tEncode"<<endl;
			cout<<"-d [filename]\tDecode"<<endl;
		}
		else if((strcmp(argv[1],"-e")==0||strcmp(argv[1],"-E")==0) && argc==3)
		{
			char buffer[128];
			char result[128];
			state=1;
			file_name=string(argv[2]);
			file_name.append(FILE_SUFFIX_CODE);
			input_file.open(argv[2],std::ios::in|std::ios::binary);
			output_file.open(file_name.c_str(),std::ios::out);
			input_file.clear();
			
			int len;
			int len2;
			int total_len;
			streampos beg,end;
			input_file.seekg(0,input_file.end);
			end=input_file.tellg();
			input_file.seekg(0,input_file.beg);
			beg=input_file.tellg();
			total_len=end-beg;
			while(total_len>0)
			{
				len=READ_LENTH_ONCE<total_len?READ_LENTH_ONCE:total_len;
				input_file.read(buffer,len);
				total_len-=len;
				len2=binary_to_base64(buffer,len,result);
				result[len2]=0;
				output_file<<(result);
			}
			cout<<"done,write to:"<<file_name.c_str()<<endl;

		}
		else if((strcmp(argv[1],"-d")==0||strcmp(argv[1],"-D")==0) && argc==3)
		{
			char buffer[128];
			char result[128];
			state=2;
			file_name=string(argv[2]);
			file_name.append(FILE_SUFFIX_DECODE);
			input_file.open(argv[2],std::ios::in);
			output_file.open(file_name.c_str(),std::ios::out|std::ios::binary);
			input_file.exceptions(0);
			int len;
			int len2;
			while(input_file.eof()==false)
			{
				len=0;
				char c;
				while((len<76)&&(input_file>>c) )
				{
					if(c==' '||c=='\n'||c=='\a'||c=='\t')
					{
						continue;
					}
					buffer[len]=c;
					len++;
				}
				len2=base64_to_binary(buffer,len,result);
				//cout.write(result,len2);
				output_file.write(result,len2);
				if(len<76)
				{
					//cout<<endl;
					output_file<<endl;
				}
			}
			if(input_file.eof())
			{
				cout<<"done,write to:"<<file_name.c_str()<<endl;
			}
		}
		else
		{
			cout<<"Error,invalid option:"<<argv[1]<<endl;
			return 0;
		}
	}
	catch (std::ios_base::failure e)
	{
		if(input_file.is_open()==false)
		{
			std::cerr <<"Error,cannot open file "<<argv[2]<<endl;
		}
		else if(input_file.eof()==true)
		{
			//open read success
		}
		else
		{
			std::cerr<<"Error,unknown, please check the input file."<<endl;
			std::cerr<<e.what()<<endl;
		}
	}
	catch (IllegalInput& e)
	{
		//delete file
		output_file.close();
		remove(file_name.c_str());
		std::cerr<<"Error, "<<e.what()<<endl;
	}

	if(input_file.is_open())
	{
		input_file.close();
	}
	if(output_file.is_open())
	{
		output_file.close();
	}
/*	
	//for test
	strcpy((char *)buffer,"Man is distinguished, not only by his reason, but by this singular passion from other animals, which is a lust of the mind, that by a perseverance of delight in the continued and indefatigable generation of knowledge, exceeds the short vehemence of any carnal pleasure.");
	int len=binary_to_base64(buffer,strlen((char*)buffer),result);
	cout<<result;
	cout<<endl;
	
	//cin>>result;
	strcpy((char*)result,"TWFuIGlzIGRpc3Rpbmd1aXNoZWQsIG5vdCBvbmx5IGJ5IGhpcyByZWFzb24sIGJ1dCBieSB0aGlz\nIHNpbmd1bGFyIHBhc3Npb24gZnJvbSBvdGhlciBhbmltYWxzLCB3aGljaCBpcyBhIGx1c3Qgb2Yg\ndGhlIG1pbmQsIHRoYXQgYnkgYSBwZXJzZXZlcmFuY2Ugb2YgZGVsaWdodCBpbiB0aGUgY29udGlu\ndWVkIGFuZCBpbmRlZmF0aWdhYmxlIGdlbmVyYXRpb24gb2Yga25vd2xlZGdlLCBleGNlZWRzIHRo\nZSBzaG9ydCB2ZWhlbWVuY2Ugb2YgYW55IGNhcm5hbCBwbGVhc3VyZS4=");
	int len2=base64_to_binary(result,strlen((char*)result),buffer);
	for(int i=0;i<len2;i++)
	{
		if(i%20==0)
		{
			cout<<endl;
		}
		printf("%c ",buffer[i]);
	}
	cout<<endl;
*/	
#ifdef _DEBUG
	system("pause");
#endif
	return 0;
}
