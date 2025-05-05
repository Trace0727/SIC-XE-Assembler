#include "headers.h"

// Pass 1 constants
#define COMMENT 35
#define INPUT_BUF_SIZE 60
#define NEW_LINE 10
#define SPACE 32
#define SYMBOL_TABLE_SIZE 100

// Pass 2 constants
#define BLANK_INSTRUCTION 0x000000
#define FLAG_B 0x04
#define FLAG_E 0x01
#define FLAG_I 0x10
#define FLAG_N 0x20
#define FLAG_P 0x02
#define FLAG_X 0x08
#define FORMAT_1 1
#define FORMAT_2 2
#define FORMAT_3 3
#define FORMAT_3_MULTIPLIER 0x1000
#define FORMAT_4 4
#define FORMAT_4_MULTIPLIER 0x100000
#define IMMEDIATE_CHARACTER '#'
#define INDEX_STRING ",X"
#define INDIRECT_CHARACTER '@'
#define MAX_RECORD_BYTE_COUNT 30
#define OPCODE_MULTIPLIER 0x100
#define OUTPUT_BUF_SIZE 70
#define REGISTER_A 0X0
#define REGISTER_B 0X3
#define REGISTER_L 0X2
#define REGISTER_MULTIPLIER 0x10
#define REGISTER_S 0X4
#define REGISTER_T 0X5
#define REGISTER_X 0X1
#define RSUB_INSTRUCTION 0x4C0000
#define BASE_MAX_RANGE 4096
#define PC_MAX_RANGE 2048
#define PC_MIN_RANGE -2048

// Pass 1 functions
void performPass1(symbol* symbolTable[], char* filename, address* addresses);
segment* prepareSegments(char* line);
void trim(char string[]);

// Pass 2 functions
int computeFlagsAndAddress(struct symbol* symbolArray[], address* addresses, segment* segments, int format);
char* createFilename(char* filename, const char* extension);
void flushTextRecord(FILE* file, objectFileData* data, address* addresses);
int getRegisters(char* operand);
int getRegisterValue(char registerName);
bool isNumeric(char* string);
void performPass2(struct symbol* symbolTable[], char* filename, address* addresses);
void writeToLstFile(FILE* file, int address, segment* segments, int opcode);
void writeToObjFile(FILE* file, objectFileData data);

int main(int argc, char* argv[])
{
	// Do not modify this statement
	address addresses = { 0x00, 0x00, 0x00 };

	// Check whether at least one (1) input file was provided
	if(argc < 2)
	{
		displayError(MISSING_COMMAND_LINE_ARGUMENTS, argv[0]);
		exit(-1);
	}

	symbol* symbols[SYMBOL_TABLE_SIZE] = { NULL };

	performPass1(symbols, argv[1], &addresses);

	//	displaySymbolTable(symbols);
	//
	//	//Display the assembly summary data
	//	printf("\nStarting Address: 0x%X\nEnding Address: 0x%X\nProgram Size (bytes): %d\n", addresses.start, addresses.current, addresses.current - addresses.start);

	performPass2(symbols, argv[1], &addresses);

	printf("\n\nDone!\n\n");
}


