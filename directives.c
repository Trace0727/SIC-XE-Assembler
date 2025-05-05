/*********************************************
 *        DO NOT REMOVE THIS MESSAGE
 *
 * This file is provided by Professor Littleton
 * to assist students with completing Project 3.
 *
 *        DO NOT REMOVE THIS MESSAGE
 **********************************************/

#include "headers.h"

#define SINGLE_QUOTE 39

// List of valid directives
enum directives {
	// Although ERROR is not a valid directive, 
	// its presence helps the isDirective() function
	ERROR, BASE, BYTE, END, RESB, RESW, START
};

// Returns the value associated with a BYTE directive
int getByteValue(int directiveType, char* string)
{
	int hexValue = 0;

	if (directiveType == 'X') {
		// Convert hex string to integer
		return (int)strtol(string, NULL, 16);
	} else if (directiveType == 'C') {
		// Convert character string to hex
		char hexString[256] = ""; // twice the size of input value as each char can represent 2 hex digits
		for (int i = 2; i < strlen(string) - 1; i++) {
			char temp[3]; // for holding hex value of a single char
			sprintf(temp, "%02X", string[i]);
			strcat(hexString, temp);
		}
		hexValue = strtol(hexString, NULL, 16);
	}
	return hexValue;
}

// Do no modify any part of this function
// Returns the number of bytes required to store the BYTE directive value in memory
int getMemoryAmount(int directiveType, char* string)
{
	char hex[9] = { '\0' };
	int temp = 0;

	switch (directiveType)
	{
	case BASE:
	case END:
	case START:
		return 0;
		break;
	case BYTE:
		if (string[0] == 'X')
		{
			if (strlen(string) != 5)
			{
				displayError(OUT_OF_RANGE_BYTE, string);
				exit(-1);
			}
			else
				return 1;
		}
		else if (string[0] == 'C')
			return strlen(string) - 3;
		break;
	case RESB:
		return strtol(string, NULL, 10);
		break;
	case RESW:
		return strtol(string, NULL, 10) * 3;
		break;
	}
	return -1; // Should not happen
}

// Returns true if the provided directive type is the BASE directive; otherwise, false
bool isBaseDirective(int directiveType)
{
	return directiveType == BASE;
}

// Returns true if the provided directive type is the BYTE directive; otherwise, false
bool isDataDirective(int directiveType)
{
	return directiveType == BYTE;
}

// Do no modify any part of this function
// Tests whether the provided string is a valid directive
// Returns true if string is valid directive; otherwise, false
int isDirective(char* string)
{
	if (strcmp(string, "BASE") == 0) { return BASE; }
	else if (strcmp(string, "BYTE") == 0) { return BYTE; }
	else if(strcmp(string, "END") == 0) { return END; }
	else if (strcmp(string, "RESB") == 0) { return RESB; }
	else if (strcmp(string, "RESW") == 0) { return RESW; }
	else if (strcmp(string, "START") == 0) { return START; }
	else { return ERROR; }
}

// Returns true if the provided directive type is the END directive; otherwise, false
bool isEndDirective(int directiveType)
{
	return directiveType == END;
}

// Returns true if the provided directive type is the RESB or RESW directive; otherwise, false
bool isReserveDirective(int directiveType)
{
	if(directiveType == RESB || directiveType == RESW)
		return true;
	return false;
}

// Returns true if the provided directive type is the START directive; otherwise, false
bool isStartDirective(int directiveType)
{
	return directiveType == START;
}
