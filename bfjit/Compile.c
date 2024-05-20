#include "extern_data.h"
#include "Compile.h"
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>

#define NUMBER_OF_ERRORS_FATAL 20
#define MAX_TOKENS 1000
#define MAX_OP_CODES MAX_TOKENS

typedef enum State
{
	STATE_MULTI,
	STATE_SCAN,
	STATE_SKIP_BRACKET,
	STATE_END
} State;

static const char* s_errorString[ERR_NUM_ERRORS] =
{
	"(%d): No source loaded\n",
	"(%d): Missing ]\n",
	"(%d): ] but no matching [\n"
};

static TokType s_sameTypes[TOK_END];

static char* getSourceText( const char* filename, int* size );
static int addError( error_t* errors, int* errorsIndex, ErrorType type, int lineNumber );
static int errorIsFatal( ErrorType type );
static void printErrors( error_t* errors, int errorsIndex );

static token_t* tokenise( const char* text, int* size );
static opcode_t* generateCode( token_t* tokens, int* size, error_t* errors, int* errorsIndex, int* fatalError );

static void addMultiOpcode( opcode_t* opcodes, int* opcodeIndex, int amount, TokType type );

static int multiplicable( TokType token );
static int positive( TokType token );
static int sameType( TokType token1, TokType token2 );
static TokType getSignedType( int value, TokType type );

static void setupSameTypes();

opcode_t* compile( const char* filename )
{
	opcode_t* opcodes = NULL;

	int errorsIndex = 0;
	error_t* errors = malloc( sizeof( error_t ) * NUMBER_OF_ERRORS_FATAL );
	int fatalError = 0;

	int size;
	char* text = getSourceText( filename, &size );
	if ( text )
	{
		token_t* tokens = tokenise( text, &size );
		free( text );
		opcodes = generateCode( tokens, &size, errors, &errorsIndex, &fatalError );
		free( tokens );
	}
	else
	{
		fatalError |= addError( errors, &errorsIndex, ERR_NO_SOURCE, 0 );
	}

	if ( errorsIndex )
	{
		free( opcodes );
		printErrors( errors, errorsIndex );
	}

	
	free( errors );

	return opcodes;
}

token_t* tokenise( const char* text, int* size )
{
	token_t* tokens = malloc( ((*size) + 1) * sizeof( token_t ));
	int tokenIndex = 0;
	int lineNumber = 0;

	for ( int i = 0; text[i]; i++ )
	{
		switch ( text[i] )
		{
		case '[':
			tokens[tokenIndex].type = TOK_OPEN_BRACKET;
			tokens[tokenIndex].lineNumber = lineNumber;
			tokenIndex++;
			break;
		case ']':
			tokens[tokenIndex].type = TOK_CLOSE_BRACKET;
			tokens[tokenIndex].lineNumber = lineNumber;
			tokenIndex++;
			break;
		case '+':
			tokens[tokenIndex].type = TOK_PLUS;
			tokens[tokenIndex].lineNumber = lineNumber;
			tokenIndex++;
			break;
		case '-':
			tokens[tokenIndex].type = TOK_MINUS;
			tokens[tokenIndex].lineNumber = lineNumber;
			tokenIndex++;
			break;
		case '<':
			tokens[tokenIndex].type = TOK_LEFT_PTR;
			tokens[tokenIndex].lineNumber = lineNumber;
			tokenIndex++;
			break;
		case '>':
			tokens[tokenIndex].type = TOK_RIGHT_PTR;
			tokens[tokenIndex].lineNumber = lineNumber;
			tokenIndex++;
			break;
		case ',':
			tokens[tokenIndex].type = TOK_INPUT_CHAR;
			tokens[tokenIndex].lineNumber = lineNumber;
			tokenIndex++;
			break;
		case '.':
			tokens[tokenIndex].type = TOK_OUTPUT_CHAR;
			tokens[tokenIndex].lineNumber = lineNumber;
			tokenIndex++;
			break;
		case '\n':
			lineNumber++;
			break;
		}
	}

	tokens[tokenIndex++].type = TOK_END;

	//Shrink Memory Footprint
	token_t* correctSize = realloc( tokens, tokenIndex * sizeof( token_t ) );
	if ( correctSize )
	{
		tokens = correctSize;
	}

	*size = tokenIndex;
	
	return tokens;
}

