#include "spimcore.h"
// Dylan Roberts
// 11/2/18
/* ALU */
/* 10 Points */
void ALU(unsigned A,unsigned B,char ALUControl,unsigned *ALUresult,char *Zero)
{	
	// Set the ALU Result
	switch (ALUControl)
	{
		int C, D; // Initialize signed integers for case 6 (slti). C is for A, D is for B. 
		case 0:
			*ALUresult = A + B;
			break;
		case 1:
			*ALUresult = A - B;
			break;
		case 2:
			C = A;
			D = B;
			*ALUresult = (C < D) ? 1 : 0;
			break;
		case 3:
			*ALUresult = (A < B) ? 1 : 0;
			break;
		case 4:
			*ALUresult = (A & B); //? 1 : 0;
			break;
		case 5:
			*ALUresult = (A | B); //? 1 : 0;
			break;
		case 6:
			*ALUresult = B << 16;
			break;
		case 7: 
			*ALUresult = ~A;
			break;
	}
	// Set the Zero to appropriate value	
	if (*ALUresult == 0)
		*Zero = 1;
	else
		*Zero = 0;
}

/* instruction fetch */
/* 10 Points */
int instruction_fetch(unsigned PC,unsigned *Mem,unsigned *instruction)
{
	// Initially, Mem[1000] = first hex number. 
	if ((PC % 4) != 0) // Make sure that program counter is word-aligned (multiple of 4 bytes). 
		return 1; // halt if 0x00000000 is encountered. 
	*instruction = Mem[PC >> 2]; // fetch instruction at Mem[PC >> 2].
	if (*instruction == 0)
		return 1; // halt if 0x00000000 is encountered. 

	return 0;
}

/* instruction partition */
/* 10 Points */
void instruction_partition(unsigned instruction, unsigned *op, unsigned *r1,unsigned *r2, unsigned *r3, unsigned *funct, unsigned *offset, unsigned *jsec)
{
	int i;
	unsigned bitMask = 1;
	unsigned temp = 0;
	// bitMask is used with logical operators to assign appropriate values to fields. 
	bitMask = bitMask << 31;
	// Initialize fields. 
	*op = 0;
	*r1 = 0;
	*r2 = 0;
	*r3 = 0;
	*funct = 0;
	*offset = 0;
	*jsec = 0;
	
	for (i = 0; i < 6; i++)
	{
		*op = *op << 1;
		if ((instruction & bitMask) != 0) // if anding instruction with bitMask results in non-zero number, increment *op by 1. 
			*op += 1;
		bitMask = bitMask >> 1; // shift bitMask to the right 1 bit.
	}	
	for (i = 0; i < 5; i++)
	{
		*r1 = *r1 << 1;
		if ((instruction & bitMask) != 0) // if anding instruction with bitMask results in non-zero number, increment *r1 by 1. 
			*r1 += 1;
		bitMask = bitMask >> 1; // shift bitMask to the right 1 bit.
	}

	for (i = 0; i < 5; i++)
	{
		*r2 = *r2 << 1; // Shift *r2 left 1 bit. 
		if ((instruction & bitMask) != 0) // if anding instruction with bitMask results in non-zero number, increment *r2 by 1. 
			*r2 += 1;
		bitMask = bitMask >> 1; // shift bitMask to the right 1 bit.
	}
	
	for (i = 0; i < 5; i++)
	{
		*r3 = *r3 << 1;				
		if ((instruction & bitMask) != 0)
			*r3 += 1;
		bitMask = bitMask >> 1; // shift bitMask to the right 1 bit.
	}
	bitMask = 1; // Assign bitMask back to 1.
	bitMask = bitMask << 5;
	for (i = 0; i < 6; i++)
	{
		*funct = *funct << 1;						
		if ((instruction & bitMask) != 0)
			*funct += 1;
		bitMask = bitMask >> 1; // shift bitMask to the right 1 bit.
	}
	
	bitMask = 1; // Assign bitMask back to 1.
	bitMask = bitMask << 15;
	for (i = 0; i < 16; i++)
	{
		*offset = *offset << 1;							
		if ((instruction & bitMask) != 0)
			*offset += 1;
		bitMask = bitMask >> 1; // shift bitMask to the right 1 bit.
	}
	
	bitMask = 1; // Assign bitMask back to 1.
	bitMask = bitMask << 25;
	for (i = 0; i < 26; i++)
	{
		*jsec = *jsec << 1;									
		if ((instruction & bitMask) != 0)
			*jsec += 1;
		bitMask = bitMask >> 1; // shift bitMask to the right 1 bit.
	}	
}

