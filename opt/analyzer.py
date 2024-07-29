import argparse


def count_remarks(remark, optlines):
	remark = f"[-Rpass-missed={remark}]\n"
	return sum(1 for s in optlines if s.endswith(remark))

def main():
	parser = argparse.ArgumentParser()
	parser.add_argument("filepath", type=str)

	args = parser.parse_args()

	opt_lines = []
	with open(args.filepath, 'r') as file:
		optlines = file.readlines()
		file.close()

	remarks = [
		"inline",
		"gvn",
		"licm",
		"regalloc",
		"slp-vectorizer",
		"loop-vectorize",
	]

	print()
	print("          remark    count")
	print("=========================")
	for remark in remarks:
		cnt = count_remarks(remark, optlines)
		print(f"{remark:>16} {cnt:>8}")

if __name__ == "__main__":
	main()
