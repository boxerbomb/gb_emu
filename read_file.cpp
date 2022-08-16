#include <iostream>


struct cart_metadata {
	unsigned char* entry_point; 	// Often NOP JP 0x150
	unsigned char* logo; 		    // Logo data
	unsigned char* title; 				// The Title
	bool cgb;						// Gameboy Color?
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


class MemoryController{
private:
	int test;
	unsigned char * cart_buffer;
	void read_data_from_cart(unsigned char* variable, unsigned int location, unsigned int size);
	void read_data_from_cart(char* variable, unsigned int location, unsigned int size);
	void read_data_from_cart(bool &variable, unsigned int location, unsigned int size=0);
public:
	cart_metadata metadata;
	void read_memory();
	void write_memory();
	void readCart();
	void read_cart_metadata();
	void freeMem();
};


// Reads entire cartidge file into cart_buffer
void MemoryController::readCart(){
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
  cart_buffer = (unsigned char*) malloc (sizeof(unsigned char)*lSize);
  if (cart_buffer == NULL) {fputs ("Memory error",stderr); exit (2);}

  // copy the file into the buffer:
  result = fread (cart_buffer,1,lSize,pFile);
  if (result != lSize) {fputs ("Reading error",stderr); exit (3);}

   // terminate
  fclose (pFile);
}

void read_data_from_cart(unsigned char* variable, unsigned int location, unsigned int size){
	for(int i=0; i<size; i++){
		(*variable)[i] = cart_buffer[i+location];
	}
}
void read_data_from_cart(char* variable, unsigned int location, unsigned int size){
	for(int i=0; i<size; i++){
		(*variable)[i] = cart_buffer[i+location];
	}
}

void read_data_from_cart(bool &variable, unsigned int location, unsigned int size=0){
		if ((*cart_buffer)[location] == 0x00){
			variable = false;
		}else{
			variable = true;
		}
}

void MemoryController::freeMem(){
	  free (cart_buffer);
}

void MemoryController::read_cart_metadata(){

	// Allocate Memory in the passed struct and then assign it data
	metadata.entry_point = (unsigned char*) malloc (sizeof(unsigned char)*4);
	read_data_from_cart(metadata.entry_point, 0x100, 4);

	metadata.logo = (unsigned char*) malloc (sizeof(unsigned char)*48);
	read_data_from_cart(&metadata.logo, 0x104, 48);

	metadata.title = (unsigned char*) malloc (sizeof(unsigned char)*15);
	read_data_from_cart(&metadata.title, 0x134, 15);

	//Just a boolean, no need to allocate memory
	read_data_from_cart(metadata.cgb, 0x143);

	//New Licensee Code could go here

	metadata.cartidge_type = (unsigned char*) malloc (sizeof(unsigned char)*1);
	read_data_from_cart(&metadata.cartidge_type, 0x147, 1);

	metadata.rom_size = (unsigned char*) malloc (sizeof(unsigned char)*1);
	read_data_from_cart(&metadata.rom_size, 0x148, 1);

	metadata.ram_size = (unsigned char*) malloc (sizeof(unsigned char)*1);
	read_data_from_cart(&metadata.ram_size, 0x149, 1);

	metadata.destination_code = (unsigned char*) malloc (sizeof(unsigned char)*1);
	read_data_from_cart(&metadata.destination_code, 0x14A, 1);

	metadata.old_licensee_code = (unsigned char*) malloc (sizeof(unsigned char)*1);
	read_data_from_cart(&metadata.old_licensee_code,0x14B,1);

	metadata.mask_rom_version = (unsigned char*) malloc (sizeof(unsigned char)*2);
	read_data_from_cart(&metadata.mask_rom_version,0x14C,1);

	metadata.header_checksum = (unsigned char*) malloc (sizeof(unsigned char)*2);
	read_data_from_cart(&metadata.header_checksum,0x14D,1);

	metadata.global_checksum = (unsigned char*) malloc (sizeof(unsigned char)*2);
	read_data_from_cart(&metadata.global_checksum,0x14E,2);
}


void printMetadata(cart_metadata *metadata){
	std::cout<<"Entry Point: 0x"<<std::hex<<(unsigned int)metadata->entry_point[3]<<(unsigned int)metadata->entry_point[2]<<std::endl;

	std::cout<<"Game Title: ";
	int index=0;
	while(metadata->title[index] != 0x00){
		std::cout<<metadata->title[index];
		index++;
	}
	std::cout<<std::endl;

	std::cout<<"Color? ";
	if(metadata->cgb == 0x00){
		std::cout<<"No"<<std::endl;
	}else if(metadata->cgb == 0x80){
		std::cout<<"Supported"<<std::endl;
	}else if(metadata->cgb == 0xC0){
		std::cout<<"Required"<<std::endl;
	}

	std::cout<<"More metadata available"<<std::endl;
}


int main () {

  MemoryController mem_control;

  mem_control.readCart();
  mem_control.read_cart_metadata();

  //readCart(&cart_buffer);
  //read_cart_metadata(&cart_buffer,&metadata);

  //Testing Purposes Only:
  printMetadata(&mem_control.metadata);
  
  return 0;
}
