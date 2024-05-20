#include "extern_data.h"
#include "Assemble.h"
#include "Compile.h"
#include <memory.h>
#include <stdlib.h>
#include <stdio.h>
#include <Windows.h>
#include <conio.h>

#define MAX_MEMORY_SIZE 5000

static void* prepareMachineCode(void* code, int size);
static void executeMachineCode(void* code, int dump );
static void dumpMemory( unsigned char* memory );
static void dumpMachineCode( unsigned char* code, int size, const char* filename );
static void freeMachineCode( void* code );

int main(int argc, char** argv)
{
	/*if ( argc <= 1 )
	{
		fprintf( stderr, "Please supply path to file...\n" );
		return EXIT_FAILURE;
	}*/

	int dump = 0;
	int dumpCode = 1;
	const char* filename = "calc.bf";
	for ( int i = 1; i < argc; i++ )
	{
		if ( argv[i][0] != '-' )
		{
			filename = argv[i];
			continue;
		}

		dump |= (strcmp( "-dump", argv[i] ) == 0);
		dumpCode |= (strcmp( "-dump_code", argv[i] ) == 0);
	}

	opcode_t* opcodes = compile( filename );

	if ( opcodes )
	{
		int codeSize;
		unsigned char* machineCode = assemble( opcodes, &codeSize );

		if ( dumpCode )
			dumpMachineCode( machineCode, codeSize, filename );

		if ( machineCode )
		{
			void* executableCode = prepareMachineCode( machineCode, codeSize );
			free( machineCode );

			if ( executableCode )
			{
				executeMachineCode( executableCode, dump );
				freeMachineCode( executableCode );
			}
			else
			{
				fprintf( stderr, "Failed to prepare machine code.\n" );
				return EXIT_FAILURE;
			}
		}
		else
		{
			fprintf( stderr, "Failed to generate machine code.\n" );
		}
	}

	return EXIT_SUCCESS;
}

void* prepareMachineCode( void* code, int size )
{
	void* executableMemory = VirtualAlloc( NULL, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE );
	memcpy_s( executableMemory, size, code, size );
	DWORD oldProtection;
	if ( VirtualProtect( executableMemory, size, PAGE_EXECUTE_READ, &oldProtection ) )
	{
		return executableMemory;
	}
	else
	{
		freeMachineCode( executableMemory );
		return NULL;
	}
}

void executeMachineCode( void* code, int dump )
{
	void (*function)(void*, void*, void*);

	function = code;

	unsigned char* memory = malloc( MAX_MEMORY_SIZE );
	memset( memory, 0, MAX_MEMORY_SIZE );

	(*function)(getchar, putchar, memory);

	if ( dump )
	{
		dumpMemory( memory );
	}

	free( memory );
}

void freeMachineCode( void* code )
{
	VirtualFree( code, 0, MEM_RELEASE );
}

void dumpMemory( unsigned char* memory )
{
	int maxIndex = 0;
	for ( int i = 0; i < MAX_MEMORY_SIZE; i++ )
	{
		if ( memory[i] )
			maxIndex = i;
	}
	maxIndex /= 8;
	maxIndex++;

	printf( "\n" );
	for ( int i = 0; i < maxIndex; i++ )
	{
		printf( "0x%04x:", i*8);
		for ( int j = 0; j < 8; j++ )
		{
			printf( " %02x", memory[i*8+j] );
		}
		printf( " |" );
		for ( int j = 0; j < 8; j++ )
		{
			if ( isprint( memory[i * 8 + j] ) )
			{
				printf( "%c", memory[i * 8 + j] );
			}
			else
			{
				printf( "." );
			}
			
		}

		printf( "| \n" );
	}
}

void dumpMachineCode( unsigned char* code, int size, const char* filename )
{
	char newFilename[100];
	sprintf_s( newFilename, 100, "%s.bin", filename );

	FILE* file = NULL;


	fopen_s( &file, newFilename, "wb+" );

	if ( file )
	{
		fwrite( code, size, 1, file );
		fclose(file);
	}
}