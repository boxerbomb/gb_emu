# Python program to read
# json file
import json
from dataclasses import dataclass

# Opening JSON file
f = open('opcodes.json')
  
# returns JSON object as 
# a dictionary
data = json.load(f)


# Stores Unique mnemonics
unprefixed_mnemonic_list = []
unprefixed_opcode_list = []
for i in data['unprefixed']:
	word = data['unprefixed'][i]['mnemonic']
	if word not in unprefixed_mnemonic_list:
		unprefixed_mnemonic_list.append(word)
		unprefixed_opcode_list.append([])


REGISTER_LIST = [
"F",
"C",
"E",
"L",
"A",
"B",
"D",
"H",
"PC",
"SP",
"AF",
"BC",
"DE",
"HL",
"d8",
"d16",
"a8",
"a16",
"r8",
"00H",
"08H",
"10H",
"18H",
"20H",
"28H",
"30H",
"38H",
"NC",
"C",
"NZ",
"Z",
"0",
"1",
"2",
"3",
"4",
"5",
"6",
"7",
]


@dataclass
class opcode_class:
    mnemonic: str
    mne_ID: int
    opcode: int
    my_bytes : int
    operand_0_reg_id: int
    operand_0_imm: int
    operand_0_bytes: int
    operand_1_reg_id: int
    operand_1_imm: int
    operand_1_bytes: int
    flag_set : int
    flag_reset : int



for i in data['unprefixed']:
	obj = data['unprefixed'][i]

	opcode = i
	print(opcode)

	mne_index = 0
	mne_ID = 99
	for mne_item in unprefixed_mnemonic_list:
		if mne_item==data['unprefixed'][i]['mnemonic']:
			mne_ID = mne_index
			break
		mne_index = mne_index + 1

	bytes_num = data['unprefixed'][i]['bytes']
	cycles = data['unprefixed'][i]['cycles'][0]

	operands_list = data['unprefixed'][i]['operands']

	if len(operands_list)>=1:
		operand0_obj = data['unprefixed'][i]['operands'][0]
		op_0_bytes = 0

		operand0_name = data['unprefixed'][i]['operands'][0]['name']
		operand_0_reg_id = 99
		operand0_name_index = 0
		for item in REGISTER_LIST:
			if operand0_name.upper()==item.upper():
				operand_0_reg_id = operand0_name_index
				break
			operand0_name_index = operand0_name_index + 1

		try:
			op_0_bytes = operand0_obj['bytes']
		except:
			op_0_bytes = 0

		if operand0_obj['immediate']==True:
			operand_0_imm = 1
		else:
			operand_0_imm = 0

	else:
		operand0_name_index = -1
		operand_0_imm = -1
		op_0_bytes = -1



	if len(operands_list)>=2:
		operand1_obj = data['unprefixed'][i]['operands'][1]
		op_1_bytes = 0

		try:
			op_1_bytes = operand1_obj['bytes']
		except:
			op_1_bytes = 0

		operand1_name = data['unprefixed'][i]['operands'][1]['name']
		operand_1_reg_id = 99
		operand1_name_index = 0
		for item in REGISTER_LIST:
			if operand1_name.upper()==item.upper():
				operand_1_reg_id = operand1_name_index
				break
			operand1_name_index = operand1_name_index + 1

		if operand1_obj['immediate']==True:
			operand_1_imm = 1
		else:
			operand_1_imm = 0

	else:
		operand1_name_index = -1
		operand_1_imm = -1
		op_1_bytes = -1

	if data['unprefixed'][i]['immediate']==True:
		immediate_string = "true"
	else:
		immediate_string = "false"

	flag_set = 0
	if data['unprefixed'][i]['flags']['Z'] == '1':
		flag_set = flag_set | 0b10000000 
	if data['unprefixed'][i]['flags']['N'] == '1':
		flag_set = flag_set | 0b01000000
	if data['unprefixed'][i]['flags']['H'] == '1':
		flag_set = flag_set | 0b00100000
	if data['unprefixed'][i]['flags']['C'] == '1':
		flag_set = flag_set | 0b00010000

	flag_reset = 0
	if data['unprefixed'][i]['flags']['Z'] == '0':
		flag_reset = flag_reset | 0b10000000 
	if data['unprefixed'][i]['flags']['N'] == '0':
		flag_reset = flag_reset | 0b01000000
	if data['unprefixed'][i]['flags']['H'] == '0':
		flag_reset = flag_reset | 0b00100000
	if data['unprefixed'][i]['flags']['C'] == '0':
		flag_reset = flag_reset | 0b00010000

	current_opcode = opcode_class(data['unprefixed'][i]['mnemonic'], mne_ID, opcode, bytes_num, operand0_name_index, operand_0_imm, op_0_bytes, operand1_name_index, operand_1_imm, op_1_bytes, flag_set, flag_reset)


	temp_index=0
	for a_item in unprefixed_mnemonic_list:
		#print(a_item,current_opcode.opcode)
		if a_item == current_opcode.mnemonic:
			print("found")
			break
		temp_index = temp_index + 1

	# Add to the per-mnemonic list
	unprefixed_opcode_list[temp_index].append(current_opcode)


temp_index=0
for item in unprefixed_mnemonic_list:
	print("void "+item+"(){")
	print("\tint opcode = 0;")
	print("\tswitch (opcode){")
	for opcode_item in unprefixed_opcode_list[temp_index]:
		print("\t\tcase "+str(opcode_item.opcode)+":")
		print("\t\t\t//Uses "+str(opcode_item.my_bytes)+" Bytes")

		if(opcode_item.operand_0_reg_id != -1):
			is_imm = ""
			if(opcode_item.operand_0_imm == 1):
				is_imm = "[IMM] "
			print("\t\t\t// Uses "+is_imm+"Operand: "+REGISTER_LIST[opcode_item.operand_0_reg_id]+" (index: "+str(opcode_item.operand_0_reg_id)+")"+" of bytes: "+str(opcode_item.operand_0_bytes))	

		if(opcode_item.operand_1_reg_id != -1):
			is_imm = ""
			if(opcode_item.operand_1_imm == 1):
				is_imm = "[IMM] "
			print("\t\t\t// Uses "+is_imm+"Operand: "+REGISTER_LIST[opcode_item.operand_1_reg_id]+" (index: "+str(opcode_item.operand_1_reg_id)+")"+" of bytes: "+str(opcode_item.operand_1_bytes))

		print("\t\t\t// Set Flags: "+str(opcode_item.flag_set)+" encoded bits")
		print("\t\t\t// Reset Flags: "+str(opcode_item.flag_reset)+" encoded bits")

		print("\t\t\tbreak;")
	print("\t}")
	print("}")
	temp_index=temp_index+1


