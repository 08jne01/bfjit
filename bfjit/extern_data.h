#pragma once
#ifndef EXTERN_DATA_H
#define EXTERN_DATA_H
#include <stdint.h>

#define MAX_STACK_SIZE 500

typedef enum OpType
{
	OP_INC_PTR,
	OP_DEC_PTR,
	OP_ADD_PTR,
	OP_SUB_PTR,
	OP_INC,
	OP_DEC,
	OP_ADD,
	OP_SUB,
	OP_ZERO,
	OP_OUTPUT_CHAR,
	OP_INPUT_CHAR,
	OP_OPEN_BRACKET,
	OP_CLOSE_BRACKET,
	OP_CODE_END
} OpType;

typedef enum TokType
{
	TOK_OPEN_BRACKET,
	TOK_CLOSE_BRACKET,
	TOK_MINUS,
	TOK_PLUS,
	TOK_LEFT_PTR,
	TOK_RIGHT_PTR,
	TOK_OUTPUT_CHAR,
	TOK_INPUT_CHAR,
	TOK_END
} TokType;

typedef enum ErrorType
{
	ERR_NO_SOURCE,
	ERR_MISSING_CLOSE,
	ERR_MISSING_OPEN,
	ERR_NUM_ERRORS
} ErrorType;

typedef struct opcode_s
{
	OpType type;
	uint32_t value;
} opcode_t;

typedef struct token_s
{
	TokType type;
	int lineNumber;
} token_t;

typedef struct error_s
{
	ErrorType type;
	int lineNumber;
} error_t;

#endif