/* instruction decode */
/* 15 Points */
int instruction_decode(unsigned op,struct_controls *controls)
{
	// Note, need to set all of the controls for each case. 
	switch (op)
	{
		case 0: // R Type Instruction
		{
			controls->RegDst = 1;
			controls->Jump = 0; 
			controls->Branch = 0; 
			controls->MemRead = 0; // verified
			controls->MemtoReg = 0; // should be zero since the value fed to register will come from ALU since R-Type
			controls->ALUOp = 7; // It's an R-Type instruction and the ALUOp is 111. ALU Control will use funct field
			controls->MemWrite = 0; // verified 
			controls->ALUSrc = 0; // Not sign-extend since R-Type
			controls->RegWrite = 1; // Will always write to the reg is referenced by rd field since R type
			return 0;
		}
		case 2:// 000010 - Jump
		{
			controls->RegDst = 2; //"don't care" 
			controls->Jump = 1; 
			controls->Branch = 2; //"don't care" 
			controls->MemRead = 0; 
			controls->MemtoReg = 2; //"don't care" 
			controls->ALUOp = 0; // "don't care" 
			controls->MemWrite = 0; 
			controls->ALUSrc = 2; // "don't care" 
			controls->RegWrite = 0; 
			return 0;
		}		
		case 4:// 000100 - beq
		{
			controls->RegDst = 2; //"don't care"   
			controls->Jump = 0; 
			controls->Branch = 1; 
			controls->MemRead = 0; 
			controls->MemtoReg = 2; //"don't care" 
			controls->ALUOp = 1; // subtract
			controls->MemWrite = 0; 
			controls->ALUSrc = 0; // "read reg2
			controls->RegWrite = 0; 
			return 0;
		}			
		case 8:// 001000 - add immediate
		{
			controls->RegDst = 2; // "don't care" - Might need to verify. 
			controls->Jump = 0; 
			controls->Branch = 0; 
			controls->MemRead = 0; 
			controls->MemtoReg = 0; // send ALU computed value back to reg, not from mem.
			controls->ALUOp = 0; // Addition 
			controls->MemWrite = 0; 
			controls->ALUSrc = 1; // Sign Extend
			controls->RegWrite = 1; 
			return 0;
		}	
		case 10:// 001010 - slti
		{
			controls->RegDst = 2; // "don't care" - Might need to verify. 
			controls->Jump = 0; 
			controls->Branch = 0; 
			controls->MemRead = 0; 
			controls->MemtoReg = 0; // send ALU computed value back to reg, not from mem.
			controls->ALUOp = 2; 
			controls->MemWrite = 0; 
			controls->ALUSrc = 1; // Sign Extend
			controls->RegWrite = 1; 
			return 0;
		}	
		case 11:// 001011 - sltiu. Differs from Case 10 in that the ALU op is 011 instead of 010
		{
			controls->RegDst = 2; 
			controls->Jump = 0; 
			controls->Branch = 0; 
			controls->MemRead = 0; 
			controls->MemtoReg = 0; 
			controls->ALUOp = 3; 
			controls->MemWrite = 0; 
			controls->ALUSrc = 1; // Sign Extend
			controls->RegWrite = 1; 
			return 0;
		}	
		case 15:// 001111 - lui
		{
			controls->RegDst = 0; 
			controls->Jump = 0; 
			controls->Branch = 0; 
			controls->MemRead = 0; 
			controls->MemtoReg = 0; // ?
			controls->ALUOp = 6; // shift extended value by 16 bits. 
			controls->MemWrite = 0; 
			controls->ALUSrc = 1; // needs to come from sign extend for offset. 
			controls->RegWrite = 1; 
			return 0;
		}	
		case 35:// 100011 - lw
		{
			controls->RegDst = 0;
			controls->Jump = 0; 
			controls->Branch = 0; 
			controls->MemRead = 1; 
			controls->MemtoReg = 1; 
			controls->ALUOp = 0; // add
			controls->MemWrite = 0; 
			controls->ALUSrc = 1; // needs to come from sign extend for offset
			controls->RegWrite = 1; 
			return 0;
		}		
		case 41:// 101001 - sltu unsigned, but will come from register. 
		{
			controls->RegDst = 2; // verify. value for write reg should come from ALU, not instruction. 
			controls->Jump = 0; 
			controls->Branch = 0; 
			controls->MemRead = 0; 
			controls->MemtoReg = 0; // alu to reg 
			controls->ALUOp = 3; 
			controls->MemWrite = 0; 
			controls->ALUSrc = 0; // need to read from reg2 
			controls->RegWrite = 1; // readback result to write reg. 
			return 0;
		}
		case 42:// 101010 - slt not unsigned, but will come from reg 
		{
			controls->RegDst = 2; 
			controls->Jump = 0; 
			controls->Branch = 0; 
			controls->MemRead = 0; 
			controls->MemtoReg = 0;
			controls->ALUOp = 2; 
			controls->MemWrite = 0; 
			controls->ALUSrc = 0;
			controls->RegWrite = 1;
			return 0;
		}	
		case 43:// 101011 - sw
		{
			controls->RegDst = 2; 
			controls->Jump = 0; 
			controls->Branch = 0; 
			controls->MemRead = 0; 
			controls->MemtoReg = 2;
			controls->ALUOp = 0; // add 
			controls->MemWrite = 1; 
			controls->ALUSrc = 1; // sign extend for offset
			controls->RegWrite = 0;
			return 0;
		}			
	}
	
	return 1; // halt if 0x00000000 is encountered, or no valid opCode.  
}