opcode_t* generateCode( token_t* tokens, int* size, error_t* errors, int* errorsIndex, int* fatalError )
{
	setupSameTypes();

	//Classic State Machine
	State state = STATE_SCAN; //State of our machine
	int index = 0; //Index of the tokens

	int bracketIndex = 0;
	token_t* bracketStack = malloc( sizeof( token_t ) * MAX_STACK_SIZE );

	int multiTokCount; //Count of multiplicable tokens we have so far.
	TokType multiTokType; //Type of the multiplicable token we have.

	int codeStarted = 0; //Have we started recording meaningful code.

	int opcodesIndex = 0;
	//Size + 1 to include end op code.
	opcode_t* opcodes = malloc(((*size) + 1) * sizeof(opcode_t));

	while ( ! (*fatalError) && state != STATE_END )
	{
		switch ( state )
		{
		case STATE_MULTI:
			codeStarted = 1;
			if ( sameType( multiTokType, tokens[index].type ) )
			{
				multiTokCount += positive( tokens[index].type );
				index++;
			}
			else
			{
				addMultiOpcode( opcodes, &opcodesIndex, multiTokCount, multiTokType );
				state = STATE_SCAN;
			}
			break;
		case STATE_SCAN:
			if ( (*size - 1) == index )
			{
				state = STATE_END;
			}
			else if ( multiplicable( tokens[index].type ) )
			{
				multiTokCount = 0;
				multiTokType = tokens[index].type;
				state = STATE_MULTI;
			}
			else if ( tokens[index].type == TOK_OPEN_BRACKET )
			{
				if ( ! codeStarted )
				{
					state = STATE_SKIP_BRACKET;
				}
				else if ( index+2 < (*size - 1) && 
					(tokens[index+1].type == TOK_PLUS || tokens[index + 1].type == TOK_MINUS) &&
					tokens[index+2].type == TOK_CLOSE_BRACKET)
				{
					opcodes[opcodesIndex++].type = OP_ZERO;
					index += 3;
				}
				else
				{
					bracketStack[++bracketIndex] = tokens[index];
					opcodes[opcodesIndex++].type = OP_OPEN_BRACKET;
					index++;
				}
			}
			else if ( tokens[index].type == TOK_CLOSE_BRACKET )
			{
				if ( bracketIndex > 0 )
				{
					bracketIndex--;
					opcodes[opcodesIndex++].type = OP_CLOSE_BRACKET;
					index++;
				}
				else
				{
					*fatalError |= addError( errors, errorsIndex, ERR_MISSING_OPEN, tokens[index].lineNumber );
				}
			}
			else if ( tokens[index].type == TOK_OUTPUT_CHAR )
			{
				opcodes[opcodesIndex++].type = OP_OUTPUT_CHAR;
				index++;
			}
			else if ( tokens[index].type == TOK_INPUT_CHAR )
			{
				opcodes[opcodesIndex++].type = OP_INPUT_CHAR;
				index++;
			}
			break;
		case STATE_SKIP_BRACKET:
			if ( tokens[index].type == TOK_CLOSE_BRACKET )
			{
				state = STATE_SCAN;
			}
			index++;
			if ( index >= (*size - 1) )
			{
				*fatalError |= addError( errors, errorsIndex, ERR_MISSING_CLOSE, tokens[--index].lineNumber );
				state = STATE_END;
			}
			break;
		}
	}

	if ( bracketIndex != 0 )
	{
		for ( int i = 1; i <= bracketIndex && ! (*fatalError); i++ )
		{
			*fatalError |= addError( errors, errorsIndex, ERR_MISSING_CLOSE, bracketStack[i].lineNumber );
		}
	}
	free( bracketStack );

	opcodes[opcodesIndex++].type = OP_CODE_END;

	//Set size of the opcode array;
	*size = opcodesIndex;

	opcode_t* correctSize = realloc( opcodes, opcodesIndex * sizeof( opcode_t ) );
	if ( correctSize )
	{
		opcodes = correctSize;
	}

	if ( *errorsIndex != 0 )
	{
		free( opcodes );
		opcodes = NULL;
	}

	return opcodes;
}

