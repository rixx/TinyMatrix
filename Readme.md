TinyMatrix
----------

TinyMatrix is a project by TigerUp and can be found [here](https://sites.google.com/site/tinymatrix/). 

I just modified it a little bit - made it a little bit smaller so people can add their own pictures or animations and still have it run on an ATtiny2313.

I also added a script that generates the required hex sequences from ASCII art files.



How to add static pictures
--------------------------

Write an ASCII file with the image you want. `#` signifies a lit LED. Take care to have at least five characters in every row, else the image generator will not work as expected. Save the file(s) with the extension `.pixx` in the img-generator folder.

From that folder, call `generate.py`. **Note:** Python 3 is required to execute this script.

Copy the output lines into the array that is `const unsigned char charset[CHARS][5] PROGMEM` around line 77 in `src/main.c`. A line above that, modify `define CHARS 15` to represent the new size of the array.

Continue at "How to change the image order" or else your image will not appear.



How to add animations
---------------------

Have a look at the `render_*()` functions in `src/main.c` and build one like it. `bitmap[a][b]` refers to the dot matrix, `render_character(i)` renders the image found at position `i` in the `charset` array, `clear_bitmap()` does exactly that.

Continue at "How to change the image order" or else your animation will not appear.



How to change the image order
-----------------------------

Change it around int the render_buffer() function. You can either call a function or render an image by calling `render_character(i)` which renders the image at index `i` of the `charset` array. 

Adjust the `define MODES` statement to include the new number of modes.

To change the very first animation/image, change the `mode` variable in the `init()` function.