// Determines the Format 3/4 flags and computes address displacement for Format 3 instruction
int computeFlagsAndAddress(symbol* symbolArray[],
                           address* addresses,
                           segment* segments,
                           int format)
{
    char operandBuf[INPUT_BUF_SIZE];
    strcpy(operandBuf, segments->operand);

    // RSUB or no‐operand
    if (strlen(operandBuf) == 0) {
        int opc = getOpcodeValue(segments->operation) & 0xFC;
        return (opc << 16);
    }

    // Default “direct” addressing (N=1, I=1)
    int flags = FLAG_N | FLAG_I;

    // Immediate (#) or indirect (@)
    if (operandBuf[0] == IMMEDIATE_CHARACTER) {
        flags = FLAG_I;
        memmove(operandBuf, operandBuf + 1, strlen(operandBuf));
    }
    else if (operandBuf[0] == INDIRECT_CHARACTER) {
        flags = FLAG_N;
        memmove(operandBuf, operandBuf + 1, strlen(operandBuf));
    }

    // Indexed (,X)
    char* idx = strstr(operandBuf, INDEX_STRING);
    if (idx) {
        flags |= FLAG_X;
        *idx = '\0';
    }

    // Build 6-bit opcode + NI bits
    int opcode = getOpcodeValue(segments->operation) & 0xFC;
    int instr  = (opcode | (flags >> 4)) << 16;

    // FORMAT 4? (set E-bit and use 20-bit address)
    if (format == FORMAT_4) {
        flags |= FLAG_E;
        int addr = getSymbolAddress(symbolArray, operandBuf);
        return instr
             | ((flags & 0xF) << 12)
             | (addr & 0xFFFFF);
    }

    // Numeric‐literal branch (#0, #5, etc.)
    if ((flags & FLAG_I) && isNumeric(operandBuf)) {
        int val = strtol(operandBuf, NULL, 10);
        return instr | (val & 0xFFF);
    }

    // Symbolic addressing: PC‐relative or base‐relative
    int target = getSymbolAddress(symbolArray, operandBuf);
    int pc     = addresses->current + addresses->increment;
    int disp   = target - pc;

    // PC‐relative?
    if (disp >= PC_MIN_RANGE && disp <= PC_MAX_RANGE) {
        flags |= FLAG_P;
        return instr | (disp & 0xFFF);
    }

    // Base‐relative?
    disp = target - addresses->base;
    if (disp >= 0 && disp <= BASE_MAX_RANGE) {
        flags |= FLAG_B;
        return instr | (disp & 0xFFF);
    }

    // Out of range
    displayError(ADDRESS_OUT_OF_RANGE, operandBuf);
    exit(1);
}

// Do no modify any part of this function
// Returns a new filename using the provided filename and extension
char* createFilename(char* filename, const char* extension)
{
	char* temp = (char*)malloc(sizeof(char) * strlen(filename) + 1);

	int n = strchr(filename, '.') - filename;
	strncpy(temp, filename, n);
	strcat(temp, extension);
	return temp;
}

// Do no modify any part of this function
// Writes existing data to Object Data file and resets values
void flushTextRecord(FILE* file, objectFileData* data, address* addresses)
{
	writeToObjFile(file, *data);

	data->recordAddress = addresses->current;
	data->recordByteCount = 0;
	data->recordEntryCount = 0;
}


// Returns a hex byte containing the registers listed in the provided operand
int getRegisters(char* operand)
{
	int registerValue = 0;

	// Compute value of first register
	registerValue = getRegisterValue(operand[0]) * REGISTER_MULTIPLIER;

	// Check if a second register is present in the operand string
	int operandLength = strlen(operand);
	if (operandLength > 1) {
		registerValue += getRegisterValue(operand[operandLength - 1]);
	}

	return registerValue;
}

// Do no modify any part of this function
// Returns the hex value for the provided register name
int getRegisterValue(char registerName)
{
	switch(registerName)
	{
	case 'A':
		return REGISTER_A;
	case 'B':
		return REGISTER_B;
	case 'L':
		return REGISTER_L;
	case 'S':
		return REGISTER_S;
	case 'T':
		return REGISTER_T;
	case 'X':
		return REGISTER_X;
	default:
		return -1;
	}
}


// Do no modify any part of this function
// Returns true if the provided string contains a numeric value; otherwise, false
bool isNumeric(char* string)
{
	for(int x = 0; x < strlen(string); x++)
	{
		if(!isdigit(string[x]))
		{
			return false;
		}
	}
	return true;
}


