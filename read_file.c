#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

// Register Names to Register Bank Index
#define REG_F 0
#define REG_C 1
#define REG_E 2
#define REG_L 3
#define REG_A 4
#define REG_B 5
#define REG_D 6
#define REG_H 7
#define REG_PC 8
#define REG_SP 9
#define REG_AF 10
#define REG_BC 11
#define REG_DE 12
#define REG_HL 13

int debug = 0;

// Reads entire cartidge file into cart_buffer
void readCart(uint8_t** cart_buffer){
  FILE * pFile;
  long lSize;
  size_t result;

  pFile = fopen ( "Tetris.gb" , "rb" );
  if (pFile==NULL) {fputs ("File error",stderr); exit (1);}

  // obtain file size:
  fseek (pFile , 0 , SEEK_END);
  lSize = ftell (pFile);
  rewind (pFile);

  // allocate memory to contain the whole file:
  *cart_buffer = (uint8_t*) malloc (sizeof(uint8_t)*lSize);
  if (cart_buffer == NULL) {fputs ("Memory error",stderr); exit (2);}

  // copy the file into the buffer:
  result = fread (*cart_buffer,1,lSize,pFile);
  if (result != lSize) {fputs ("Reading error",stderr); exit (3);}

   // terminate
  fclose (pFile);
}

void read_data_from_cart(uint8_t** cart_buffer, uint8_t** variable, unsigned int location, unsigned int size){
	for(int i=0; i<size; i++){
		(*variable)[i] = (*cart_buffer)[i+location];
	}
}


struct cart_metadata {
	uint8_t* entry_point; 	// Often NOP JP 0x150
	uint8_t* logo; 		    // Logo data
	uint8_t* title; 				// The Title
	uint8_t* cgb;						// Gameboy Color?
	// New Licencsee Code? Not sure what this is 0x144-0x145
	bool sgb;						// Super Gameboy
	uint8_t* cartidge_type;
	uint8_t* rom_size;
	uint8_t* ram_size;
	uint8_t* destination_code;
	uint8_t* old_licensee_code;
	uint8_t* mask_rom_version;
	uint8_t* header_checksum;
	uint8_t* global_checksum;
};

void read_cart_metadata(uint8_t** cart_buffer, struct cart_metadata *metadata){

	// Allocate Memory in the passed struct and then assign it data
	metadata->entry_point = (uint8_t*) malloc (sizeof(uint8_t)*4);
	read_data_from_cart(cart_buffer, &metadata->entry_point, 0x100, 4);

	metadata->logo = (uint8_t*) malloc (sizeof(uint8_t)*48);
	read_data_from_cart(cart_buffer, &metadata->logo, 0x104, 48);

	metadata->title = (uint8_t*) malloc (sizeof(uint8_t)*15);
	read_data_from_cart(cart_buffer, &metadata->title, 0x134, 15);

	metadata->cgb= (uint8_t*) malloc (sizeof(uint8_t)*1);
	read_data_from_cart(cart_buffer, &metadata->cgb, 0x143,1);

	//New Licensee Code could go here

	metadata->cartidge_type = (uint8_t*) malloc (sizeof(uint8_t)*1);
	read_data_from_cart(cart_buffer, &metadata->cartidge_type, 0x147, 1);

	metadata->rom_size = (uint8_t*) malloc (sizeof(uint8_t)*1);
	read_data_from_cart(cart_buffer, &metadata->rom_size, 0x148, 1);

	metadata->ram_size = (uint8_t*) malloc (sizeof(uint8_t)*1);
	read_data_from_cart(cart_buffer, &metadata->ram_size, 0x149, 1);

	metadata->destination_code = (uint8_t*) malloc (sizeof(uint8_t)*1);
	read_data_from_cart(cart_buffer, &metadata->destination_code, 0x14A, 1);

	metadata->old_licensee_code = (uint8_t*) malloc (sizeof(uint8_t)*1);
	read_data_from_cart(cart_buffer, &metadata->old_licensee_code,0x14B,1);

	metadata->mask_rom_version = (uint8_t*) malloc (sizeof(uint8_t)*2);
	read_data_from_cart(cart_buffer, &metadata->mask_rom_version,0x14C,1);

	metadata->header_checksum = (uint8_t*) malloc (sizeof(uint8_t)*2);
	read_data_from_cart(cart_buffer, &metadata->header_checksum,0x14D,1);

	metadata->global_checksum = (uint8_t*) malloc (sizeof(uint8_t)*2);
	read_data_from_cart(cart_buffer, &metadata->global_checksum,0x14E,2);
}

void printMetadata(struct cart_metadata *metadata){
	printf("Entry Point 0x%X%X\n",(unsigned int)metadata->entry_point[3],(unsigned int)metadata->entry_point[2]);

	printf("Game Title: ");
	int index=0;
	while(metadata->title[index] != 0x00){
		printf("%c",metadata->title[index]);
		index++;
	}
	printf("\n");

	printf("Color? ");
	if(*metadata->cgb == 0x00){
		printf("No\n");
	}else if(*metadata->cgb == 0x80){
		printf("Supported\n");
	}else if(*metadata->cgb == 0xC0){
		printf("Required\n");
	}

	printf("More metadata available\n");
}

/*
	Memory Idenifiers:
	11 : Interrupt Enable Register
	10 : Internal High RAM
	09 : I/O Ports
	08 : Empty
	07 : Sprite Atttribute Table
	06 : ECHO RAM
	05 : 4KB Work RAM bank 1~N
	04 : 4KB Work RAM Bank 0
	03 : 8kb External Ram (In Cartridge, if any)
	02 : 8kb Video Ram
	01 : 16KB ROM BANK 01~NN
	00 : 16KB ROM BANK 00
*/
struct mem_sel{
	int mem_id;
	int mem_offset;
};

struct mem_sel get_mem_info(unsigned int address){
	struct mem_sel return_data;

	if(address < 0x4000){
		//00 : 16KB ROM BANK 00
		return_data.mem_id = 0;
		return_data.mem_offset = address;
	}else if(address < 0x8000){
		//01 : 16KB ROM BANK 01~NN
		return_data.mem_id = 1;
		return_data.mem_offset = address-0x4000;
	}else if(address < 0xA000){
		//02 : 8kb Video Ram
		return_data.mem_id = 2;
		return_data.mem_offset = address-0x8000;
	}else if(address < 0xC000){
		//03 : 8kb External Ram (In Cartridge, if any)
		return_data.mem_id = 3;
		return_data.mem_offset = address-0xA000;
	}else if(address < 0xD000){
		//04 : 4KB Work RAM Bank 0
		return_data.mem_id = 4;
		return_data.mem_offset = address-0xC000;
	}else if(address < 0xE000){
		//05 : 4KB Work RAM bank 1~N
		return_data.mem_id = 5;
		return_data.mem_offset = address-0xD000;
	}else if(address < 0xFE00){
		//06 : ECHO RAM
		return_data.mem_id = 6;
		return_data.mem_offset = address-0xE000;
	}else if(address < 0xFEA0){
		//07 : Sprite Attribute Table
		return_data.mem_id = 7;
		return_data.mem_offset = address-0xFE00;
	}else if(address < 0xFF00){
		//08 : Not Useable
		return_data.mem_id = 8;
		return_data.mem_offset = address-0xFEA0;
	}else if(address < 0xFF80){
		//09 : I/O Ports
		return_data.mem_id = 9;
		return_data.mem_offset = address-0xFF00;
	}else if(address < 0xFFFF){
		//10 : High RAM
		return_data.mem_id = 10;
		return_data.mem_offset = address-0xFF80;
	}else if(address == 0xFFFF){
		return_data.mem_id = 11;
		return_data.mem_offset = 0;
	}else{
		return_data.mem_id = -1;
		return_data.mem_id = 0;
	}


