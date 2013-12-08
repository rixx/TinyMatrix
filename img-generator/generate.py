"""
Module to build hex-numbers out of small ASCII art images for the TinyMatrix
project

"""
import os


class Image(object):
    """
    The Image class contains an image in the already transformed matrix required
    by the TinyMatrix project
    """

    def __init__(self, src_file):
        """
        the constructor takes a read-opened file and parses it
        """
        self.name = src_file.name[:-5]
        self.grid = [['0'] for i in range(5)]
        file_row = 0

        for line in src_file:
            file_column = 0

            if file_row >= 7:
                break

            for char in line:
                if file_column >= 5:
                    break

                self.grid[file_column].insert(1, '1' if char == '#' else '0')
                file_column += 1

            file_row += 1

        src_file.close()


    def hex_output(self):
        """
        returns the output string for an image, five comma-separated hex-numbers
        in braces
        """
        output = "{ "
        output_list = []

        for row in self.grid:
            output_list.append(str(hex(int(''.join(row), 2))))

        output += ", ".join(output_list)
        output += " },   //"
        output += self.name
        return output


IMAGE_LIST = []

WORKING_DIR = os.getcwd()

for dirpath, dirnames, files in os.walk(WORKING_DIR):
    for file in files:
        if file.endswith(".pixx"):
            new_image = Image(open(file, 'r'))
            IMAGE_LIST.append(new_image)

for image in IMAGE_LIST:
    print(image.hex_output())