// Do no modify any part of this function
// Performs Pass 1 of the SIC/XE assembler
void performPass1(symbol* symbolTable[], char* filename, address* addresses)
{
	char line[INPUT_BUF_SIZE];
	FILE* file;
	int directiveType = 0;

	file = fopen(filename, "r");
	if (!file)
	{
		displayError(FILE_NOT_FOUND, filename);
		exit(-1);
	}

	while (fgets(line, INPUT_BUF_SIZE, file))
	{
		if (addresses->current >= 0x100000)
		{
			char value[10];
			sprintf(value, "0x%X", addresses->current);
			displayError(OUT_OF_MEMORY, value);
			exit(-1);
		}

		if (line[0] < 32) // Line is blank
		{
			displayError(BLANK_RECORD, NULL);
			exit(-1);
		}
		else if (line[0] == COMMENT) // Line is comment
		{
			continue;
		}
		else
		{
			segment* segments = prepareSegments(line);

			if (isDirective(segments->label) || isOpcode(segments->label))
			{
				displayError(ILLEGAL_SYMBOL, segments->label);
				exit(-1);
			}

			if (directiveType != isDirective(segments->operation))
			{
				if (isStartDirective(directiveType))
				{
					addresses->start = addresses->current = strtol(segments->operand, NULL, 16);
					continue;
				}
				else
				{
					addresses->increment = getMemoryAmount(directiveType, segments->operand);
				}
			}
			else if (isOpcode(segments->operation))
			{
				addresses->increment = getOpcodeFormat(segments->operation);
			}
			else
			{
				displayError(ILLEGAL_OPCODE_DIRECTIVE, segments->operation);
				exit(-1);
			}

			if (strlen(segments->label) > 0)
			{
				insertSymbol(symbolTable, segments->label, addresses->current);
			}

			addresses->current += addresses->increment;
		}
		memset(line, '\0', INPUT_BUF_SIZE);
	}
	fclose(file);
}


// Performs Pass 2 of the SIC/XE assembler
void performPass2(symbol* symbolTable[],
                  char* filename,
                  address* addresses)
{
    // Open listing and object files
    char* lstName = createFilename(filename, ".lst");
    char* objName = createFilename(filename, ".obj");
    FILE* lst = fopen(lstName, "w");
    FILE* obj = fopen(objName, "w");
    if (!lst || !obj) {
        displayError(FILE_NOT_FOUND, filename);
        exit(-1);
    }

    // 1) Header record
    objectFileData hdr = {0};
    hdr.recordType   = 'H';
    strncpy(hdr.programName, filename, NAME_SIZE - 1);
    hdr.startAddress = addresses->start;
    hdr.programSize  = addresses->current - addresses->start;
    writeToObjFile(obj, hdr);

    // 2) Begin first Text record
    objectFileData txt = {0};
    txt.recordType    = 'T';
    txt.recordAddress = addresses->start;

    // 3) Pass 2: re-scan the source
    FILE* src = fopen(filename, "r");
    char  line[INPUT_BUF_SIZE];
    while (fgets(line, sizeof line, src)) {
        segment* seg   = prepareSegments(line);
        int      dtype = isDirective(seg->operation);

        // A) BYTE directive?  pack C'…' or X'…'
        if (isDataDirective(dtype)) {
            // seg->operand is like "C'EOF'" or "X'F1'"
            char* op      = seg->operand;
            int   code    = 0;
            int   nbytes  = 0;

            if (op[0] == 'C') {
                // character literal
                char* p = strchr(op, '\'') + 1;
                char* end = strrchr(op, '\'');
                while (p < end) {
                    code = (code << 8) | (unsigned char)(*p++);
                    nbytes++;
                }
            }
            else if (op[0] == 'X') {
                // hex literal
                char* p = strchr(op, '\'') + 1;
                char* end = strrchr(op, '\'');
                int len = end - p;
                nbytes = len / 2;
                code = (int)strtol(p, NULL, 16);
            }
            else {
                // Shouldn’t happen, but guard it
                displayError(UNKNOWN_SYMBOL, op);
                exit(1);
            }

            // Flush T-record if adding this BYTE would exceed 30 bytes
            if (txt.recordByteCount + nbytes > MAX_RECORD_BYTE_COUNT) {
                flushTextRecord(obj, &txt, addresses);
                txt.recordType    = 'T';
                txt.recordAddress = addresses->current;
            }

            // Append the BYTE literal bytes
            txt.recordEntries[txt.recordEntryCount++] = (recordEntry){ nbytes, code };
            txt.recordByteCount += nbytes;

            // Write to listing and advance location by nbytes
            writeToLstFile(lst, addresses->current, seg, code);
            addresses->current += nbytes;
            continue;
        }

        // B) Any other directive? (START, END, RESW, RESB, BASE, etc.)
        if (!isOpcode(seg->operation)) {
            writeToLstFile(lst, addresses->current, seg, 0);
            addresses->current += addresses->increment;
            continue;
        }

        // C) Real opcode → compute object code
        int fmt     = getOpcodeFormat(seg->operation);
        int objCode = computeFlagsAndAddress(
                          symbolTable,
                          addresses,
                          seg,
                          (fmt == 4 ? FORMAT_4 : FORMAT_3)
                      );
        int nbytes  = (fmt == 2 ? 2 : (fmt == 1 ? 1 : 3));

        // Flush T-record if it would overflow
        if (txt.recordByteCount + nbytes > MAX_RECORD_BYTE_COUNT) {
            flushTextRecord(obj, &txt, addresses);
            txt.recordType    = 'T';
            txt.recordAddress = addresses->current;
        }

        // Append this instruction
        txt.recordEntries[txt.recordEntryCount++] = (recordEntry){ nbytes, objCode };
        txt.recordByteCount += nbytes;

        // Write to listing and advance
        writeToLstFile(lst, addresses->current, seg, objCode);
        addresses->current += addresses->increment;
    }
    fclose(src);

    // 4) Flush final Text record if any
    if (txt.recordEntryCount)
        flushTextRecord(obj, &txt, addresses);

    // 5) End record
    objectFileData endRec = {0};
    endRec.recordType   = 'E';
    endRec.startAddress = addresses->start;
    writeToObjFile(obj, endRec);

    // 6) Close files
    fclose(lst);
    fclose(obj);
}