	if(debug>0){
		printf("Accessed Memory with ID:%i and offset:%i\n",return_data.mem_id,return_data.mem_offset);
	}
}


/*
// Register Names to Register Bank Index
#define REG_F 0
#define REG_C 1
#define REG_E 2
#define REG_L 3
#define REG_A 4
#define REG_B 5
#define REG_D 6
#define REG_H 7
#define REG_PC 8
#define REG_SP 9
#define REG_AF 10
#define REG_BC 11
#define REG_DE 12
#define REG_HL 13
*/


struct registerBank{
	// They are all 16-bits
	// 8-bit registers are read and write with bitshifts
	uint16_t reg_list[6];
};

void initRegisters(struct registerBank *bank){
	for(int i=0; i<6; i++){
		(*bank).reg_list[i] = (uint16_t)0;
	}
}

uint8_t readRegister8(struct registerBank *bank,int regID){
   switch(regID) {
   	  //F
      case 0 :
         printf("Reading %i \n",(*bank).reg_list[10-8]);
         return (*bank).reg_list[10-8]&0xFF;
      //C
      case 1 :
      	return (*bank).reg_list[11-8]&0xFF;
      //E
      case 2 :
         return (*bank).reg_list[12-8]&0xFF;
      //L
      case 3 :
         return (*bank).reg_list[13-8]&0xFF;
      //A
      case 4 :
         return ((*bank).reg_list[10-8]&0xFF00)>>8;
      //B
      case 5 :
         return ((*bank).reg_list[11-8]&0xFF00)>>8;
      //D
      case 6 :
      	return ((*bank).reg_list[12-8]&0xFF00)>>8;
      //H
      case 7 :
         return ((*bank).reg_list[13-8]&0xFF00)>>8;
      default :
         printf("Error, Reading 16-bit register but returing 8\n");
         return ((*bank).reg_list[regID-8]&0xFF);
   }
	//Normal Operation
	return (*bank).reg_list[regID];
}

uint16_t readRegister16(struct registerBank *bank,int regID){
	if(regID<8){
		printf("Error: Trying To Read 16-bits from 8-bit function\n");
	}
	return (*bank).reg_list[regID-8];
}

void writeRegister16(struct registerBank *bank, int regID, uint16_t data){
	(*bank).reg_list[regID-8] = data;
}

void writeRegister8(struct registerBank *bank, int regID, uint8_t data){
	if(regID<8){

		// 8-bit Register Write
	   switch(regID) {
	   	  //F
	      case 0 :
	         (*bank).reg_list[10-8] = ((*bank).reg_list[10-8]&0xFF00)|data;
	         printf("%i %i %i\n",(*bank).reg_list[10-8],data,(*bank).reg_list[10-8]|data);
	         break;
	      //C
	      case 1 :
	      	(*bank).reg_list[11-8] = ((*bank).reg_list[11-8]&0xFF00)|data;
	      	break;
	      //E
	      case 2 :
	         (*bank).reg_list[12-8] = ((*bank).reg_list[12-8]&0xFF00)|data;
	         break;
	      //L
	      case 3 :
	         (*bank).reg_list[13-8] = ((*bank).reg_list[13-8]&0xFF00)|data;
	         break;
	      //A
	      case 4 :
	         (*bank).reg_list[10-8] = ((*bank).reg_list[10-8]&0x00FF)|(data<<8);
	         break;
	      //B
	      case 5 :
	         (*bank).reg_list[11-8] = ((*bank).reg_list[11-8]&0x00FF)|(data<<8);
	         break;
	      //D
	      case 6 :
	      	(*bank).reg_list[12-8] = ((*bank).reg_list[12-8]&0x00FF)|(data<<8);
	      	break;
	      //H
	      case 7 :
	         (*bank).reg_list[13-8] = ((*bank).reg_list[13-8]&0x00FF)|(data<<8);
	         break;
	    }

	}else{
		// 16-bit Register Write
		printf("16-bit write to\n");
		(*bank).reg_list[regID-8] = data;
	}
}

void testRegisters(struct registerBank *bank){
	// Testing Registers (They work)
	for(uint8_t i=0; i<8; i++){
		writeRegister8(bank, i, i*2);
	}
	for(uint8_t i=0; i<8; i++){
		printf("%i\n",readRegister8(bank, i));
	}
	printf("---------------------\n");
	// Testing Registers (They work)
	for(uint8_t i=8; i<14; i++){
		writeRegister16(bank, i, i*2);
	}
	for(uint8_t i=8; i<14; i++){
		printf("%i\n",readRegister16(bank, i));
	}
}

struct Operand{
	int regID;
	uint8_t bytes;
	bool immediate;
};

struct Operand* new_operand(int regID, uint8_t bytes, bool immediate) { 
  struct Operand* p = malloc(sizeof(struct Operand));
  p->regID = regID;
  p->bytes = bytes;
  p->immediate = immediate;
  return p;
}

const char *mnemonics[45] = {"NOP", "LD", "INC", "DEC", "RLCA", "ADD", "RRCA", "STOP", "RLA", "JR", "RRA","DAA", "CPL", "SCF", "CCF", "HALT", "ADC","SUB","SBC","AND","XOR","OR","CP","RET","POP","JP","CALL","PUSH","PREFIX","ILLEGAL_D3","RETI","ILLEGAL_BD","ILLEGAL_DD","LDH","ILLEGAL_E3","ILLEGAL_E4","ILLEGAL_EB","ILLEGAL_EC","ILLEGAL_ED","DI","ILLEGAL_F4","EI","ILLEGAL_FC","ILLEGAL_FD"};
struct Instruction{
	uint8_t opcode;
	uint8_t mne_ID;
	uint8_t bytes;
	uint8_t cycles;
	struct Operand *operand0;
	struct Operand *operand1;
	bool immediate;
	uint8_t flagset; // Packed into highest 4-bits
	uint8_t flagreset; // Packed into highest 4-bits
};
struct Instruction* new_inst(uint8_t opcode, uint8_t mne_ID, uint8_t bytes, uint8_t cycles, struct Operand* operand0, struct Operand* operand1, bool immediate, uint8_t flagset,uint8_t flagreset){
	struct Instruction* p = malloc(sizeof(struct Instruction));
	p->opcode = opcode;
	p->mne_ID = mne_ID;
	p->bytes = bytes;
	p->cycles = cycles;
	p->operand0 = operand0;
	p->operand1 = operand1;
	p->immediate = immediate;
	p->flagset = flagset;
	p->flagreset = flagreset;
	return p;
}

