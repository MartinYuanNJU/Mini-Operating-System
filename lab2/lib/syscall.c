#include "lib.h"
#include "types.h"
/*
 * io lib here
 * 库函数写在这
 */
//static inline int32_t syscall(int num, uint32_t a1,uint32_t a2,
int32_t syscall(int num, uint32_t a1,uint32_t a2,
		uint32_t a3, uint32_t a4, uint32_t a5)
{
	int32_t ret = 0;
	//Generic system call: pass system call number in AX
	//up to five parameters in DX,CX,BX,DI,SI
	//Interrupt kernel with T_SYSCALL
	//
	//The "volatile" tells the assembler not to optimize
	//this instruction away just because we don't use the
	//return value
	//
	//The last clause tells the assembler that this can potentially
	//change the condition and arbitrary memory locations.

	/*
	XXX Note: ebp shouldn't be flushed
	    May not be necessary to store the value of eax, ebx, ecx, edx, esi, edi
	*/
	uint32_t eax, ecx, edx, ebx, esi, edi;
	uint16_t selector;
	
	asm volatile("movl %%eax, %0":"=m"(eax));
	asm volatile("movl %%ecx, %0":"=m"(ecx));
	asm volatile("movl %%edx, %0":"=m"(edx));
	asm volatile("movl %%ebx, %0":"=m"(ebx));
	asm volatile("movl %%esi, %0":"=m"(esi));
	asm volatile("movl %%edi, %0":"=m"(edi));
	asm volatile("movl %0, %%eax"::"m"(num));
	asm volatile("movl %0, %%ecx"::"m"(a1));
	asm volatile("movl %0, %%edx"::"m"(a2));
	asm volatile("movl %0, %%ebx"::"m"(a3));
	asm volatile("movl %0, %%esi"::"m"(a4));
	asm volatile("movl %0, %%edi"::"m"(a5));
	asm volatile("int $0x80");
	asm volatile("movl %%eax, %0":"=m"(ret));
	asm volatile("movl %0, %%eax"::"m"(eax));
	asm volatile("movl %0, %%ecx"::"m"(ecx));
	asm volatile("movl %0, %%edx"::"m"(edx));
	asm volatile("movl %0, %%ebx"::"m"(ebx));
	asm volatile("movl %0, %%esi"::"m"(esi));
	asm volatile("movl %0, %%edi"::"m"(edi));
	
	asm volatile("movw %%ss, %0":"=m"(selector)); //XXX %ds is reset after iret
	//selector = 16;
	asm volatile("movw %%ax, %%ds"::"a"(selector));
	
	return ret;
}

// API to support format in printf, if you can't understand, use your own implementation!
int dec2Str(int decimal, char *buffer, int size, int count);
int hex2Str(uint32_t hexadecimal, char *buffer, int size, int count);
int str2Str(char *string, char *buffer, int size, int count);

int printf(const char *format,...){
	int i=0; // format index
	char buffer[MAX_BUFFER_SIZE];
	int count=0; // buffer index
	int index=0; // parameter index
	void *paraList=(void*)&format;
	int state=0; // 0: legal character; 1: '%'; 2: illegal format
	int decimal=0;
	uint32_t hexadecimal=0;
	char *string=0;
	char character=0;
	while(format[i]!=0) {
        // TODO: support more format %s %d %x and so on
		if(state==0)
		{
			if(format[i]=='%')
				state=1;
			else
			{
				buffer[count]=format[i];
				count++;
				if(count==MAX_BUFFER_SIZE) {
					syscall(SYS_WRITE, STD_OUT, (uint32_t)buffer, (uint32_t)MAX_BUFFER_SIZE, 0, 0);
					count=0;
				}
			}
		}
		else if(state==1) // There is a % at the front
		{
			if(format[i]=='d')
			{
				decimal=*((int *)((int)paraList+sizeof(char *)+index));
				count=dec2Str(decimal,buffer,MAX_BUFFER_SIZE,count);
				index+=sizeof(int);
			}
			else if(format[i]=='x')
			{
				hexadecimal=*((uint32_t *)((int)paraList+sizeof(char *)+index));
				count=hex2Str(hexadecimal,buffer,MAX_BUFFER_SIZE,count);
				index+=sizeof(int);
			}
			else if(format[i]=='c')
			{
				character=*((char *)((int)paraList+sizeof(char *)+index));
				char temp[2];
				temp[0]=character;
				temp[1]='\0';
				count=str2Str(temp,buffer,MAX_BUFFER_SIZE,count);
				index+=sizeof(int);
			}
			else if(format[i]=='s')
			{
				string=*((char **)((int)paraList+sizeof(char *)+index));
				count=str2Str(string,buffer,MAX_BUFFER_SIZE,count);
				index+=sizeof(int);
			}
			else
			{
				buffer[count]=format[i];
				count++;
				if(count==MAX_BUFFER_SIZE) {
					syscall(SYS_WRITE, STD_OUT, (uint32_t)buffer, (uint32_t)MAX_BUFFER_SIZE, 0, 0);
					count=0;
				}
			}
			state=0;
		}
		i++;
	}
	if(count!=0)
		syscall(SYS_WRITE, STD_OUT, (uint32_t)buffer, (uint32_t)count, 0, 0);
    return 0;
}

