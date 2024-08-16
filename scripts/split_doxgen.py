#!/usr/bin/env python3

# To update the generated files From the src directory run this command.
# in the src directory
#
# find . -name *.h -exec python ../scripts/generate_enum_strings.py {} \;
#
# The script generate_enum_strings.sh will do this for you.

import sys
import os
import re

def ProcessFile(input_filename):
	exclude_files = []
	output_filenames = dict()
	output_file_stream = None
	output_filename = None

	with open(input_filename, "r") as input_file_stream:
		for line in input_file_stream.readlines():
			regex = re.search("# [0-9]* \"([A-Za-z_0-9-\./]*h)\"", line)
			if regex:
				last_output_filename = output_filename

				if output_file_stream:
					output_file_stream.close()
					output_file_stream = None

				output_filename = os.path.relpath(os.path.abspath(regex.group(1)))

				if output_filename not in exclude_files:
					if output_filename in output_filenames:
						output_file_stream = open(output_filename, "r+")
					else:
						output_path = os.path.dirname(output_filename)

						if (output_path != "") and (not os.path.exists(output_path)):
							os.makedirs(output_path)
						output_file_stream = open(output_filename, "w+")
						output_file_stream.write(" " * 2000) # Space to insert #pragma etc later.
						output_filenames[output_filename] = set()

					if last_output_filename is not None:
						output_filenames[last_output_filename].add(output_filename)

					output_file_stream.seek(0, 2)
			else:
				if output_file_stream is not None:
					output_file_stream.write(line)

	output_file_stream.close()
	output_file_stream = None

	for output_filename in output_filenames:
		with open(output_filename, "r+") as output_file_stream:
			output_file_stream.seek(0, 0)
			output_file_stream.write("#pragma once\n\n")

			for include_filename in output_filenames[output_filename]:
				output_file_stream.write("#include \"" + include_filename + "\"\n")

		os.system("clang-format -i " + output_filename)

	for output_filename in exclude_files:
		os.remove(output_filename)


if __name__ == "__main__":
	ProcessFile(sys.argv[1])
