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

// Pass 1 functions
int getMemoryAmount(int directiveType, char* string);
int isDirective(char* string);
bool isStartDirective(int directiveType);

// Pass 2 functions
int getByteValue(int directiveType, char* string);
bool isBaseDirective(int directiveType);
bool isDataDirective(int directiveType);
bool isEndDirective(int directiveType);
bool isReserveDirective(int directiveType);