// Do no modify any part of this function
// Separates a SIC/XE instruction into individual sections
segment* prepareSegments(char* statement)
{
	segment* temp = malloc(sizeof(segment));

	strncpy(temp->label, statement, SEGMENT_SIZE - 1);
	strncpy(temp->operation, statement + SEGMENT_SIZE - 1, SEGMENT_SIZE - 1);
	strncpy(temp->operand, statement + (SEGMENT_SIZE - 1) * 2, SEGMENT_SIZE - 1);

	trim(temp->label); // Label
	trim(temp->operation); // Operation
	trim(temp->operand); // Operand

	return temp;
}


// Do no modify any part of this function
// Removes spaces from the end of a segment value
void trim(char value[])
{
	for (int x = 0; x < SEGMENT_SIZE; x++)
	{
		if (value[x] == SPACE)
		{
			value[x] = '\0';
		}
	}
}


// Do no modify any part of this function
// Write SIC/XE instructions along with address and object code information of source code listing file
void writeToLstFile(FILE* file, int address, segment* segments, int opcode)
{
	char ctrlString[27];
	int length;

	int directiveType = isDirective(segments->operation);

	if (isStartDirective(directiveType) ||
			isBaseDirective(directiveType) ||
			isReserveDirective(directiveType))
	{
		fprintf(file, "%-8X%-8s%-8s%-8s\n", address, segments->label, segments->operation, segments->operand);
	}
	else if (isEndDirective(directiveType))
	{
		fprintf(file, "%-8X%-8s%-8s%-8s", address, segments->label, segments->operation, segments->operand);
	}
	else
	{
		if (isDataDirective(directiveType))
		{
			length = getMemoryAmount(directiveType, segments->operand) * 2;
		}
		else
		{
			length = getOpcodeFormat(segments->operation) * 2;
		}
		sprintf(ctrlString, "%%-8X%%-8s%%-8s%%-8s %%0%dX\n", length);

		fprintf(file, ctrlString, address, segments->label, segments->operation, segments->operand, opcode);
	}
}


// Do no modify any part of this function
// Write object code data to object code file
void writeToObjFile(FILE* file, objectFileData data)
{
	char ctrlString[27];

	if (data.recordType == 'H')
	{
		fprintf(file, "H%-6s%06X%06X\n", data.programName, data.startAddress, data.programSize);
	}
	else if (data.recordType == 'T')
	{
		fprintf(file, "T%06X%02X", data.recordAddress, data.recordByteCount);
		for (int x = 0; x < data.recordEntryCount; x++)
		{
			sprintf(ctrlString, "%%0%dX", data.recordEntries[x].numBytes * 2);
			fprintf(file, ctrlString, data.recordEntries[x].value);
		}
		fprintf(file, "\n");
	}
	else if (data.recordType == 'E')
	{
		fprintf(file, "E%06X", data.startAddress);
	}
	else if (data.recordType == 'M')
	{
		for (int x = 0; x < data.modificationCount; x++)
		{
			fprintf(file, "M%06X05+%s\n", data.modificationEntries[x], data.programName);
		}
	}
}
