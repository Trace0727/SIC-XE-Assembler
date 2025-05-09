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

	// Check if at least one input file was provided
	if(argc < 2)
	{
		displayError(MISSING_COMMAND_LINE_ARGUMENTS, argv[0]);
		exit(-1);
	}

	symbol* symbols[SYMBOL_TABLE_SIZE] = { NULL };

	performPass1(symbols, argv[1], &addresses);

	performPass2(symbols, argv[1], &addresses);

	printf("\n\nDone!\n\n");
}

// Determines the Format 3/4 flags and computes address displacement for Format 3 instruction
int computeFlagsAndAddress(symbol* symbolArray[], address* addresses, segment* segments, int format)
{
	// Extract operation and operand from the segment
	    char op[SEGMENT_SIZE];
	    char operand[SEGMENT_SIZE];
	    strcpy(op, segments->operation);
	    strcpy(operand, segments->operand);

	    // Determine if operand uses immediate (#), indirect (@), or indexed (,X) addressing
	    bool isImmediate = (operand[0] == IMMEDIATE_CHARACTER);
	    bool isIndirect = (operand[0] == INDIRECT_CHARACTER);
	    bool isIndexed = (strstr(operand, INDEX_STRING) != NULL);

	    // Clean up operand: remove addressing symbols and ",X" if present
	    if (isImmediate || isIndirect) {
	        memmove(operand, operand + 1, strlen(operand));
	    }
	    char* comma = strstr(operand, INDEX_STRING);
	    if (comma != NULL) {
	        *comma = '\0'; // Remove ,X
	    }

	    // Set n and i flags
	    int n, i;
	    if (isImmediate && isNumeric(operand)) {
	        // Pure immediate numeric
	        n = 0;
	        i = 1;
	    } else if (format == FORMAT_4) {
	        // Format 4 always uses simple addressing
	        n = 1;
	        i = 1;
	    } else {
	        // Format 3: set based on @ or #
	        n = (isIndirect || (!isImmediate && !isIndirect)) ? 1 : 0;
	        i = (isImmediate || (!isImmediate && !isIndirect)) ? 1 : 0;
	    }

	    // First byte: 6-bit opcode + ni flags
	    int opcode = getOpcodeValue(op) & 0xFC;
	    int byte1 = opcode | (n << 1) | i;

	    // Handle immediate numeric literals
	    if (isImmediate && isNumeric(operand)) {
	        int value = strtol(operand, NULL, 10);
	        if (format == FORMAT_4) {
	            // 4-byte encoding: opcode + nixbpe + 20-bit address
	            int byte2 = (1 << 4); // e=1, rest flags 0
	            return (byte1 << 24) | (byte2 << 16) | ((value >> 8) << 8) | (value & 0xFF);
	        } else {
	            // 3-byte encoding: opcode + nixbpe + 12-bit constant
	            return (byte1 << 16) | (value & 0xFFF);
	        }
	    }

	    // Lookup target address from symbol table
	    int targetAddr = getSymbolAddress(symbolArray, operand);

	    // Calculate displacement or address based on format
	    int disp = 0, b = 0, p = 0, e = (format == FORMAT_4 ? 1 : 0);
	    int nextPC = addresses->current + 3; // PC-relative uses PC after this instruction

	    if (format == FORMAT_4) {
	        // Use absolute 20-bit address for format 4
	        disp = targetAddr;
	    } else {
	        // Format 3: use PC or base relative addressing
	        int diff = targetAddr - nextPC;
	        if (diff >= PC_MIN_RANGE && diff <= PC_MAX_RANGE) {
	            p = 1;
	            disp = diff & 0xFFF;
	        } else {
	            b = 1;
	            disp = (targetAddr - addresses->base) & 0xFFF;
	        }
	    }

	    int x = isIndexed ? 1 : 0;
	    int flags = (x << 3) | (b << 2) | (p << 1) | e;

	    // Final object code: depends on format
	    if (format == FORMAT_4) {
	        int byte2 = (flags << 4) | ((disp >> 16) & 0x0F);
	        int byte3 = (disp >> 8) & 0xFF;
	        int byte4 = disp & 0xFF;
	        return (byte1 << 24) | (byte2 << 16) | (byte3 << 8) | byte4;
	    } else {
	        int byte2 = (flags << 4) | ((disp >> 8) & 0x0F);
	        int byte3 = disp & 0xFF;
	        return (byte1 << 16) | (byte2 << 8) | byte3;
	    }
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

// Do no modify any part of this function
// Returns a hex byte containing the registers listed in the provided operand
int getRegisters(char* operand)
{
	int registerValue = 0;
	registerValue = getRegisterValue(operand[0]) * REGISTER_MULTIPLIER;

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
	case 'A': return REGISTER_A;
	case 'B': return REGISTER_B;
	case 'L': return REGISTER_L;
	case 'S': return REGISTER_S;
	case 'T': return REGISTER_T;
	case 'X': return REGISTER_X;
	default: return -1;
	}
}

