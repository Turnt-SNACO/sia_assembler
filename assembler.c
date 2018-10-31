#include <stdio.h>
#include <string.h>
#include <stdlib.h>

char * TrimLeft(char * s);
int AssembleLine(char * text, unsigned char * bytes);
unsigned char GetOpcode(char * key_word);

int main(int argc, char ** argv) {
	FILE * src = fopen(argv[1],"r");
	FILE * dst = fopen(argv[2],"w");
	while (!feof(src)) {
		unsigned char bytes[4];
		char line[1000];
		printf ("about to read\n");
		if (NULL != fgets(line, 1000, src)) {
			printf ("read: %s\n",line);
			int byte_count = AssembleLine(line,bytes);
			fwrite(bytes,byte_count,1,dst);
		}
	}
	fclose(src);
	fclose(dst);
	return 0;
}

char * TrimLeft(char * s) {
	while (* s == ' ' || * s == '\t') s++;
	return s;
}

char GetRegister(char * text) {
	if (* text == 'r' || * text == 'R') text++;
	else {
		printf("%s should use 'r' or 'R' to indicate register!\n", text);
		exit(1);
	}
	return atoi(text);
}

int AssembleLine(char * text, unsigned char * bytes) {
	text = TrimLeft(text);
	char * key_word = strtok(text, " ");
	bytes[0] = GetOpcode(key_word);
	printf("%s\n", key_word);
	//HALT
	if (bytes[0] == 0x00) {
		bytes[1] = 0x00;
		return 2;
	}
	//3R Commands (add, sub, mult, div, and, or)
	if (bytes[0] <= 0x60 && bytes[0] >= 0x10) {
		bytes[0] |= GetRegister(strtok(NULL, " "));
		bytes[1] = GetRegister(strtok(NULL, " ")) << 4 | GetRegister(strtok(NULL, " "));
		printf("JESUS CHRIST\n");
		return 2;
	}
	//Left/Right Shift
	if (bytes[0] == 0x70) {
		bytes[0] |= GetRegister(strtok(NULL, " "));
		if (strcmp("rightshift", key_word) == 0) bytes[1] = 0x20;
		else bytes[1] = 0x00;
		unsigned int sft_amt = atoi(strtok(NULL, " "));
		if (sft_amt > 15) bytes[1] |= 0x10;
		bytes[1] |= (sft_amt & 0x0F);
		return 2;
	}
	//Interrupt
	if (bytes[0] == 0x80) {
		unsigned int int_code = atoi(strtok(NULL, " "));
		bytes[0] |= (unsigned char)((int_code >> 8) & 0x0F);
		bytes[1] = (unsigned char)int_code;
		return 2;
	}
	//Addimmediate
	if (bytes[0] == 0x90) {
		bytes[0] |= GetRegister(strtok(NULL, " "));
		bytes[1] = atoi(strtok(NULL, " "));
		return 2;
	}
	//Branch
	if (bytes[0] == 0xA0 || bytes[0] == 0xB0) {
		bytes[0] |= GetRegister(strtok(NULL, " "));
		bytes[1] = (GetRegister(strtok(NULL, " ")) << 4);
		int offset = atoi(strtok(NULL, " "));
		bytes[1] |= (unsigned char)((offset >> 16) & 0x0F);
		bytes[2] = (unsigned char)(offset >> 8);
		bytes[3] = (unsigned char)(offset);
		return 4;	
	}
	//Jump
	if (bytes[0] == 0xC0) {
		unsigned int int_code = atoi(strtok(NULL, " "));
		bytes[3] = (unsigned char)int_code;
		bytes[2] = (unsigned char)(int_code >> 8);
		bytes[1] = (unsigned char)(int_code >> 16);
		bytes[0] |= (unsigned char)((int_code >> 24) & 0x0F);
		return 4;
	}
	//Iterate Over
	if (bytes[0] == 0xD0) {
		bytes[0] |=  GetRegister(strtok(NULL, " "));
		bytes[1] = atoi(strtok(NULL, " "));
		unsigned int jump_offset = atoi(strtok(NULL, " "));
		bytes[2] = (unsigned char)(jump_offset >> 8);
		bytes[3] = (unsigned char)(jump_offset);
		return 4;
	}
	//Load/Store
	if (bytes[0] == 0xE0 || bytes[0] == 0xF0) {
		bytes[0] |= GetRegister(strtok(NULL, " "));
		bytes[1] = (GetRegister(strtok(NULL, " ")) << 4);
		bytes[1] |= (atoi(strtok(NULL, " ")) & 0x0F);
		return 2;
	}
}
unsigned char GetOpcode(char * key_word) {
	if (strcmp("halt", key_word) == 0)
		return 0x00;
	if (strcmp("add", key_word) == 0) 
		return 0x10;
	if (strcmp("subtract", key_word) == 0) 
		return 0x50;
	if (strcmp("or", key_word) == 0) 
		return 0x60;
	if (strcmp("and", key_word) == 0) 
		return 0x20;
	if (strcmp("multiply", key_word) == 0) 
		return 0x40;
	if (strcmp("divide", key_word) == 0) 
		return 0x30;
	if (strcmp("halt", key_word) == 0) 
		return 0x00;
	if (strcmp("load", key_word) == 0) 
		return 0xE0;
	if (strcmp("store", key_word) == 0) 
		return 0xF0;
	if (strcmp("branchifless", key_word) == 0) 
		return 0xB0;
	if (strcmp("branchifequal", key_word) == 0) 
		return 0xA0;
	if (strcmp("jump", key_word) == 0) 
		return 0xC0;
	if (strcmp("rightshift", key_word) == 0 || strcmp("leftshift", key_word) == 0) 
		return 0x70;
	if (strcmp("iterateover", key_word) == 0) 
		return 0xD0;
	if (strcmp("interrupt", key_word) == 0) 
		return 0x80;
	if (strcmp("addimmediate", key_word) == 0)
		return 0x90;
	printf("PANIC! No opcode found!\n");
	exit(1);
}
