#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

int debug = 0;

// Reads entire cartidge file into cart_buffer
void readCart(unsigned char** cart_buffer){
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
  *cart_buffer = (unsigned char*) malloc (sizeof(unsigned char)*lSize);
  if (cart_buffer == NULL) {fputs ("Memory error",stderr); exit (2);}

  // copy the file into the buffer:
  result = fread (*cart_buffer,1,lSize,pFile);
  if (result != lSize) {fputs ("Reading error",stderr); exit (3);}

   // terminate
  fclose (pFile);
}

void read_data_from_cart(unsigned char** cart_buffer, unsigned char** variable, unsigned int location, unsigned int size){
	for(int i=0; i<size; i++){
		(*variable)[i] = (*cart_buffer)[i+location];
	}
}


struct cart_metadata {
	unsigned char* entry_point; 	// Often NOP JP 0x150
	unsigned char* logo; 		    // Logo data
	unsigned char* title; 				// The Title
	unsigned char* cgb;						// Gameboy Color?
	// New Licencsee Code? Not sure what this is 0x144-0x145
	bool sgb;						// Super Gameboy
	unsigned char* cartidge_type;
	unsigned char* rom_size;
	unsigned char* ram_size;
	unsigned char* destination_code;
	unsigned char* old_licensee_code;
	unsigned char* mask_rom_version;
	unsigned char* header_checksum;
	unsigned char* global_checksum;
};

void read_cart_metadata(unsigned char** cart_buffer, struct cart_metadata *metadata){

	// Allocate Memory in the passed struct and then assign it data
	metadata->entry_point = (unsigned char*) malloc (sizeof(unsigned char)*4);
	read_data_from_cart(cart_buffer, &metadata->entry_point, 0x100, 4);

	metadata->logo = (unsigned char*) malloc (sizeof(unsigned char)*48);
	read_data_from_cart(cart_buffer, &metadata->logo, 0x104, 48);

	metadata->title = (unsigned char*) malloc (sizeof(unsigned char)*15);
	read_data_from_cart(cart_buffer, &metadata->title, 0x134, 15);

	metadata->cgb= (unsigned char*) malloc (sizeof(unsigned char)*1);
	read_data_from_cart(cart_buffer, &metadata->cgb, 0x143,1);

	//New Licensee Code could go here

	metadata->cartidge_type = (unsigned char*) malloc (sizeof(unsigned char)*1);
	read_data_from_cart(cart_buffer, &metadata->cartidge_type, 0x147, 1);

	metadata->rom_size = (unsigned char*) malloc (sizeof(unsigned char)*1);
	read_data_from_cart(cart_buffer, &metadata->rom_size, 0x148, 1);

	metadata->ram_size = (unsigned char*) malloc (sizeof(unsigned char)*1);
	read_data_from_cart(cart_buffer, &metadata->ram_size, 0x149, 1);

	metadata->destination_code = (unsigned char*) malloc (sizeof(unsigned char)*1);
	read_data_from_cart(cart_buffer, &metadata->destination_code, 0x14A, 1);

	metadata->old_licensee_code = (unsigned char*) malloc (sizeof(unsigned char)*1);
	read_data_from_cart(cart_buffer, &metadata->old_licensee_code,0x14B,1);

	metadata->mask_rom_version = (unsigned char*) malloc (sizeof(unsigned char)*2);
	read_data_from_cart(cart_buffer, &metadata->mask_rom_version,0x14C,1);

	metadata->header_checksum = (unsigned char*) malloc (sizeof(unsigned char)*2);
	read_data_from_cart(cart_buffer, &metadata->header_checksum,0x14D,1);

	metadata->global_checksum = (unsigned char*) malloc (sizeof(unsigned char)*2);
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
  unsigned char *memoryBank[12];

  struct cart_metadata metadata;

  // Remember memoryBank[0] is the cartidge bank
  readCart(&memoryBank[0]);
  read_cart_metadata(&memoryBank[0],&metadata);

  //Testing Purposes Only:
  printMetadata(&metadata);
   
  //Free Cartidge Bank, this will have to act on all memory later
  free (memoryBank[0]);
  return 0;
}