// Do no modify any part of this function
// Returns true if the provided string contains a numeric value; otherwise, false
bool isNumeric(char* string)
{
	for(int x = 0; x < strlen(string); x++)
	{
		if(!isdigit(string[x])) return false;
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

	while (fgets(line, INPUT_BUF_SIZE, file)) {
	    if (addresses->current >= 0x100000) {
	        char value[10];
	        sprintf(value, "0x%X", addresses->current);
	        displayError(OUT_OF_MEMORY, value);
	        exit(-1);
	    }

	    if (line[0] < 32) {
	        displayError(BLANK_RECORD, NULL);
	        exit(-1);
	    } else if (line[0] == COMMENT) {
	        continue;
	    }

	    segment* segments = prepareSegments(line);

	    if (isDirective(segments->label) || isOpcode(segments->label)) {
	        displayError(ILLEGAL_SYMBOL, segments->label);
	        exit(-1);
	    }

	    int dirType = isDirective(segments->operation);

	    if (isStartDirective(dirType)) {
	        addresses->start = addresses->current = strtol(segments->operand, NULL, 16);
	        continue;
	    }

	    if (dirType) {
	        addresses->increment = getMemoryAmount(dirType, segments->operand);
	    } else if (isOpcode(segments->operation)) {
	        addresses->increment = getOpcodeFormat(segments->operation);
	    } else {
	        displayError(ILLEGAL_OPCODE_DIRECTIVE, segments->operation);
	        exit(-1);
	    }

	    if (strlen(segments->label) > 0) {
	        insertSymbol(symbolTable, segments->label, addresses->current);
	    }

	    addresses->current += addresses->increment;
	    memset(line, '\0', INPUT_BUF_SIZE);
	}
	fclose(file);
}

// Performs Pass 2 of the SIC/XE assembler
void performPass2(symbol* symbolTable[], char* filename, address* addresses)
{
    int execAddr = addresses->start;

    char* lstName = createFilename(filename, ".lst");
    char* objName = createFilename(filename, ".obj");
    FILE* lst = fopen(lstName, "w");
    FILE* obj = fopen(objName, "w+");  // opened in "w+" mode for read/write

    if (!lst || !obj) {
        displayError(FILE_NOT_FOUND, filename);
        exit(-1);
    }

    objectFileData hdr = {0};
    hdr.recordType = 'H';

    objectFileData txt = {0};
    txt.recordType = 'T';

    FILE* src = fopen(filename, "r");
    char line[INPUT_BUF_SIZE];

    bool startSeen = false;
    addresses->current = addresses->start;

    // First pass to extract header values
    while (fgets(line, sizeof line, src)) {
        if (line[0] < 32 || line[0] == COMMENT) continue;

        segment* seg = prepareSegments(line);
        int dtype = isDirective(seg->operation);

        if (isStartDirective(dtype) && !startSeen) {
            startSeen = true;
            hdr.startAddress = strtol(seg->operand, NULL, 16);
            addresses->start = hdr.startAddress;
            addresses->current = hdr.startAddress;
            strncpy(hdr.programName, seg->label, NAME_SIZE - 1);
            break;
        }
    }

    // Reserve space for header record
    fseek(obj, 40, SEEK_SET); // skip room for 1 header line of about ~40 characters

    rewind(src);
    txt.recordAddress = addresses->current;

    while (fgets(line, sizeof line, src)) {
        if (line[0] < 32 || line[0] == COMMENT) continue;

        segment* seg = prepareSegments(line);
        int dtype = isDirective(seg->operation);

        if (isStartDirective(dtype)) {
            int addr = strtol(seg->operand, NULL, 16);
            addresses->start = addr;
            addresses->current = addr;
            txt.recordAddress = addr;
            writeToLstFile(lst, addresses->current, seg, 0);
            continue;
        }

        if (isEndDirective(dtype)) {
            if (seg->operand[0] != '\0') {
                execAddr = getSymbolAddress(symbolTable, seg->operand);
            }
            writeToLstFile(lst, addresses->current, seg, 0);
            continue;
        }

        if (isBaseDirective(dtype)) {
            addresses->base = getSymbolAddress(symbolTable, seg->operand);
            writeToLstFile(lst, addresses->current, seg, 0);
            continue;
        }

        if (isReserveDirective(dtype)) {
            if (txt.recordEntryCount > 0) {
                flushTextRecord(obj, &txt, addresses);
                txt.recordType = 'T';
            }
            writeToLstFile(lst, addresses->current, seg, 0);
            addresses->current += getMemoryAmount(dtype, seg->operand);
            txt.recordAddress = addresses->current; // 🟢 FIX HERE
            continue;
        }

        if (isDataDirective(dtype)) {
            int nbytes = 0, code = 0;
            if (seg->operand[0] == 'X') {
                char* start = strchr(seg->operand, '\'') + 1;
                char* end = strchr(start, '\'');
                char hex[9] = {0};
                strncpy(hex, start, end - start);
                code = strtol(hex, NULL, 16);
                nbytes = strlen(hex) / 2;
            } else if (seg->operand[0] == 'C') {
                char* start = strchr(seg->operand, '\'') + 1;
                char* end = strchr(start, '\'');
                nbytes = end - start;
                for (int i = 0; i < nbytes; i++)
                    code = (code << 8) | (unsigned char)start[i];
            } else {
                code = getByteValue(dtype, seg->operand);
                nbytes = getMemoryAmount(dtype, seg->operand);
            }

            if (txt.recordByteCount + nbytes > MAX_RECORD_BYTE_COUNT) {
                if (txt.recordEntryCount > 0) {
                    flushTextRecord(obj, &txt, addresses);
                    txt.recordType = 'T';
                    txt.recordAddress = addresses->current;
                }
            }

            txt.recordEntries[txt.recordEntryCount++] = (recordEntry){ nbytes, code };
            txt.recordByteCount += nbytes;

            writeToLstFile(lst, addresses->current, seg, code);
            addresses->current += nbytes;
            continue;
        }

        if (isOpcode(seg->operation)) {
            int fmt = getOpcodeFormat(seg->operation);
            int objCode = 0, nbytes = 0;

            if (strcmp(seg->operation, "RSUB") == 0) {
                objCode = 0x4F0000;
                nbytes = 3;
                seg->operand[0] = '\0';
            } else if (fmt == FORMAT_1) {
                objCode = getOpcodeValue(seg->operation) & 0xFF;
                nbytes = 1;
            } else if (fmt == FORMAT_2) {
                int regs = getRegisters(seg->operand) & 0xFF;
                objCode = ((getOpcodeValue(seg->operation) & 0xFF) << 8) | regs;
                nbytes = 2;
            } else {
                objCode = computeFlagsAndAddress(symbolTable, addresses, seg, (fmt == FORMAT_4 ? FORMAT_4 : FORMAT_3));
                nbytes = (fmt == FORMAT_4 ? 4 : 3);
            }

            if (txt.recordByteCount + nbytes > MAX_RECORD_BYTE_COUNT) {
                if (txt.recordEntryCount > 0) {
                    flushTextRecord(obj, &txt, addresses);
                    txt.recordType = 'T';
                    txt.recordAddress = addresses->current;
                }
            }

            txt.recordEntries[txt.recordEntryCount++] = (recordEntry){ nbytes, objCode };
            txt.recordByteCount += nbytes;

            writeToLstFile(lst, addresses->current, seg, objCode);
            addresses->current += nbytes;
            continue;
        }

        writeToLstFile(lst, addresses->current, seg, 0);
    }

    fclose(src);

    if (txt.recordEntryCount > 0) {
        flushTextRecord(obj, &txt, addresses);
    }

    hdr.programSize = addresses->current - hdr.startAddress;

    // Seek back to top and write the header now that size is known
    fseek(obj, 0, SEEK_SET);
    writeToObjFile(obj, hdr);

    objectFileData endRec = {0};
    endRec.recordType = 'E';
    endRec.startAddress = execAddr;
    fseek(obj, 0, SEEK_END);
    writeToObjFile(obj, endRec);

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
	trim(temp->label);
	trim(temp->operation);
	trim(temp->operand);
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
		fprintf(file, "%-8X%-8s%-8s%-11s\n", address, segments->label, segments->operation, segments->operand);
	}
	else if (isEndDirective(directiveType))
	{
		fprintf(file, "%-8X%-8s%-8s%-11s", address, segments->label, segments->operation, segments->operand);
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
		sprintf(ctrlString, "%%-8X%%-8s%%-8s%%-11s %%0%dX\n", length);

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
