

typedef struct instruction_s
{
	const unsigned char* opcode;
	int size;
	int argument;
} instruction_t;



#define setInstruction(opcode_enum, opcode_array, arg) \
instructionSet[opcode_enum].opcode = opcode_array; \
instructionSet[opcode_enum].size = sizeof(opcode_array); \
instructionSet[opcode_enum].argument = arg;

//#ifndef _WIN32
const unsigned char op_header[] =
{
	
	0x57, //push rdi
	0x56, //push rsi
	0x53, //push rbx
	0x55, //push rbp
	0x48,0x89,0xe5, //mov rbp, rsp; home space?
	0x48,0x83,0xec,0x20, //sub rsp, 0x28

	0x4c,0x89,0xc3, //mov rbx, r8
	0x48,0x89,0xcf,//mov rdi, rcx
	0x48,0x89,0xd6  //mov rsi, rdx
};

const unsigned char op_incRBX[] =
{
	0x48,0xff,0xc3 //inc rbx
};

const unsigned char op_decRBX[] =
{
	0x48,0xff,0xcb //dec rbx
};

const unsigned char op_addRBX[] = //this instruction is 4 bytes larger than this size
{
	0x48,0x81,0xc3 //add rbx, x (where x is a 4 byte integer)
};

const unsigned char op_subRBX[] = //this instruction is 4 bytes larger than this size
{
	0x48,0x81,0xeb //sub rbx, x (where x is a 4 byte integer)
};

const unsigned char op_inc[] =
{
	0xfe,0x03 //inc byte [rbx]
};

const unsigned char op_dec[] =
{
	0xfe,0x0b //dec byte [rbx]
};

const unsigned char op_add[] = //this instruction is 4 bytes larger than this size
{
	0x80, 0x03 //add [rbx], byte x (where x is a 4 byte integer)
};

const unsigned char op_sub[] = //this instruction is 4 bytes larger than this size
{
	0x80, 0x2b //sub [rbx], byte x (where x is a 4 byte integer)
};

const unsigned char op_zero[] =
{
	0xc6,0x03,0x00 //mov [rbx], byte 0
};

const unsigned char op_getChar[] =
{
	0xff,0xd7, //call rdi (getchar)
	//0x3c,0x0d, //cmp al, 0x0a
	//0x75, 0x02,
	//0xb0, 0x0a,
	//0x74,0xfa, //je -6
	0x88,0x03 //mov [rbx], al
};

const unsigned char op_putChar[] =
{
	0x48,0x0f,0xb6,0x0b, //movzx rcx, byte [rbx]
	0xff,0xd6 //call rsi (putchar)
};

const unsigned char op_openBracket[] = //this instruction is 4 bytes larger than this size
{
	0x80,0x3b,0x00, //cmp [rbx], byte 0
	0x0f, 0x84 //je x (where x is a 4 byte offset)
};

#define JNE_OPERAND_SIZE 2

const unsigned char op_closeBracket[] = //this instruction is 4 bytes larger than this size
{
	0x80,0x3b,0x00, //cmp [rbx], byte 0
	0x0f,0x85 //jne x (where x is a 4 byte offset)
};

const unsigned char op_footer[] =
{
	0x48,0x83,0xc4,0x20, //add rsp, 0x28; home space?
	0x48, 0x89, 0xec, //mov rsp, rbp
	0x5d, //pop rbp
	0x5b, //pop rbx
	0x5e, //pop rsi
	0x5f, //pop rdi
	0xc3, //ret
};
//#else



//#endif