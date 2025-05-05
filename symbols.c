/*********************************************
 *        DO NOT REMOVE THIS MESSAGE
 *
 * This file is provided by Professor Littleton
 * to assist students with completing Project 3.
 *
 *        DO NOT REMOVE THIS MESSAGE
 **********************************************/

#include "headers.h"

#define MOD_SIZE 10
#define SYMBOL_TABLE_SEGMENTS 10
#define SYMBOL_TABLE_SIZE 100

int computeHash(char* input);
bool isDirectAddressing(char* string);

// Do no modify any part of this function
// Compute a hash value for the provide symbol name
int computeHash(char* symbolName)
{
	int hash = 0;

	for (int x = 0; x < strlen(symbolName); x++)
	{
		hash += symbolName[x];
	}
	return hash % MOD_SIZE * SYMBOL_TABLE_SEGMENTS;
}

// Do no modify any part of this function
// Print the contents of the Symbol Table to the screen
void displaySymbolTable(symbol* symbolTable[])
{
	printf("\n%-5s  %-6s  %-7s\n", "Index", " Name ", "Address");
	printf("%-5s  %-6s  %-7s\n", "-----", "------", "-------");
	for (int x = 0; x < SYMBOL_TABLE_SIZE; x++)
	{
		if (symbolTable[x] == NULL)
			continue;
		printf("%5d  %-6s  0x%X\n", x, symbolTable[x]->name, symbolTable[x]->address);
	}
}

// Do no modify any part of this function
// Returns the address of the specified string if found; otherwise, -1
int getSymbolAddress(struct symbol* symbolArray[], char* string)
{
	int hashIndex;

	if(!isDirectAddressing(string))
	{
		memmove(string, &(string[1]), strlen(&(string[1])) + 1);
	}
	hashIndex = computeHash(string);

	while (symbolArray[hashIndex] != NULL && hashIndex < SYMBOL_TABLE_SIZE)
	{
		if (strcmp(symbolArray[hashIndex]->name, string) == 0)
		{
			return symbolArray[hashIndex]->address;
		}
		hashIndex++;
	}
	displayError(UNKNOWN_SYMBOL, string);
	exit(-1);
}

// Do no modify any part of this function
// Set each element of the Symbol Table to NULL
void initializeSymbolTable(symbol* symbolTable[])
{
	for (int x = 0; x < SYMBOL_TABLE_SIZE; x++)
	{
		symbolTable[x] = NULL;
	}
}

// Do no modify any part of this function
// Add a symbol to an empty location in the Symbol Table
void insertSymbol(symbol* symbolTable[], char symbolName[], int symbolAddress)
{
	int hashIndex = computeHash(symbolName);

	for (int x = hashIndex; x < SYMBOL_TABLE_SIZE; x++)
	{
		if (symbolTable[x] == NULL)
		{
			symbolTable[x] = (symbol*)malloc(sizeof(symbol));
			strcpy(symbolTable[x]->name, symbolName);
			symbolTable[x]->address = symbolAddress;

			// printf("Added %s to Symbol Table at Index %2d\n", symbolName, x);
			return;
		}
		else
		{
			if (strcmp(symbolTable[x]->name, symbolName) == 0)
			{
				displayError(DUPLICATE, symbolTable[x]->name);
				exit(-1);
			}
		}
	}
}

// Do no modify any part of this function
// Tests whether the provided string contains an Indirect '@' or Immediate '#' symbol
// Returns false if string contains an Indirect or Immediate symbol; otherwise, true
bool isDirectAddressing(char* string)
{
	return !(string[0] == '#' || string[0] == '@');
}
