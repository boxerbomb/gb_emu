# Python program to read
# json file
import json
  
# Opening JSON file
f = open('opcodes.json')
  
# returns JSON object as 
# a dictionary
data = json.load(f)


# Stores Unique mnemonics
unprefixed_mnemonic_list = []
for i in data['unprefixed']:
	word = data['unprefixed'][i]['mnemonic']
	if word not in unprefixed_mnemonic_list:
		unprefixed_mnemonic_list.append(word)


opcode_list = []
index = 0
for word in unprefixed_mnemonic_list:
	opcode_list.append([])
	for i in data['unprefixed']:
		if word == data['unprefixed'][i]['mnemonic']:
			opcode_list[index].append(i)
	index=index+1

print("if(prefix_mode==false){")
print("\tswitch(matched_inst.opcode){")
index=0
for word in opcode_list:
	opcode_string = ""
	for opcode in opcode_list[index]:
		opcode_string = opcode_string + "case "+opcode + ": "
	print("\t\t"+opcode_string)
	print("\t\t\t"+unprefixed_mnemonic_list[index]+"();")
	print("\t\t\tbreak;")
	index=index+1
print("\t\tdefault:")
print("\t\t\tprintf(\"Invalid Opcode.\\n\");")
print("\t}")
print("}")



# Stores Unique mnemonics
cbprefixed_mnemonic_list = []
for i in data['cbprefixed']:
	word = data['cbprefixed'][i]['mnemonic']
	if word not in cbprefixed_mnemonic_list:
		cbprefixed_mnemonic_list.append(word)


opcode_list = []
index = 0
for word in cbprefixed_mnemonic_list:
	opcode_list.append([])
	for i in data['cbprefixed']:
		if word == data['cbprefixed'][i]['mnemonic']:
			opcode_list[index].append(i)
	index=index+1

print("if(prefix_mode==true){")
print("switch(matched_inst.opcode){")
index=0
for word in opcode_list:
	opcode_string = ""
	for opcode in opcode_list[index]:
		opcode_string = opcode_string + "case "+opcode + ": "
	print("\t\t"+opcode_string)
	print("\t\t\t"+cbprefixed_mnemonic_list[index]+"();")
	print("\t\t\tbreak;")
	index=index+1
print("\t\tdefault:")
print("\t\t\tprintf(\"Invalid Opcode.\\n\");")
print("\t}")
print("}")