/* Read Register */
/* 5 Points */
void read_register(unsigned r1,unsigned r2,unsigned *Reg,unsigned *data1,unsigned *data2)
{
	// Assign Data held in registers r1 and r2 to variables *data1 and *data2. 
	*data1 = Reg[r1];
	*data2 = Reg[r2]; 
}

/* Sign Extend */
/* 10 Points */
void sign_extend(unsigned offset,unsigned *extended_value)
{
	// Check if extended_value is negative. If it is:
		// fill the 16 bits before it with 1'sbrk
	unsigned int bitMask = 1;
	unsigned const int fillInVal = 4294901760; // This constant has bits in range 16-31 set to 1, and bits 0-15 set to zero.  

	bitMask = bitMask << 6;
	if ((bitMask & offset) != 0) // if negative number.
		*extended_value = (offset | fillInVal); // assign *extended_value to the result of offset being or'd with fillInVal
	else
		*extended_value = offset; // leave bits 16-31 filled set to zero. 
}

/* ALU operations */
/* 10 Points */
int ALU_operations(unsigned data1,unsigned data2,unsigned extended_value,unsigned funct,char ALUOp,char ALUSrc,unsigned *ALUresult,char *Zero)
{
	if (ALUOp < 0)
		return 1; 
	if (ALUSrc == 1) // if ALUSrc, use sign extended value instead of value from ReadRegister2. 
		data2 = extended_value;

	if (ALUOp == 7)
	{
		switch(funct) // in this case, appropriate value will need to be passed as argument into ALU() since it will not always be 7. 
			{
				case 32: // add
					ALU(data1, data2, 0, ALUresult, Zero);
					break;
				case 34: // subtract
					ALU(data1, data2, 1, ALUresult, Zero);
					break;
				case 36: // and
					ALU(data1, data2, 4, ALUresult, Zero);
					break;
				case 37: // or
					ALU(data1, data2, 5, ALUresult, Zero);
					break;
				case 42: // set on less than
					ALU(data1, data2, 2, ALUresult, Zero);
					break;
				case 43: // set on less than unsigned
					ALU(data1, data2, 3, ALUresult, Zero);
					break;
				default:
					return 1; 
			}
		}
		else // ALUOp has been updated in instruction_decode() and can be passed into ALU()
			ALU(data1, data2, ALUOp, ALUresult, Zero);
	return 0;
}

/* Read / Write Memory */
/* 10 Points */
int rw_memory(unsigned ALUresult,unsigned data2,char MemWrite,char MemRead,unsigned *memdata,unsigned *Mem)
{		
	if (MemWrite == 1) // if I have write permission for memory.
	{
		if ((ALUresult < 0) || ((ALUresult >> 2) >= 16384)) // 16384 is (65536 / 4), the size defined in spimcore.c. Should not index greater than or equal to this. 
			return 1;

		Mem[ALUresult >> 2] = data2;
	}
	if (MemRead == 1) // if I have read permission for memory
	{
		if ((ALUresult < 0) || ((ALUresult >> 2) >= 16384)) // 16384 is (65536 / 4), the size defined in spimcore.c. Should not index greater than or equal to this. 
			return 1;
		if ((ALUresult % 4) != 0)
			return 1;	
		*memdata = Mem[ALUresult >> 2];
	}
	return 0;
}


/* Write Register */
/* 10 Points */
void write_register(unsigned r2,unsigned r3,unsigned memdata,unsigned ALUresult,char RegWrite,char RegDst,char MemtoReg,unsigned *Reg)
{
	// set read reg equal to either r2 or r3.
	// determine whether to write data from ALU result or memdata. 
	unsigned read;
	if (RegDst == 1) // Asserted means destination is from the rd field (r3)
		read = r3;
	else	// de-asserted means that destination is from the rt field (r2)
		read = r2;

	if (RegWrite == 1)
	{
		// write to register addressed by read
		if (MemtoReg == 1)// it needs to come from memory
			Reg[read] = memdata;
		else // otherwise, it needs to come from ALU
			Reg[read] = ALUresult;
	}
}

/* PC update */
/* 10 Points */
void PC_update(unsigned jsec,unsigned extended_value,char Branch,char Jump,char Zero,unsigned *PC)
{	
	if (Jump == 0) // new location of *PC will be relative to old *PC, so it will be incremented by extended_value (if nested conditions are met). 
	{
		*PC += 4; 		
		if ((Branch == 1) && (Zero == 1))
		{
			extended_value = extended_value << 2;
			*PC += extended_value;
		}
	}
	else // new location of *PC will not be relative to old *PC, so will assign to jsec after jsec gets shifted twice to the left. 
	{
		jsec = jsec << 2; // changes from 26 to 28 bits. 
		*PC = jsec;
	}
}