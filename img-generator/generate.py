import os, sys


class Image:

    def __init__(self, src_file):
        self.name = src_file.name
        
        self.grid = [[0] for i in range(5)]

        file_row = 0

        for line in src_file:

            if file_row >= 7:
                break

            file_column = 0

            for char in line:
                if file_column >= 5:
                    break 

                self.grid[file_column].insert(1, 1 if char == '#' else 0)
                file_column += 1

            file_row += 1

        print(self.grid)


    def hex_output():
        


image_list = []

directory = os.getcwd()

for dirpath, dirnames, files in os.walk(directory):
    for src_file in files:
        if src_file.endswith(".pixx"):
            print(src_file)
            new_image = Image(open(src_file, 'r'))
            image_list.append(new_image)




