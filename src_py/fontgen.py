#
# FontGen
# Author: Jared Sanson (jared@jared.geek.nz)
#
# Requires Python Imaging Library (PIL)
#
# To add a new font, create a new FONT dictionary and tweak the parameters
# until the output .png looks correct.
# Monospaced fonts work best, but some variable-width ones work well too.
#
# Once the png file looks good, you can simply include the .h file in your
# project and use it. (how you use it is up to you)
#

# Edited to make each byte in the header file support rows instead of columns,
# allowing for glyph height > 8
# - Angelo

from PIL import Image, ImageFont, ImageDraw
import os.path

# MONOSPACE:
FONT = {'fname': r'AtariST.ttf', 'size': 16, 'yoff': 0, 'xoff': -1, 'w': 8, 'h': 16}
# FONT = {'fname': r'3x7-pixels.ttf', 'size': 8, 'yoff': 0, 'xoff': 0, 'w': 4, 'h': 8}

FONT_FILE = FONT['fname']
FONT_SIZE = FONT['size']
FONT_Y_OFFSET = FONT.get('yoff', 0)
FONT_X_OFFSET = FONT.get('xoff', 0)

CHAR_WIDTH = FONT.get('w', 5)
CHAR_HEIGHT = FONT.get('h', 8)

FONT_BEGIN = ' '
FONT_END = '~'
FONTSTR = ''.join(chr(x) for x in range(ord(FONT_BEGIN), ord(FONT_END)+1))

OUTPUT_NAME = os.path.splitext(FONT_FILE)[0] + '_font'
OUTPUT_PNG = OUTPUT_NAME + '.png'
OUTPUT_H = OUTPUT_NAME + '.h'

GLYPH_WIDTH = CHAR_WIDTH + 1

WIDTH = GLYPH_WIDTH * len(FONTSTR)
HEIGHT = CHAR_HEIGHT

img = Image.new("RGBA", (WIDTH, HEIGHT), (255,255,255))
fnt = ImageFont.truetype(FONT_FILE, FONT_SIZE)

drw = ImageDraw.Draw(img)
#drw.fontmode = 1

for i in range(len(FONTSTR)):
    drw.text((i*GLYPH_WIDTH+FONT_X_OFFSET,FONT_Y_OFFSET), FONTSTR[i], (0,0,0), font=fnt)

img.save(OUTPUT_PNG)

#### Convert to C-header format
f = open(OUTPUT_H, 'w')
num_chars = len(FONTSTR)
f.write('const unsigned char font[%d][%d] = {\n' % (num_chars+1, CHAR_HEIGHT))

chars = []
for i in range(num_chars):
    ints = []
    for y in range(CHAR_HEIGHT):
        val = 0
        for relative_x in range(CHAR_WIDTH):
            absolute_x = i*GLYPH_WIDTH + relative_x
            rgb = img.getpixel((absolute_x, y))
            val = (val << 1) | (1 if rgb[0] == 0 else 0)

        ints.append('0x%.2x' % (val))
    c = FONTSTR[i]
    if c == '\\': c = '"\\"' # bugfix
    f.write('\t{%s}, // %s\n' % (','.join(ints), c))

# for i in range(num_chars):
#     ints = []
#     for j in range(CHAR_WIDTH):
#         x = i*GLYPH_WIDTH + j
#         val = 0
#         for y in range(CHAR_HEIGHT):
#             rgb = img.getpixel((x,y))
#             val = (val >> 1) | (0x80 if rgb[0] == 0 else 0)

#         ints.append('0x%.2x' % (val))
#     c = FONTSTR[i]
#     if c == '\\': c = '"\\"' # bugfix
#     f.write('\t{%s}, // %s\n' % (','.join(ints), c))


f.write('\t{%s}\n' % (','.join(['0x00']*CHAR_WIDTH)))
f.write('};\n\n')

f.write('#define FONT_NAME "%s"\n' % OUTPUT_NAME)

f.close()
