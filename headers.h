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

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

#define NAME_SIZE 7
#define SEGMENT_SIZE 9

#include "directives.h"
#include "errors.h"
#include "opcodes.h"
#include "symbols.h"

// Pass 1 structures
// Used for managing the various addresses for Pass 1 and Pass 2
typedef struct address {
	int start;
	int current;
	int increment;
	int base;
} address;

// Used for managing the various segments of a SIC/XE instruction
typedef struct segment {
	// Label   Operation   Operand
	// -----   ---------   -------
	// CLOOP   JSUB        RDREC
	char label[SEGMENT_SIZE];
	char operation[SEGMENT_SIZE];
	char operand[SEGMENT_SIZE];
} segment;

// Pass 2 structures
// Used to store Text Record entries for the Object Code file 
typedef struct recordEntry {
	int numBytes;
	int value;
} recordEntry;

// Used to store important data for the Object Code file
typedef struct objectFileData {
	int modificationCount;         // M records
	int modificationEntries[60];   // M records
	char programName[NAME_SIZE];   // H and M records
	int programSize;               // H record
	int recordAddress;             // T records
	int recordByteCount;           // T records
	recordEntry recordEntries[60]; // Store T record data
	int recordEntryCount;          // T records
	char recordType;               // H, T, E or M
	int startAddress;              // H and E records
} objectFileData;