void populate_instructions(struct Instruction **unprefixed_list){
	// 99's need to be fixed with appropiate constants in Python
	(*unprefixed_list)[0] = *new_inst(0x00,0,1,4,NULL,NULL,true,0b0,0b0);
	(*unprefixed_list)[1] = *new_inst(0x01,1,3,12,new_operand(11,0,true),new_operand(14,2,true),true,0b0,0b0);
	(*unprefixed_list)[2] = *new_inst(0x02,1,1,8,new_operand(11,0,false),new_operand(4,0,true),false,0b0,0b0);
	(*unprefixed_list)[3] = *new_inst(0x03,2,1,8,new_operand(11,0,true),NULL,true,0b0,0b0);
	(*unprefixed_list)[4] = *new_inst(0x04,2,1,4,new_operand(5,0,true),NULL,true,0b0,0b1000000);
	(*unprefixed_list)[5] = *new_inst(0x05,3,1,4,new_operand(5,0,true),NULL,true,0b1000000,0b0);
	(*unprefixed_list)[6] = *new_inst(0x06,1,2,8,new_operand(5,0,true),new_operand(14,1,true),true,0b0,0b0);
	(*unprefixed_list)[7] = *new_inst(0x07,4,1,4,NULL,NULL,true,0b0,0b11100000);
	(*unprefixed_list)[8] = *new_inst(0x08,1,3,20,new_operand(99,2,false),new_operand(9,0,true),false,0b0,0b0);
	(*unprefixed_list)[9] = *new_inst(0x09,5,1,8,new_operand(13,0,true),new_operand(11,0,true),true,0b0,0b1000000);
	(*unprefixed_list)[10] = *new_inst(0x0A,1,1,8,new_operand(4,0,true),new_operand(11,0,false),false,0b0,0b0);
	(*unprefixed_list)[11] = *new_inst(0x0B,3,1,8,new_operand(11,0,true),NULL,true,0b0,0b0);
	(*unprefixed_list)[12] = *new_inst(0x0C,2,1,4,new_operand(1,0,true),NULL,true,0b0,0b1000000);
	(*unprefixed_list)[13] = *new_inst(0x0D,3,1,4,new_operand(1,0,true),NULL,true,0b1000000,0b0);
	(*unprefixed_list)[14] = *new_inst(0x0E,1,2,8,new_operand(1,0,true),new_operand(14,1,true),true,0b0,0b0);
	(*unprefixed_list)[15] = *new_inst(0x0F,6,1,4,NULL,NULL,true,0b0,0b11100000);
	(*unprefixed_list)[16] = *new_inst(0x10,7,2,4,new_operand(99,1,true),NULL,true,0b0,0b0);
	(*unprefixed_list)[17] = *new_inst(0x11,1,3,12,new_operand(12,0,true),new_operand(14,2,true),true,0b0,0b0);
	(*unprefixed_list)[18] = *new_inst(0x12,1,1,8,new_operand(12,0,false),new_operand(4,0,true),false,0b0,0b0);
	(*unprefixed_list)[19] = *new_inst(0x13,2,1,8,new_operand(12,0,true),NULL,true,0b0,0b0);
	(*unprefixed_list)[20] = *new_inst(0x14,2,1,4,new_operand(6,0,true),NULL,true,0b0,0b1000000);
	(*unprefixed_list)[21] = *new_inst(0x15,3,1,4,new_operand(6,0,true),NULL,true,0b1000000,0b0);
	(*unprefixed_list)[22] = *new_inst(0x16,1,2,8,new_operand(6,0,true),new_operand(14,1,true),true,0b0,0b0);
	(*unprefixed_list)[23] = *new_inst(0x17,8,1,4,NULL,NULL,true,0b0,0b11100000);
	(*unprefixed_list)[24] = *new_inst(0x18,9,2,12,new_operand(99,1,true),NULL,true,0b0,0b0);
	(*unprefixed_list)[25] = *new_inst(0x19,5,1,8,new_operand(13,0,true),new_operand(12,0,true),true,0b0,0b1000000);
	(*unprefixed_list)[26] = *new_inst(0x1A,1,1,8,new_operand(4,0,true),new_operand(12,0,false),false,0b0,0b0);
	(*unprefixed_list)[27] = *new_inst(0x1B,3,1,8,new_operand(12,0,true),NULL,true,0b0,0b0);
	(*unprefixed_list)[28] = *new_inst(0x1C,2,1,4,new_operand(2,0,true),NULL,true,0b0,0b1000000);
	(*unprefixed_list)[29] = *new_inst(0x1D,3,1,4,new_operand(2,0,true),NULL,true,0b1000000,0b0);
	(*unprefixed_list)[30] = *new_inst(0x1E,1,2,8,new_operand(2,0,true),new_operand(14,1,true),true,0b0,0b0);
	(*unprefixed_list)[31] = *new_inst(0x1F,10,1,4,NULL,NULL,true,0b0,0b11100000);
	(*unprefixed_list)[32] = *new_inst(0x20,9,2,12,new_operand(99,0,true),new_operand(14,1,true),true,0b0,0b0);
	(*unprefixed_list)[33] = *new_inst(0x21,1,3,12,new_operand(13,0,true),new_operand(14,2,true),true,0b0,0b0);
	(*unprefixed_list)[34] = *new_inst(0x22,1,1,8,new_operand(13,0,false),new_operand(4,0,true),false,0b0,0b0);
	(*unprefixed_list)[35] = *new_inst(0x23,2,1,8,new_operand(13,0,true),NULL,true,0b0,0b0);
	(*unprefixed_list)[36] = *new_inst(0x24,2,1,4,new_operand(7,0,true),NULL,true,0b0,0b1000000);
	(*unprefixed_list)[37] = *new_inst(0x25,3,1,4,new_operand(7,0,true),NULL,true,0b1000000,0b0);
	(*unprefixed_list)[38] = *new_inst(0x26,1,2,8,new_operand(7,0,true),new_operand(14,1,true),true,0b0,0b0);
	(*unprefixed_list)[39] = *new_inst(0x27,11,1,4,NULL,NULL,true,0b0,0b100000);
	(*unprefixed_list)[40] = *new_inst(0x28,9,2,12,new_operand(99,0,true),new_operand(14,1,true),true,0b0,0b0);
	(*unprefixed_list)[41] = *new_inst(0x29,5,1,8,new_operand(13,0,true),new_operand(13,0,true),true,0b0,0b1000000);
	(*unprefixed_list)[42] = *new_inst(0x2A,1,1,8,new_operand(4,0,true),new_operand(13,0,false),false,0b0,0b0);
	(*unprefixed_list)[43] = *new_inst(0x2B,3,1,8,new_operand(13,0,true),NULL,true,0b0,0b0);
	(*unprefixed_list)[44] = *new_inst(0x2C,2,1,4,new_operand(3,0,true),NULL,true,0b0,0b1000000);
	(*unprefixed_list)[45] = *new_inst(0x2D,3,1,4,new_operand(3,0,true),NULL,true,0b1000000,0b0);
	(*unprefixed_list)[46] = *new_inst(0x2E,1,2,8,new_operand(3,0,true),new_operand(14,1,true),true,0b0,0b0);
	(*unprefixed_list)[47] = *new_inst(0x2F,12,1,4,NULL,NULL,true,0b1100000,0b0);
	(*unprefixed_list)[48] = *new_inst(0x30,9,2,12,new_operand(99,0,true),new_operand(14,1,true),true,0b0,0b0);
	(*unprefixed_list)[49] = *new_inst(0x31,1,3,12,new_operand(9,0,true),new_operand(14,2,true),true,0b0,0b0);
	(*unprefixed_list)[50] = *new_inst(0x32,1,1,8,new_operand(13,0,false),new_operand(4,0,true),false,0b0,0b0);
	(*unprefixed_list)[51] = *new_inst(0x33,2,1,8,new_operand(9,0,true),NULL,true,0b0,0b0);
	(*unprefixed_list)[52] = *new_inst(0x34,2,1,12,new_operand(13,0,false),NULL,false,0b0,0b1000000);
	(*unprefixed_list)[53] = *new_inst(0x35,3,1,12,new_operand(13,0,false),NULL,false,0b1000000,0b0);
	(*unprefixed_list)[54] = *new_inst(0x36,1,2,12,new_operand(13,0,false),new_operand(14,1,true),false,0b0,0b0);
	(*unprefixed_list)[55] = *new_inst(0x37,13,1,4,NULL,NULL,true,0b10000,0b1100000);
	(*unprefixed_list)[56] = *new_inst(0x38,9,2,12,new_operand(1,0,true),new_operand(14,1,true),true,0b0,0b0);
	(*unprefixed_list)[57] = *new_inst(0x39,5,1,8,new_operand(13,0,true),new_operand(9,0,true),true,0b0,0b1000000);
	(*unprefixed_list)[58] = *new_inst(0x3A,1,1,8,new_operand(4,0,true),new_operand(13,0,false),false,0b0,0b0);
	(*unprefixed_list)[59] = *new_inst(0x3B,3,1,8,new_operand(9,0,true),NULL,true,0b0,0b0);
	(*unprefixed_list)[60] = *new_inst(0x3C,2,1,4,new_operand(4,0,true),NULL,true,0b0,0b1000000);
	(*unprefixed_list)[61] = *new_inst(0x3D,3,1,4,new_operand(4,0,true),NULL,true,0b1000000,0b0);
	(*unprefixed_list)[62] = *new_inst(0x3E,1,2,8,new_operand(4,0,true),new_operand(14,1,true),true,0b0,0b0);
	(*unprefixed_list)[63] = *new_inst(0x3F,14,1,4,NULL,NULL,true,0b0,0b1100000);
	(*unprefixed_list)[64] = *new_inst(0x40,1,1,4,new_operand(5,0,true),new_operand(5,0,true),true,0b0,0b0);
	(*unprefixed_list)[65] = *new_inst(0x41,1,1,4,new_operand(5,0,true),new_operand(1,0,true),true,0b0,0b0);
	(*unprefixed_list)[66] = *new_inst(0x42,1,1,4,new_operand(5,0,true),new_operand(6,0,true),true,0b0,0b0);
	(*unprefixed_list)[67] = *new_inst(0x43,1,1,4,new_operand(5,0,true),new_operand(2,0,true),true,0b0,0b0);
	(*unprefixed_list)[68] = *new_inst(0x44,1,1,4,new_operand(5,0,true),new_operand(7,0,true),true,0b0,0b0);
	(*unprefixed_list)[69] = *new_inst(0x45,1,1,4,new_operand(5,0,true),new_operand(3,0,true),true,0b0,0b0);
	(*unprefixed_list)[70] = *new_inst(0x46,1,1,8,new_operand(5,0,true),new_operand(13,0,false),false,0b0,0b0);
	(*unprefixed_list)[71] = *new_inst(0x47,1,1,4,new_operand(5,0,true),new_operand(4,0,true),true,0b0,0b0);
	(*unprefixed_list)[72] = *new_inst(0x48,1,1,4,new_operand(1,0,true),new_operand(5,0,true),true,0b0,0b0);
	(*unprefixed_list)[73] = *new_inst(0x49,1,1,4,new_operand(1,0,true),new_operand(1,0,true),true,0b0,0b0);
	(*unprefixed_list)[74] = *new_inst(0x4A,1,1,4,new_operand(1,0,true),new_operand(6,0,true),true,0b0,0b0);
	(*unprefixed_list)[75] = *new_inst(0x4B,1,1,4,new_operand(1,0,true),new_operand(2,0,true),true,0b0,0b0);
	(*unprefixed_list)[76] = *new_inst(0x4C,1,1,4,new_operand(1,0,true),new_operand(7,0,true),true,0b0,0b0);
	(*unprefixed_list)[77] = *new_inst(0x4D,1,1,4,new_operand(1,0,true),new_operand(3,0,true),true,0b0,0b0);
	(*unprefixed_list)[78] = *new_inst(0x4E,1,1,8,new_operand(1,0,true),new_operand(13,0,false),false,0b0,0b0);
	(*unprefixed_list)[79] = *new_inst(0x4F,1,1,4,new_operand(1,0,true),new_operand(4,0,true),true,0b0,0b0);
	(*unprefixed_list)[80] = *new_inst(0x50,1,1,4,new_operand(6,0,true),new_operand(5,0,true),true,0b0,0b0);
	(*unprefixed_list)[81] = *new_inst(0x51,1,1,4,new_operand(6,0,true),new_operand(1,0,true),true,0b0,0b0);
	(*unprefixed_list)[82] = *new_inst(0x52,1,1,4,new_operand(6,0,true),new_operand(6,0,true),true,0b0,0b0);
	(*unprefixed_list)[83] = *new_inst(0x53,1,1,4,new_operand(6,0,true),new_operand(2,0,true),true,0b0,0b0);
	(*unprefixed_list)[84] = *new_inst(0x54,1,1,4,new_operand(6,0,true),new_operand(7,0,true),true,0b0,0b0);
	(*unprefixed_list)[85] = *new_inst(0x55,1,1,4,new_operand(6,0,true),new_operand(3,0,true),true,0b0,0b0);
	(*unprefixed_list)[86] = *new_inst(0x56,1,1,8,new_operand(6,0,true),new_operand(13,0,false),false,0b0,0b0);
	(*unprefixed_list)[87] = *new_inst(0x57,1,1,4,new_operand(6,0,true),new_operand(4,0,true),true,0b0,0b0);
	(*unprefixed_list)[88] = *new_inst(0x58,1,1,4,new_operand(2,0,true),new_operand(5,0,true),true,0b0,0b0);
	(*unprefixed_list)[89] = *new_inst(0x59,1,1,4,new_operand(2,0,true),new_operand(1,0,true),true,0b0,0b0);
	(*unprefixed_list)[90] = *new_inst(0x5A,1,1,4,new_operand(2,0,true),new_operand(6,0,true),true,0b0,0b0);
	(*unprefixed_list)[91] = *new_inst(0x5B,1,1,4,new_operand(2,0,true),new_operand(2,0,true),true,0b0,0b0);
	(*unprefixed_list)[92] = *new_inst(0x5C,1,1,4,new_operand(2,0,true),new_operand(7,0,true),true,0b0,0b0);
	(*unprefixed_list)[93] = *new_inst(0x5D,1,1,4,new_operand(2,0,true),new_operand(3,0,true),true,0b0,0b0);
	(*unprefixed_list)[94] = *new_inst(0x5E,1,1,8,new_operand(2,0,true),new_operand(13,0,false),false,0b0,0b0);
	(*unprefixed_list)[95] = *new_inst(0x5F,1,1,4,new_operand(2,0,true),new_operand(4,0,true),true,0b0,0b0);
	(*unprefixed_list)[96] = *new_inst(0x60,1,1,4,new_operand(7,0,true),new_operand(5,0,true),true,0b0,0b0);
	(*unprefixed_list)[97] = *new_inst(0x61,1,1,4,new_operand(7,0,true),new_operand(1,0,true),true,0b0,0b0);
	(*unprefixed_list)[98] = *new_inst(0x62,1,1,4,new_operand(7,0,true),new_operand(6,0,true),true,0b0,0b0);
	(*unprefixed_list)[99] = *new_inst(0x63,1,1,4,new_operand(7,0,true),new_operand(2,0,true),true,0b0,0b0);
	(*unprefixed_list)[100] = *new_inst(0x64,1,1,4,new_operand(7,0,true),new_operand(7,0,true),true,0b0,0b0);
	(*unprefixed_list)[101] = *new_inst(0x65,1,1,4,new_operand(7,0,true),new_operand(3,0,true),true,0b0,0b0);
	(*unprefixed_list)[102] = *new_inst(0x66,1,1,8,new_operand(7,0,true),new_operand(13,0,false),false,0b0,0b0);
	(*unprefixed_list)[103] = *new_inst(0x67,1,1,4,new_operand(7,0,true),new_operand(4,0,true),true,0b0,0b0);
	(*unprefixed_list)[104] = *new_inst(0x68,1,1,4,new_operand(3,0,true),new_operand(5,0,true),true,0b0,0b0);
	(*unprefixed_list)[105] = *new_inst(0x69,1,1,4,new_operand(3,0,true),new_operand(1,0,true),true,0b0,0b0);
	(*unprefixed_list)[106] = *new_inst(0x6A,1,1,4,new_operand(3,0,true),new_operand(6,0,true),true,0b0,0b0);
	(*unprefixed_list)[107] = *new_inst(0x6B,1,1,4,new_operand(3,0,true),new_operand(2,0,true),true,0b0,0b0);
	(*unprefixed_list)[108] = *new_inst(0x6C,1,1,4,new_operand(3,0,true),new_operand(7,0,true),true,0b0,0b0);
	(*unprefixed_list)[109] = *new_inst(0x6D,1,1,4,new_operand(3,0,true),new_operand(3,0,true),true,0b0,0b0);
	(*unprefixed_list)[110] = *new_inst(0x6E,1,1,8,new_operand(3,0,true),new_operand(13,0,false),false,0b0,0b0);
	(*unprefixed_list)[111] = *new_inst(0x6F,1,1,4,new_operand(3,0,true),new_operand(4,0,true),true,0b0,0b0);
	(*unprefixed_list)[112] = *new_inst(0x70,1,1,8,new_operand(13,0,false),new_operand(5,0,true),false,0b0,0b0);
	(*unprefixed_list)[113] = *new_inst(0x71,1,1,8,new_operand(13,0,false),new_operand(1,0,true),false,0b0,0b0);
	(*unprefixed_list)[114] = *new_inst(0x72,1,1,8,new_operand(13,0,false),new_operand(6,0,true),false,0b0,0b0);
	(*unprefixed_list)[115] = *new_inst(0x73,1,1,8,new_operand(13,0,false),new_operand(2,0,true),false,0b0,0b0);
	(*unprefixed_list)[116] = *new_inst(0x74,1,1,8,new_operand(13,0,false),new_operand(7,0,true),false,0b0,0b0);
	(*unprefixed_list)[117] = *new_inst(0x75,1,1,8,new_operand(13,0,false),new_operand(3,0,true),false,0b0,0b0);
	(*unprefixed_list)[118] = *new_inst(0x76,15,1,4,NULL,NULL,true,0b0,0b0);
	(*unprefixed_list)[119] = *new_inst(0x77,1,1,8,new_operand(13,0,false),new_operand(4,0,true),false,0b0,0b0);
	(*unprefixed_list)[120] = *new_inst(0x78,1,1,4,new_operand(4,0,true),new_operand(5,0,true),true,0b0,0b0);
	(*unprefixed_list)[121] = *new_inst(0x79,1,1,4,new_operand(4,0,true),new_operand(1,0,true),true,0b0,0b0);
	(*unprefixed_list)[122] = *new_inst(0x7A,1,1,4,new_operand(4,0,true),new_operand(6,0,true),true,0b0,0b0);
	(*unprefixed_list)[123] = *new_inst(0x7B,1,1,4,new_operand(4,0,true),new_operand(2,0,true),true,0b0,0b0);
	(*unprefixed_list)[124] = *new_inst(0x7C,1,1,4,new_operand(4,0,true),new_operand(7,0,true),true,0b0,0b0);
	(*unprefixed_list)[125] = *new_inst(0x7D,1,1,4,new_operand(4,0,true),new_operand(3,0,true),true,0b0,0b0);
	(*unprefixed_list)[126] = *new_inst(0x7E,1,1,8,new_operand(4,0,true),new_operand(13,0,false),false,0b0,0b0);
	(*unprefixed_list)[127] = *new_inst(0x7F,1,1,4,new_operand(4,0,true),new_operand(4,0,true),true,0b0,0b0);
	(*unprefixed_list)[128] = *new_inst(0x80,5,1,4,new_operand(4,0,true),new_operand(5,0,true),true,0b0,0b1000000);
	(*unprefixed_list)[129] = *new_inst(0x81,5,1,4,new_operand(4,0,true),new_operand(1,0,true),true,0b0,0b1000000);
	(*unprefixed_list)[130] = *new_inst(0x82,5,1,4,new_operand(4,0,true),new_operand(6,0,true),true,0b0,0b1000000);
	(*unprefixed_list)[131] = *new_inst(0x83,5,1,4,new_operand(4,0,true),new_operand(2,0,true),true,0b0,0b1000000);
	(*unprefixed_list)[132] = *new_inst(0x84,5,1,4,new_operand(4,0,true),new_operand(7,0,true),true,0b0,0b1000000);
	(*unprefixed_list)[133] = *new_inst(0x85,5,1,4,new_operand(4,0,true),new_operand(3,0,true),true,0b0,0b1000000);
	(*unprefixed_list)[134] = *new_inst(0x86,5,1,8,new_operand(4,0,true),new_operand(13,0,false),false,0b0,0b1000000);
	(*unprefixed_list)[135] = *new_inst(0x87,5,1,4,new_operand(4,0,true),new_operand(4,0,true),true,0b0,0b1000000);
	(*unprefixed_list)[136] = *new_inst(0x88,16,1,4,new_operand(4,0,true),new_operand(5,0,true),true,0b0,0b1000000);
	(*unprefixed_list)[137] = *new_inst(0x89,16,1,4,new_operand(4,0,true),new_operand(1,0,true),true,0b0,0b1000000);
	(*unprefixed_list)[138] = *new_inst(0x8A,16,1,4,new_operand(4,0,true),new_operand(6,0,true),true,0b0,0b1000000);
	(*unprefixed_list)[139] = *new_inst(0x8B,16,1,4,new_operand(4,0,true),new_operand(2,0,true),true,0b0,0b1000000);
	(*unprefixed_list)[140] = *new_inst(0x8C,16,1,4,new_operand(4,0,true),new_operand(7,0,true),true,0b0,0b1000000);
	(*unprefixed_list)[141] = *new_inst(0x8D,16,1,4,new_operand(4,0,true),new_operand(3,0,true),true,0b0,0b1000000);
	(*unprefixed_list)[142] = *new_inst(0x8E,16,1,8,new_operand(4,0,true),new_operand(13,0,false),false,0b0,0b1000000);
	(*unprefixed_list)[143] = *new_inst(0x8F,16,1,4,new_operand(4,0,true),new_operand(4,0,true),true,0b0,0b1000000);
	(*unprefixed_list)[144] = *new_inst(0x90,17,1,4,new_operand(5,0,true),NULL,true,0b1000000,0b0);
	(*unprefixed_list)[145] = *new_inst(0x91,17,1,4,new_operand(1,0,true),NULL,true,0b1000000,0b0);
	(*unprefixed_list)[146] = *new_inst(0x92,17,1,4,new_operand(6,0,true),NULL,true,0b1000000,0b0);
	(*unprefixed_list)[147] = *new_inst(0x93,17,1,4,new_operand(2,0,true),NULL,true,0b1000000,0b0);
	(*unprefixed_list)[148] = *new_inst(0x94,17,1,4,new_operand(7,0,true),NULL,true,0b1000000,0b0);
	(*unprefixed_list)[149] = *new_inst(0x95,17,1,4,new_operand(3,0,true),NULL,true,0b1000000,0b0);
	(*unprefixed_list)[150] = *new_inst(0x96,17,1,8,new_operand(13,0,false),NULL,false,0b1000000,0b0);
	(*unprefixed_list)[151] = *new_inst(0x97,17,1,4,new_operand(4,0,true),NULL,true,0b11000000,0b110000);
	(*unprefixed_list)[152] = *new_inst(0x98,18,1,4,new_operand(4,0,true),new_operand(5,0,true),true,0b1000000,0b0);
	(*unprefixed_list)[153] = *new_inst(0x99,18,1,4,new_operand(4,0,true),new_operand(1,0,true),true,0b1000000,0b0);
	(*unprefixed_list)[154] = *new_inst(0x9A,18,1,4,new_operand(4,0,true),new_operand(6,0,true),true,0b1000000,0b0);
	(*unprefixed_list)[155] = *new_inst(0x9B,18,1,4,new_operand(4,0,true),new_operand(2,0,true),true,0b1000000,0b0);
	(*unprefixed_list)[156] = *new_inst(0x9C,18,1,4,new_operand(4,0,true),new_operand(7,0,true),true,0b1000000,0b0);
	(*unprefixed_list)[157] = *new_inst(0x9D,18,1,4,new_operand(4,0,true),new_operand(3,0,true),true,0b1000000,0b0);
	(*unprefixed_list)[158] = *new_inst(0x9E,18,1,8,new_operand(4,0,true),new_operand(13,0,false),false,0b1000000,0b0);
	(*unprefixed_list)[159] = *new_inst(0x9F,18,1,4,new_operand(4,0,true),new_operand(4,0,true),true,0b1000000,0b0);
	(*unprefixed_list)[160] = *new_inst(0xA0,19,1,4,new_operand(5,0,true),NULL,true,0b100000,0b1010000);
	(*unprefixed_list)[161] = *new_inst(0xA1,19,1,4,new_operand(1,0,true),NULL,true,0b100000,0b1010000);
	(*unprefixed_list)[162] = *new_inst(0xA2,19,1,4,new_operand(6,0,true),NULL,true,0b100000,0b1010000);
	(*unprefixed_list)[163] = *new_inst(0xA3,19,1,4,new_operand(2,0,true),NULL,true,0b100000,0b1010000);
	(*unprefixed_list)[164] = *new_inst(0xA4,19,1,4,new_operand(7,0,true),NULL,true,0b100000,0b1010000);
	(*unprefixed_list)[165] = *new_inst(0xA5,19,1,4,new_operand(3,0,true),NULL,true,0b100000,0b1010000);
	(*unprefixed_list)[166] = *new_inst(0xA6,19,1,8,new_operand(13,0,false),NULL,false,0b100000,0b1010000);
	(*unprefixed_list)[167] = *new_inst(0xA7,19,1,4,new_operand(4,0,true),NULL,true,0b100000,0b1010000);
	(*unprefixed_list)[168] = *new_inst(0xA8,20,1,4,new_operand(5,0,true),NULL,true,0b0,0b1110000);
	(*unprefixed_list)[169] = *new_inst(0xA9,20,1,4,new_operand(1,0,true),NULL,true,0b0,0b1110000);
	(*unprefixed_list)[170] = *new_inst(0xAA,20,1,4,new_operand(6,0,true),NULL,true,0b0,0b1110000);
	(*unprefixed_list)[171] = *new_inst(0xAB,20,1,4,new_operand(2,0,true),NULL,true,0b0,0b1110000);
	(*unprefixed_list)[172] = *new_inst(0xAC,20,1,4,new_operand(7,0,true),NULL,true,0b0,0b1110000);
	(*unprefixed_list)[173] = *new_inst(0xAD,20,1,4,new_operand(3,0,true),NULL,true,0b0,0b1110000);
	(*unprefixed_list)[174] = *new_inst(0xAE,20,1,8,new_operand(13,0,false),NULL,false,0b0,0b1110000);
	(*unprefixed_list)[175] = *new_inst(0xAF,20,1,4,new_operand(4,0,true),NULL,true,0b10000000,0b1110000);
	(*unprefixed_list)[176] = *new_inst(0xB0,21,1,4,new_operand(5,0,true),NULL,true,0b0,0b1110000);
	(*unprefixed_list)[177] = *new_inst(0xB1,21,1,4,new_operand(1,0,true),NULL,true,0b0,0b1110000);
	(*unprefixed_list)[178] = *new_inst(0xB2,21,1,4,new_operand(6,0,true),NULL,true,0b0,0b1110000);
	(*unprefixed_list)[179] = *new_inst(0xB3,21,1,4,new_operand(2,0,true),NULL,true,0b0,0b1110000);
	(*unprefixed_list)[180] = *new_inst(0xB4,21,1,4,new_operand(7,0,true),NULL,true,0b0,0b1110000);
	(*unprefixed_list)[181] = *new_inst(0xB5,21,1,4,new_operand(3,0,true),NULL,true,0b0,0b1110000);
	(*unprefixed_list)[182] = *new_inst(0xB6,21,1,8,new_operand(13,0,false),NULL,false,0b0,0b1110000);
	(*unprefixed_list)[183] = *new_inst(0xB7,21,1,4,new_operand(4,0,true),NULL,true,0b0,0b1110000);
	(*unprefixed_list)[184] = *new_inst(0xB8,22,1,4,new_operand(5,0,true),NULL,true,0b1000000,0b0);
	(*unprefixed_list)[185] = *new_inst(0xB9,22,1,4,new_operand(1,0,true),NULL,true,0b1000000,0b0);
	(*unprefixed_list)[186] = *new_inst(0xBA,22,1,4,new_operand(6,0,true),NULL,true,0b1000000,0b0);
	(*unprefixed_list)[187] = *new_inst(0xBB,22,1,4,new_operand(2,0,true),NULL,true,0b1000000,0b0);
	(*unprefixed_list)[188] = *new_inst(0xBC,22,1,4,new_operand(7,0,true),NULL,true,0b1000000,0b0);
	(*unprefixed_list)[189] = *new_inst(0xBD,22,1,4,new_operand(3,0,true),NULL,true,0b1000000,0b0);
	(*unprefixed_list)[190] = *new_inst(0xBE,22,1,8,new_operand(13,0,false),NULL,false,0b1000000,0b0);
	(*unprefixed_list)[191] = *new_inst(0xBF,22,1,4,new_operand(4,0,true),NULL,true,0b11000000,0b110000);
	(*unprefixed_list)[192] = *new_inst(0xC0,23,1,20,new_operand(99,0,true),NULL,true,0b0,0b0);
	(*unprefixed_list)[193] = *new_inst(0xC1,24,1,12,new_operand(11,0,true),NULL,true,0b0,0b0);
	(*unprefixed_list)[194] = *new_inst(0xC2,25,3,16,new_operand(99,0,true),new_operand(14,2,true),true,0b0,0b0);
	(*unprefixed_list)[195] = *new_inst(0xC3,25,3,16,new_operand(99,2,true),NULL,true,0b0,0b0);
	(*unprefixed_list)[196] = *new_inst(0xC4,26,3,24,new_operand(99,0,true),new_operand(14,2,true),true,0b0,0b0);
	(*unprefixed_list)[197] = *new_inst(0xC5,27,1,16,new_operand(11,0,true),NULL,true,0b0,0b0);
	(*unprefixed_list)[198] = *new_inst(0xC6,5,2,8,new_operand(4,0,true),new_operand(14,1,true),true,0b0,0b1000000);
	(*unprefixed_list)[199] = *new_inst(0xC7,28,1,16,new_operand(99,0,true),NULL,true,0b0,0b0);
	(*unprefixed_list)[200] = *new_inst(0xC8,23,1,20,new_operand(99,0,true),NULL,true,0b0,0b0);
	(*unprefixed_list)[201] = *new_inst(0xC9,23,1,16,NULL,NULL,true,0b0,0b0);
	(*unprefixed_list)[202] = *new_inst(0xCA,25,3,16,new_operand(99,0,true),new_operand(14,2,true),true,0b0,0b0);
	(*unprefixed_list)[203] = *new_inst(0xCB,29,1,4,NULL,NULL,true,0b0,0b0);
	(*unprefixed_list)[204] = *new_inst(0xCC,26,3,24,new_operand(99,0,true),new_operand(14,2,true),true,0b0,0b0);
	(*unprefixed_list)[205] = *new_inst(0xCD,26,3,24,new_operand(99,2,true),NULL,true,0b0,0b0);
	(*unprefixed_list)[206] = *new_inst(0xCE,16,2,8,new_operand(4,0,true),new_operand(14,1,true),true,0b0,0b1000000);
	(*unprefixed_list)[207] = *new_inst(0xCF,28,1,16,new_operand(99,0,true),NULL,true,0b0,0b0);
	(*unprefixed_list)[208] = *new_inst(0xD0,23,1,20,new_operand(99,0,true),NULL,true,0b0,0b0);
	(*unprefixed_list)[209] = *new_inst(0xD1,24,1,12,new_operand(12,0,true),NULL,true,0b0,0b0);
	(*unprefixed_list)[210] = *new_inst(0xD2,25,3,16,new_operand(99,0,true),new_operand(14,2,true),true,0b0,0b0);
	(*unprefixed_list)[211] = *new_inst(0xD3,30,1,4,NULL,NULL,true,0b0,0b0);
	(*unprefixed_list)[212] = *new_inst(0xD4,26,3,24,new_operand(99,0,true),new_operand(14,2,true),true,0b0,0b0);
	(*unprefixed_list)[213] = *new_inst(0xD5,27,1,16,new_operand(12,0,true),NULL,true,0b0,0b0);
	(*unprefixed_list)[214] = *new_inst(0xD6,17,2,8,new_operand(99,1,true),NULL,true,0b1000000,0b0);
	(*unprefixed_list)[215] = *new_inst(0xD7,28,1,16,new_operand(99,0,true),NULL,true,0b0,0b0);
	(*unprefixed_list)[216] = *new_inst(0xD8,23,1,20,new_operand(1,0,true),NULL,true,0b0,0b0);
	(*unprefixed_list)[217] = *new_inst(0xD9,31,1,16,NULL,NULL,true,0b0,0b0);
	(*unprefixed_list)[218] = *new_inst(0xDA,25,3,16,new_operand(1,0,true),new_operand(14,2,true),true,0b0,0b0);
	(*unprefixed_list)[219] = *new_inst(0xDB,32,1,4,NULL,NULL,true,0b0,0b0);
	(*unprefixed_list)[220] = *new_inst(0xDC,26,3,24,new_operand(1,0,true),new_operand(14,2,true),true,0b0,0b0);
	(*unprefixed_list)[221] = *new_inst(0xDD,33,1,4,NULL,NULL,true,0b0,0b0);
	(*unprefixed_list)[222] = *new_inst(0xDE,18,2,8,new_operand(4,0,true),new_operand(14,1,true),true,0b1000000,0b0);
	(*unprefixed_list)[223] = *new_inst(0xDF,28,1,16,new_operand(99,0,true),NULL,true,0b0,0b0);
	(*unprefixed_list)[224] = *new_inst(0xE0,34,2,12,new_operand(99,1,false),new_operand(4,0,true),false,0b0,0b0);
	(*unprefixed_list)[225] = *new_inst(0xE1,24,1,12,new_operand(13,0,true),NULL,true,0b0,0b0);
	(*unprefixed_list)[226] = *new_inst(0xE2,1,1,8,new_operand(1,0,false),new_operand(4,0,true),false,0b0,0b0);
	(*unprefixed_list)[227] = *new_inst(0xE3,35,1,4,NULL,NULL,true,0b0,0b0);
	(*unprefixed_list)[228] = *new_inst(0xE4,36,1,4,NULL,NULL,true,0b0,0b0);
	(*unprefixed_list)[229] = *new_inst(0xE5,27,1,16,new_operand(13,0,true),NULL,true,0b0,0b0);
	(*unprefixed_list)[230] = *new_inst(0xE6,19,2,8,new_operand(99,1,true),NULL,true,0b100000,0b1010000);
	(*unprefixed_list)[231] = *new_inst(0xE7,28,1,16,new_operand(99,0,true),NULL,true,0b0,0b0);
	(*unprefixed_list)[232] = *new_inst(0xE8,5,2,16,new_operand(9,0,true),new_operand(14,1,true),true,0b0,0b11000000);
	(*unprefixed_list)[233] = *new_inst(0xE9,25,1,4,new_operand(13,0,true),NULL,true,0b0,0b0);
	(*unprefixed_list)[234] = *new_inst(0xEA,1,3,16,new_operand(99,2,false),new_operand(4,0,true),false,0b0,0b0);
	(*unprefixed_list)[235] = *new_inst(0xEB,37,1,4,NULL,NULL,true,0b0,0b0);
	(*unprefixed_list)[236] = *new_inst(0xEC,38,1,4,NULL,NULL,true,0b0,0b0);
	(*unprefixed_list)[237] = *new_inst(0xED,39,1,4,NULL,NULL,true,0b0,0b0);
	(*unprefixed_list)[238] = *new_inst(0xEE,20,2,8,new_operand(99,1,true),NULL,true,0b0,0b1110000);
	(*unprefixed_list)[239] = *new_inst(0xEF,28,1,16,new_operand(99,0,true),NULL,true,0b0,0b0);
	(*unprefixed_list)[240] = *new_inst(0xF0,34,2,12,new_operand(4,0,true),new_operand(14,1,false),false,0b0,0b0);
	(*unprefixed_list)[241] = *new_inst(0xF1,24,1,12,new_operand(10,0,true),NULL,true,0b0,0b0);
	(*unprefixed_list)[242] = *new_inst(0xF2,1,1,8,new_operand(4,0,true),new_operand(1,0,false),false,0b0,0b0);
	(*unprefixed_list)[243] = *new_inst(0xF3,40,1,4,NULL,NULL,true,0b0,0b0);
	(*unprefixed_list)[244] = *new_inst(0xF4,41,1,4,NULL,NULL,true,0b0,0b0);
	(*unprefixed_list)[245] = *new_inst(0xF5,27,1,16,new_operand(10,0,true),NULL,true,0b0,0b0);
	(*unprefixed_list)[246] = *new_inst(0xF6,21,2,8,new_operand(99,1,true),NULL,true,0b0,0b1110000);
	(*unprefixed_list)[247] = *new_inst(0xF7,28,1,16,new_operand(99,0,true),NULL,true,0b0,0b0);
	(*unprefixed_list)[248] = *new_inst(0xF8,1,2,12,new_operand(13,0,true),new_operand(9,0,true),true,0b0,0b11000000);
	(*unprefixed_list)[249] = *new_inst(0xF9,1,1,8,new_operand(9,0,true),new_operand(13,0,true),true,0b0,0b0);
	(*unprefixed_list)[250] = *new_inst(0xFA,1,3,16,new_operand(4,0,true),new_operand(14,2,false),false,0b0,0b0);
	(*unprefixed_list)[251] = *new_inst(0xFB,42,1,4,NULL,NULL,true,0b0,0b0);
	(*unprefixed_list)[252] = *new_inst(0xFC,43,1,4,NULL,NULL,true,0b0,0b0);
	(*unprefixed_list)[253] = *new_inst(0xFD,44,1,4,NULL,NULL,true,0b0,0b0);
	(*unprefixed_list)[254] = *new_inst(0xFE,22,2,8,new_operand(99,1,true),NULL,true,0b1000000,0b0);
	(*unprefixed_list)[255] = *new_inst(0xFF,28,1,16,new_operand(99,0,true),NULL,true,0b0,0b0);

}

