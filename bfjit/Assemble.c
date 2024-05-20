#include "extern_data.h"
#include "Assemble.h"
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include "InstructionSet.h"
#include <assert.h>

#define MAX_CODE_SIZE 1000000


static instruction_t instructionSet[OP_CODE_END];

static size_t getSize( OpType type );
static void setupInstructionSetTable();

unsigned char* assemble( const opcode_t* code, int* size )
{
	setupInstructionSetTable();

	//Assign some memory for our machine code
	unsigned char* machineCode = malloc( MAX_CODE_SIZE );

	//Init bracketStack for recording bracket positions.
	int* bracketStack = malloc( MAX_STACK_SIZE * sizeof( int ) );
	int bracketStackIndex = 0;

	
	int codeIndex = 0;
	memcpy_s( machineCode + codeIndex, sizeof( op_header ), op_header, sizeof( op_header ) );
	codeIndex += sizeof( op_header );

	for (int i = 0; code[i].type != OP_CODE_END; i++ )
	{
		instruction_t instruction = instructionSet[code[i].type];
		//Copy the constant part of the instruction
		memcpy_s( machineCode + codeIndex, instruction.size, instruction.opcode, instruction.size );
		codeIndex += instruction.size;

		//If we have anything extra to add
		if ( instruction.argument )
		{
			if ( code[i].type == OP_CLOSE_BRACKET )
			{
				int openAddress = bracketStack[bracketStackIndex--];
				uint32_t offset;

				if ( i >= 1 && code[i - 1].type == OP_ZERO )
				{
					//Optimisation remove jump from code if there is a zero instruction before.
					//Bit hacky to have already copied in but we can just plop the codeIndex back the instruction
					//size.
					codeIndex -= instruction.size;
				}
				else
				{
					//This cell can be non-zero so jump is possible.

					offset = openAddress - codeIndex - sizeof( uint32_t ); //diff of jump backwards
					//-sizeof(uint32_t) is to get to the int address we are writing to.

					//Copy jump offset into code
					memcpy_s( machineCode + codeIndex, sizeof( uint32_t ), &offset, sizeof( uint32_t ) );
					codeIndex += sizeof( uint32_t ); //advance the code index.
				}


				//Offset now is the jump forwards to after the close bracket instruction.
				offset = codeIndex - openAddress;
				//Write jump forwards into the open bracket address space
				// - sizeof(uint32_t) is to get to the index where the int
				//is to be placed.
				memcpy_s( machineCode + openAddress - sizeof( uint32_t ), sizeof( uint32_t ), &offset, sizeof( uint32_t ) );

			}
			else if ( code[i].type == OP_OPEN_BRACKET )
			{
				codeIndex += sizeof( uint32_t ); //leave space for the address
				bracketStack[++bracketStackIndex] = codeIndex; //add the jump address to stack

				assert( bracketStackIndex <= MAX_STACK_SIZE );
			}
			else
			{
				//This is not a bracket so we can paste the value in.
				size_t size = 0;

				switch ( code[i].type )
				{
				case OP_ADD:
				case OP_SUB:
					{
						uint8_t value = code[i].value % 256;
						size = sizeof( uint8_t );
						memcpy_s( machineCode + codeIndex, size, &value, size );
					}
					break;
				case OP_ADD_PTR:
				case OP_SUB_PTR:
					{
						size = sizeof( uint32_t );
						memcpy_s( machineCode + codeIndex, size, &code[i].value, size );
					}
					break;
				}

				codeIndex += size;
			}
		}
	}

	memcpy_s( machineCode + codeIndex, sizeof( op_footer ), op_footer, sizeof( op_footer ) );
	codeIndex += sizeof( op_footer );

	*size = codeIndex;

	free( bracketStack );

	return machineCode;
}

static size_t getSize( OpType type )
{
	

	return sizeof( uint32_t );
}

void setupInstructionSetTable()
{
	static int s_instructionTableSetup = 0;

	if ( s_instructionTableSetup )
	{
		return;
	}

	setInstruction( OP_INC_PTR, op_incRBX, 0 );
	setInstruction( OP_DEC_PTR, op_decRBX, 0 );
	setInstruction( OP_ADD_PTR, op_addRBX, 1 );
	setInstruction( OP_SUB_PTR, op_subRBX, 1 );
	setInstruction( OP_INC, op_inc, 0 );
	setInstruction( OP_DEC, op_dec, 0 );
	setInstruction( OP_ADD, op_add, 1 );
	setInstruction( OP_SUB, op_sub, 1 );
	setInstruction( OP_ZERO, op_zero, 0 );
	setInstruction( OP_OUTPUT_CHAR, op_putChar, 0 );
	setInstruction( OP_INPUT_CHAR, op_getChar, 0 );
	setInstruction( OP_OPEN_BRACKET, op_openBracket, 1 );
	setInstruction( OP_CLOSE_BRACKET, op_closeBracket, 1 );

	s_instructionTableSetup = 1;
}