int dec2Str(int decimal, char *buffer, int size, int count) {
	int i=0;
	int temp;
	int number[16];

	if(decimal<0){
		buffer[count]='-';
		count++;
		if(count==size) {
			syscall(SYS_WRITE, STD_OUT, (uint32_t)buffer, (uint32_t)size, 0, 0);
			count=0;
		}
		temp=decimal/10;
		number[i]=temp*10-decimal;
		decimal=temp;
		i++;
		while(decimal!=0){
			temp=decimal/10;
			number[i]=temp*10-decimal;
			decimal=temp;
			i++;
		}
	}
	else{
		temp=decimal/10;
		number[i]=decimal-temp*10;
		decimal=temp;
		i++;
		while(decimal!=0){
			temp=decimal/10;
			number[i]=decimal-temp*10;
			decimal=temp;
			i++;
		}
	}

	while(i!=0){
		buffer[count]=number[i-1]+'0';
		count++;
		if(count==size) {
			syscall(SYS_WRITE, STD_OUT, (uint32_t)buffer, (uint32_t)size, 0, 0);
			count=0;
		}
		i--;
	}
	return count;
}

int hex2Str(uint32_t hexadecimal, char *buffer, int size, int count) {
	int i=0;
	uint32_t temp=0;
	int number[16];

	temp=hexadecimal>>4;
	number[i]=hexadecimal-(temp<<4);
	hexadecimal=temp;
	i++;
	while(hexadecimal!=0){
		temp=hexadecimal>>4;
		number[i]=hexadecimal-(temp<<4);
		hexadecimal=temp;
		i++;
	}

	while(i!=0){
		if(number[i-1]<10)
			buffer[count]=number[i-1]+'0';
		else
			buffer[count]=number[i-1]-10+'a';
		count++;
		if(count==size) {
			syscall(SYS_WRITE, STD_OUT, (uint32_t)buffer, (uint32_t)size, 0, 0);
			count=0;
		}
		i--;
	}
	return count;
}

int str2Str(char *string, char *buffer, int size, int count) {
	int i=0;
	while(string[i]!=0){
		buffer[count]=string[i];
		count++;
		if(count==size) {
			syscall(SYS_WRITE, STD_OUT, (uint32_t)buffer, (uint32_t)size, 0, 0);
			count=0;
		}
		i++;
	}
	return count;
}

// API to support format in scanf, if you can't understand, use your own implementation!
int matchWhiteSpace(char *buffer, int size, int *count);
int str2Dec(int *dec, char *buffer, int size, int *count);
int str2Hex(int *hex, char *buffer, int size, int *count);
int str2Str2(char *string, int avail, char *buffer, int size, int *count);