void findInstruction(uint16_t address){
	printf("0x%X : 0x%X\n",address,0xF0F0);
}

/*
	Memory Idenifiers:
	11 : Interrupt Enable Register
	10 : Internal High RAM
	09 : I/O Ports
	08 : Empty
	07 : Sprite Atttribute Table
	06 : ECHO RAM
	05 : 4KB Work RAM bank 1~N
	04 : 4KB Work RAM Bank 0
	03 : 8kb External Ram (In Cartridge, if any)
	02 : 8kb Video Ram
	01 : 16KB ROM BANK 01~NN
	00 : 16KB ROM BANK 00
*/
int main () {
	// List of all instructions
	struct Instruction *unprefixed_list = (struct Instruction*)malloc(sizeof(struct Instruction)*256);
	populate_instructions(&unprefixed_list);

	// Entire Memeory Bank split into sections
	uint8_t *memoryBank[12];

	// All CPU Registers
	struct registerBank Registers;
	initRegisters(&Registers);
	printf("Done with Registers\n");

	//Stores cartidge Metadata
	struct cart_metadata metadata;

	// Read all data from ROM file into cartride
	readCart(&memoryBank[0]); // memoryBank[0] is the cartridge
	// Fill metadata object with data from cartidge memory
	read_cart_metadata(&memoryBank[0],&metadata);
	// Set Program Counter from Metadata
	// This is a bit of a shortcut, the entrypoint really contains code
	// In Most cases it is a NOP JMP Low High, though
	uint16_t init_pc_value = (metadata.entry_point[3]*256)+metadata.entry_point[2];	
	writeRegister16(&Registers, REG_PC, init_pc_value);

	// Read Through Entire Cartridge
	// This will not be the final loop, just for testing purposes
	while(readRegister16(&Registers,REG_PC)<512){
		uint16_t current_count = readRegister16(&Registers,REG_PC);
		
		//Right now just prints an instructions mnemonic and skips over the data that it uses
		// Not sure if it skips over the right amount or now.
		for(int op_index=0;op_index<256;op_index++){
			if(unprefixed_list[op_index].opcode == memoryBank[0][current_count]){
				struct Instruction matched_inst = unprefixed_list[op_index];
				printf("%X : %s \n",current_count,mnemonics[matched_inst.mne_ID]);
				current_count = current_count + (matched_inst.bytes-1);
				break;
			}
		}


		current_count++;
		writeRegister16(&Registers,REG_PC,current_count);
	}
	
	//Testing Purposes Only:
	//testRegisters(&Registers);
	//printMetadata(&metadata);
	return 0;
}