void addMultiOpcode( opcode_t* opcodes, int* opcodeIndex, int amount, TokType type )
{
	TokType signedType = getSignedType( amount, type );
	if ( abs( amount ) == 1 )
	{
		switch ( signedType )
		{
		case TOK_LEFT_PTR:
			opcodes[*opcodeIndex].type = OP_DEC_PTR;
			break;
		case TOK_RIGHT_PTR:
			opcodes[*opcodeIndex].type = OP_INC_PTR;
			break;
		case TOK_PLUS:
			opcodes[*opcodeIndex].type = OP_INC;
			break;
		case TOK_MINUS:
			opcodes[*opcodeIndex].type = OP_DEC;
			break;

		}

		opcodes[*opcodeIndex].value = 0;
		(*opcodeIndex)++;
	}
	else
	{
		switch ( signedType )
		{
		case TOK_LEFT_PTR:
			/*for ( int i = 0; i < abs( amount ); i++ )
			{
				opcodes[*opcodeIndex].type = OP_DEC_PTR;
				opcodes[*opcodeIndex].value = 0;
				(*opcodeIndex)++;
			}*/
			opcodes[*opcodeIndex].type = OP_SUB_PTR;
			break;
		case TOK_RIGHT_PTR:
			/*for ( int i = 0; i < abs( amount ); i++ )
			{
				opcodes[*opcodeIndex].type = OP_INC_PTR;
				opcodes[*opcodeIndex].value = 0;
				(*opcodeIndex)++;
			}*/
			opcodes[*opcodeIndex].type = OP_ADD_PTR;
			break;
		case TOK_PLUS:
			/*for ( int i = 0; i < abs( amount ); i++ )
			{
				opcodes[*opcodeIndex].type = OP_INC;
				opcodes[*opcodeIndex].value = 0;
				(*opcodeIndex)++;
			}*/
			opcodes[*opcodeIndex].type = OP_ADD;
			break;
		case TOK_MINUS:
			/*for ( int i = 0; i < abs( amount ); i++ )
			{
				opcodes[*opcodeIndex].type = OP_DEC;
				opcodes[*opcodeIndex].value = 0;
				(*opcodeIndex)++;
			}*/
			opcodes[*opcodeIndex].type = OP_SUB;
			break;
		}
		opcodes[*opcodeIndex].value = abs(amount);
		(*opcodeIndex)++;
	}
}

char* getSourceText( const char* filename, int* fileLength )
{
	char* text = NULL;

	FILE* file = NULL;
	fopen_s( &file, filename, "rb" );

	if ( file )
	{
		fseek( file, 0, SEEK_END );
		size_t size = ftell( file );
		fseek( file, 0, SEEK_SET );

		text = (char*)malloc( size + 1 );
		fread_s( text, size, 1, size, file );
		text[size] = 0;
		*fileLength = size;
		fclose( file );
	}
	else
	{
		fprintf( stderr, "Failed to open source file: %s\n", filename );
	}

	return text;
}

int addError( error_t* errors, int* errorsIndex, ErrorType type, int lineNumber )
{
	error_t error;
	error.type = type;
	error.lineNumber = lineNumber;

	errors[(*errorsIndex)++] = error;

	return errorIsFatal( type ) || *errorsIndex == NUMBER_OF_ERRORS_FATAL;
}

int errorIsFatal( ErrorType type )
{
	return type == ERR_NO_SOURCE;
}

void printErrors( error_t* errors, int errorsIndex )
{
	for ( int i = 0; i < errorsIndex; i++ )
	{
		if ( errorIsFatal( errors[i].type ) )
		{
			fprintf( stderr, "FATAL ERROR\n" );
			fprintf( stderr, s_errorString[errors[i].type], errors[i].lineNumber );
		}
		else
		{
			fprintf( stderr, s_errorString[errors[i].type], errors[i].lineNumber );
		}
	}

	if ( errorsIndex >= NUMBER_OF_ERRORS_FATAL )
	{
		fprintf( stderr, "%d errors, considered fatal...", errorsIndex );
	}
	
}

int multiplicable( TokType token )
{
	return token == TOK_LEFT_PTR || token == TOK_RIGHT_PTR || token == TOK_PLUS || token == TOK_MINUS;
}

int positive( TokType token )
{
	int ret = 0;
	if ( token == TOK_LEFT_PTR || token == TOK_MINUS )
	{
		ret = -1;
	}
	else if ( token == TOK_RIGHT_PTR || token == TOK_PLUS )
	{
		ret = 1;
	}
	return ret;
}

int sameType( TokType token1, TokType token2 )
{//This can potentially access crap memory, so make sure only the comparible types
 //call this function.

	return token1 == token2 || s_sameTypes[token1] == token2;
}

TokType getSignedType( int value, TokType type )
{
	if ( value > 0 )
	{
		return type > s_sameTypes[type] ? type : s_sameTypes[type];
	}
	else
	{
		return type < s_sameTypes[type] ? type : s_sameTypes[type];
	}
}

void setupSameTypes()
{
	s_sameTypes[TOK_LEFT_PTR] = TOK_RIGHT_PTR;
	s_sameTypes[TOK_RIGHT_PTR] = TOK_LEFT_PTR;

	s_sameTypes[TOK_PLUS] = TOK_MINUS;
	s_sameTypes[TOK_MINUS] = TOK_PLUS;

	s_sameTypes[TOK_OPEN_BRACKET] = TOK_OPEN_BRACKET;
	s_sameTypes[TOK_CLOSE_BRACKET] = TOK_CLOSE_BRACKET;

	s_sameTypes[TOK_OUTPUT_CHAR] = TOK_OUTPUT_CHAR;
	s_sameTypes[TOK_INPUT_CHAR] = TOK_INPUT_CHAR;

	s_sameTypes[TOK_END] = TOK_END;
}