int scanf(const char *format,...) {
    // TODO: implement scanf function, return the number of input parameters
	int parameter_num=0;
	int i=0;
	char buffer[MAX_BUFFER_SIZE];
	for(int i=0;i<MAX_BUFFER_SIZE;i++)
		buffer[i]=0;
	int count=0;
	int index=0;
	void *paraList=(void*)&format;
	int state=0;
	int *decimal=0;
	int strlength=0;
	uint32_t *hexadecimal=0;
	char *character=0;
	char *string=0;
	while(format[i]!=0)
	{
		if(state==0)
		{
			if(format[i]=='%')
				state=1;
			else if(format[i]==' ')
			{
				matchWhiteSpace(buffer,MAX_BUFFER_SIZE,&count);
				state=0;
			}
			else
			{
				if(buffer[count]==0)
				{
					int Ret=0;
					do{
						Ret=syscall(SYS_READ, STD_IN, (uint32_t)buffer, MAX_BUFFER_SIZE, 0, 0);
					}while(Ret == 0 || Ret == -1);
					count=0;
				}
				if(format[i]!=buffer[count])
					return parameter_num;
				else
				{
					count++;
					state=0;
				}
			}
		}
		else if(state==1)
		{
			if(format[i]=='d')
			{
				decimal=*(int **)((int)paraList+sizeof(char *)+index);
				str2Dec(decimal,buffer,MAX_BUFFER_SIZE,&count);
				index+=sizeof(int);
				state=0;
				parameter_num++;
			}
			else if(format[i]=='x')
			{
				hexadecimal=*(uint32_t **)((int)paraList+sizeof(char *)+index);
				str2Hex((int *)hexadecimal,buffer,MAX_BUFFER_SIZE,&count);
				index+=sizeof(int);
				state=0;
				parameter_num++;
			}
			else if(format[i]=='c')
			{
				character=*((char **)((int)paraList+sizeof(char *)+index));
				char temp[2];
				str2Str2(temp,2,buffer,MAX_BUFFER_SIZE,&count);
				*character=temp[0];
				index+=sizeof(int);
				state=0;
				parameter_num++;
			}
			else if(format[i]=='s')
			{
				string=*((char **)((int)paraList+sizeof(char *)+index));
				str2Str2(string,MAX_BUFFER_SIZE,buffer,MAX_BUFFER_SIZE,&count);
				index+=sizeof(int);
				state=0;
				parameter_num++;
			}
			else if(format[i]>='0'&&format[i]<='9')
			{
				strlength=format[i]-'0';
				state=2;
			}
			else
				state=0;
		}
		else if(state==2)
		{
			if(format[i]>='0'&&format[i]<='9')
			{
				strlength=strlength*10+format[i]-'0';
				state=2;
			}
			else if(format[i]=='s'&&strlength>0)
			{
				string=*((char **)((char *)paraList+sizeof(char *)+index));
				str2Str2(string,strlength+1,buffer,MAX_BUFFER_SIZE,&count);
				index+=sizeof(int);
				strlength=0;
				state=0;
				parameter_num++;
			}
			else
			{
				strlength=0;
				state=0;
			}
		}
		i++;
	}
    return parameter_num;
}

int matchWhiteSpace(char *buffer, int size, int *count){
	int ret=0;
	while(1){
		if(buffer[*count]==0){
			do{
				ret=syscall(SYS_READ, STD_IN, (uint32_t)buffer, (uint32_t)size, 0, 0);
			}while(ret == 0 || ret == -1);
			(*count)=0;
		}
		if(buffer[*count]==' ' ||
		   buffer[*count]=='\t' ||
		   buffer[*count]=='\n'){
			(*count)++;
		}
		else
			return 0;
	}
}

