# Python program to read
# json file
  
  
import json
  
# Opening JSON file
f = open('opcodes.json')
  
# returns JSON object as 
# a dictionary
data = json.load(f)

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
"HL"
]
  
# Stores Unique mnemonics
unprefixed_mnemonic_list = []
for i in data['unprefixed']:
	word = data['unprefixed'][i]['mnemonic']
	if word not in unprefixed_mnemonic_list:
		unprefixed_mnemonic_list.append(word)

# Stores Uniqie mnemonics
cb_mnemonic_list = []
for i in data['cbprefixed']:
	word = data['cbprefixed'][i]['mnemonic']
	if word not in cb_mnemonic_list:
		cb_mnemonic_list.append(word)

unprefixed_count = 0
for i in data['unprefixed']:
	obj = data['unprefixed'][i]

	opcode = i

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
	operand0_string="NULL"
	operand1_string="NULL"

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

		operand0_string = "new_operand("+str(operand_0_reg_id)+","+str(op_0_bytes)+","
		if operand0_obj['immediate']==True:
			operand0_string = operand0_string + "true"
		else:
			operand0_string = operand0_string + "false"
		operand0_string = operand0_string + ")"

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

		operand1_string = "new_operand("+str(operand1_name_index)+","+str(op_1_bytes)+","
		if operand1_obj['immediate']==True:
			operand1_string = operand1_string + "true"
		else:
			operand1_string = operand1_string + "false"
		operand1_string = operand1_string + ")"

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

	print("(*unprefixed_list)["+str(unprefixed_count)+"] = *new_inst("+str(opcode)+","+str(mne_ID)+","+str(bytes_num)+","+str(cycles)+","+operand0_string+","+operand1_string+","+immediate_string+","+bin(flag_set)+","+bin(flag_reset)+");")
	unprefixed_count = unprefixed_count +1
	

# Closing file
f.close()