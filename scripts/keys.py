import random


outfile = "keys.txt"

with open(outfile, 'w') as file:
	file.truncate()
	file.close()

# piece square keys
for i in range(16):
	if (i in [0, 7, 8, 15]):
		keys = ["0"] * 64
	else:
		keys = [random.getrandbits(64) for _ in range(64)]
		keys = [f"0x{key:016x}" for key in keys]

	with open(outfile, 'a') as file:
		for j in range(0, 64, 8):
			line = ", ".join(keys[j:j+8]) + ','
			file.write(line + '\n')
		file.write('\n')
		file.close()
	
# side to move key
with open(outfile, 'a') as file:
	file.write(f"0x{random.getrandbits(64):016x}\n\n")
	file.close()

# castling rights keys
with open(outfile, 'a') as file:
	file.write(f"0x{random.getrandbits(64):016x}\n")
	file.write(f"0x{random.getrandbits(64):016x}\n")
	file.write(f"0x{random.getrandbits(64):016x}\n")
	file.write(f"0x{random.getrandbits(64):016x}\n\n")
	file.close()

# en passant keys
with open(outfile, 'a') as file:
	file.write(f"0x{random.getrandbits(64):016x}\n")
	file.write(f"0x{random.getrandbits(64):016x}\n")
	file.write(f"0x{random.getrandbits(64):016x}\n")
	file.write(f"0x{random.getrandbits(64):016x}\n")
	file.write(f"0x{random.getrandbits(64):016x}\n")
	file.write(f"0x{random.getrandbits(64):016x}\n")
	file.write(f"0x{random.getrandbits(64):016x}\n")
	file.write(f"0x{random.getrandbits(64):016x}\n")
	file.close()