int str2Dec(int *dec, char *buffer, int size, int *count) {
	int sign=0; // positive integer
	int state=0;
	int integer=0;
	int ret=0;
	while(1){
		if(buffer[*count]==0){
			do{
				ret=syscall(SYS_READ, STD_IN, (uint32_t)buffer, (uint32_t)size, 0, 0);
			}while(ret == 0 || ret == -1);
			(*count)=0;
		}
		if(state==0){
			if(buffer[*count]=='-'){
				state=1;
				sign=1;
				(*count)++;
			}
			else if(buffer[*count]>='0' &&
				buffer[*count]<='9'){
				state=2;
				integer=buffer[*count]-'0';
				(*count)++;
			}
			else if(buffer[*count]==' ' ||
				buffer[*count]=='\t' ||
				buffer[*count]=='\n'){
				state=0;
				(*count)++;
			}
			else
				return -1;
		}
		else if(state==1){
			if(buffer[*count]>='0' &&
			   buffer[*count]<='9'){
				state=2;
				integer=buffer[*count]-'0';
				(*count)++;
			}
			else
				return -1;
		}
		else if(state==2){
			if(buffer[*count]>='0' &&
			   buffer[*count]<='9'){
				state=2;
				integer*=10;
				integer+=buffer[*count]-'0';
				(*count)++;
			}
			else{
				if(sign==1)
					*dec=-integer;
				else
					*dec=integer;
				return 0;
			}
		}
		else
			return -1;
	}
	return 0;
}

int str2Hex(int *hex, char *buffer, int size, int *count) {
	int state=0;
	int integer=0;
	int ret=0;
	while(1){
		if(buffer[*count]==0){
			do{
				ret=syscall(SYS_READ, STD_IN, (uint32_t)buffer, (uint32_t)size, 0, 0);
			}while(ret == 0 || ret == -1);
			(*count)=0;
		}
		if(state==0){
			if(buffer[*count]=='0'){
				state=1;
				(*count)++;
			}
			else if(buffer[*count]==' ' ||
				buffer[*count]=='\t' ||
				buffer[*count]=='\n'){
				state=0;
				(*count)++;
			}
			else
				return -1;
		}
		else if(state==1){
			if(buffer[*count]=='x'){
				state=2;
				(*count)++;
			}
			else
				return -1;
		}
		else if(state==2){
			if(buffer[*count]>='0' && buffer[*count]<='9'){
				state=3;
				integer*=16;
				integer+=buffer[*count]-'0';
				(*count)++;
			}
			else if(buffer[*count]>='a' && buffer[*count]<='f'){
				state=3;
				integer*=16;
				integer+=buffer[*count]-'a'+10;
				(*count)++;
			}
			else if(buffer[*count]>='A' && buffer[*count]<='F'){
				state=3;
				integer*=16;
				integer+=buffer[*count]-'A'+10;
				(*count)++;
			}
			else
				return -1;
		}
		else if(state==3){
			if(buffer[*count]>='0' && buffer[*count]<='9'){
				state=3;
				integer*=16;
				integer+=buffer[*count]-'0';
				(*count)++;
			}
			else if(buffer[*count]>='a' && buffer[*count]<='f'){
				state=3;
				integer*=16;
				integer+=buffer[*count]-'a'+10;
				(*count)++;
			}
			else if(buffer[*count]>='A' && buffer[*count]<='F'){
				state=3;
				integer*=16;
				integer+=buffer[*count]-'A'+10;
				(*count)++;
			}
			else{
				*hex=integer;
				return 0;
			}
		}
		else
			return -1;
	}
	return 0;
}

int str2Str2(char *string, int avail, char *buffer, int size, int *count) {
	int i=0;
	int state=0;
	int ret=0;
	while(i < avail-1){
		if(buffer[*count]==0){
			do{
				ret=syscall(SYS_READ, STD_IN, (uint32_t)buffer, (uint32_t)size, 0, 0);
			}while(ret == 0 || ret == -1);
			(*count)=0;
		}
		if(state==0){
			if(buffer[*count]==' ' ||
			   buffer[*count]=='\t' ||
			   buffer[*count]=='\n'){
				state=0;
				(*count)++;
			}
			else{
				state=1;
				string[i]=buffer[*count];
				i++;
				(*count)++;
			}
		}
		else if(state==1){
			if(buffer[*count]==' ' ||
			   buffer[*count]=='\t' ||
			   buffer[*count]=='\n'){
				string[i]=0;
				return 0;
			}
			else{
				state=1;
				string[i]=buffer[*count];
				i++;
				(*count)++;
			}
		}
		else
			return -1;
	}
	string[i]=0;
	return 0;
}
