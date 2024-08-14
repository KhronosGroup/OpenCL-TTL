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
    output_file_names = set()
    output_file_stream = None

    with open(input_filename, "r") as input_file_stream:
        for line in input_file_stream.readlines():
            regex = re.search("# [0-9]* \"([A-Za-z_0-9-\./]*h)\"", line)
            if regex:
                if output_file_stream:
                    output_file_stream.close()
                    output_file_stream = None

                output_file_name = os.path.relpath(os.path.abspath(regex.group(1)))
                print(output_file_name)

                if output_file_name in output_file_names:
                    output_file_stream = open(output_file_name, "r+")
                else:
                    output_path = os.path.dirname(output_file_name)

                    if (output_path != "") and (not os.path.exists(output_path)):
                        os.makedirs(output_path)
                    output_file_stream = open(output_file_name, "w+")
                    output_file_names.add(output_file_name)

                output_file_stream.seek(0, 2)
            else:
                if output_file_stream is not None:
                    output_file_stream.write(line)

    output_file_stream.close()
    output_file_stream = None

    for output_file_name in output_file_names:
        os.system("clang-format -i " + output_file_name)

if __name__ == "__main__":
    ProcessFile(sys.argv[1])
