/*********************************************
*        DO NOT REMOVE THIS MESSAGE
*
* This file is provided by Professor Littleton
* to assist students with completing Project 3.
*
*  DO NOT MODIFY THIS FILE WITHOUT PERMISSION
*
*        DO NOT REMOVE THIS MESSAGE
**********************************************/
#pragma once

// List of possible errors
enum errors {
	// Pass 1 errors
	BLANK_RECORD = 1, DUPLICATE, FILE_NOT_FOUND, ILLEGAL_OPCODE_DIRECTIVE, ILLEGAL_SYMBOL, 
	MISSING_COMMAND_LINE_ARGUMENTS, OUT_OF_MEMORY, OUT_OF_RANGE_BYTE, OUT_OF_RANGE_WORD, 
	
	// Pass 2 errors
	ADDRESS_OUT_OF_RANGE,  // Format 3 opcode, but PC- and BASE-relative addressing is out of range
	ILLEGAL_OPCODE_FORMAT, // Format 4 is indicated for a Format 1 or Format 2 opcode
	UNKNOWN_SYMBOL         // The specified operand name is not found in the Symbol Table
};

void displayError(int errorType, char* errorInfo);