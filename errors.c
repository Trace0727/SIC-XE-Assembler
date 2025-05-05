/*********************************************
 *        DO NOT REMOVE THIS MESSAGE
 *
 * This file is provided by Professor Littleton
 * to assist students with completing Project 3.
 *
 *        DO NOT REMOVE THIS MESSAGE
 **********************************************/

#include "headers.h"

// Do no modify any part of this function
// Displays the specified error along with the provided error information
void displayError(int errorType, char* errorInfo)
{
	// Determine which error message to display
	switch (errorType)
	{
	// Pass 1 errors
	// Blank line found
	case BLANK_RECORD:
		printf("ERROR: Source File Contains Blank Lines.\n");
		break;
		// The symbol name already exists in the Symbol Table
	case DUPLICATE:
		printf("ERROR: Duplicate Symbol Name (%s) Found in Source File.\n", errorInfo);
		break;
		// The provided file was not found
	case FILE_NOT_FOUND:
		printf("FATAL ERROR: File Not Found (%s).\n", errorInfo);
		break;
		// An unknown opcode or directive name exists in the Operation segment of an instruction
	case ILLEGAL_OPCODE_DIRECTIVE:
		printf("ERROR: Illegal Opcode or Directive (%s) Found in Source File.\n", errorInfo);
		break;
		// An opcode or directive name exists in the Label segment of an instruction
	case ILLEGAL_SYMBOL:
		printf("ERROR: Symbol Name (%s) Cannot be a Command or Directive.\n", errorInfo);
		break;
		// The input filename was not provided as a command-line argument
	case MISSING_COMMAND_LINE_ARGUMENTS:
		printf("Usage: %s inputFile\n", errorInfo);
		break;
		// The current memory value exceeds the maximum SIC/XE memory (0x100000)
	case OUT_OF_MEMORY:
		printf("ERROR: Program Address (%s) Exceeds Maximum Memory Address [0x100000].\n", errorInfo);
		break;
		// The specified BYTE value exceeds the valid range of 00 to FF
	case OUT_OF_RANGE_BYTE:
		printf("ERROR: Byte Value (%s) Out of Range [00 to FF].\n", errorInfo);
		break;

		// Pass 2 errors
		// Format 3 opcode, but PC- and BASE-relative addressing is out of range
	case ADDRESS_OUT_OF_RANGE:
		printf("ERROR: Format 3 Opcode (%s) Address Displacement Out of Range [-2,048 to 4,096].\n", errorInfo);
		break;
		// Format 4 is indicated for a Format 1 or Format 2 opcode
	case ILLEGAL_OPCODE_FORMAT:
		printf("ERROR: Format 4 Indicated (%s) for Other Than Format 3 Opcode.\n", errorInfo);
		break;
		// The specified operand name is not found in the Symbol Table
	case UNKNOWN_SYMBOL:
		printf("ERROR: Unknown Operand Symbol (%s).\n", errorInfo);
		break;
	}
}
