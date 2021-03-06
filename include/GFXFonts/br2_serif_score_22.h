const uint8_t br2_serif_score_22Bitmaps[] PROGMEM = {
  0x7E, 0xFF, 0xE7, 0xE7, 0xE7, 0xE7, 0xE7, 0xE7, 0xE7, 0xE7, 0xE7, 0xE7,
  0xE7, 0xE7, 0xE7, 0xE7, 0xE7, 0xE7, 0xE7, 0xE7, 0xE7, 0xE7, 0xE7, 0xE7,
  0xE7, 0xFF, 0x7E, 0x0C, 0x1C, 0x3C, 0x5C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C,
  0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C,
  0x1C, 0x1C, 0x1C, 0x1C, 0xFF, 0xFF, 0x7E, 0xFF, 0xE7, 0xE7, 0xE7, 0x67,
  0x07, 0x07, 0x07, 0x0E, 0x0E, 0x1C, 0x1C, 0x3C, 0x38, 0x38, 0x70, 0x70,
  0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE1, 0xFF, 0xFF, 0x7E, 0xFF, 0xE7,
  0xE7, 0xE7, 0x67, 0x07, 0x07, 0x07, 0x07, 0x07, 0x1F, 0x1F, 0x07, 0x07,
  0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0xC7, 0xE7, 0xE7, 0xE7, 0xFF, 0x7E,
  0xE0, 0x70, 0x38, 0x1C, 0xFE, 0x77, 0x3B, 0x9D, 0xCE, 0xE7, 0x73, 0xB9,
  0xDF, 0xEF, 0xF0, 0x38, 0x1C, 0x0E, 0x07, 0x03, 0x81, 0xC0, 0xE0, 0x70,
  0x38, 0x1C, 0x0E, 0x07, 0x03, 0x83, 0xE0, 0xFF, 0xFF, 0xE1, 0xE0, 0xE0,
  0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xFE, 0xFF, 0x07, 0x07, 0x07, 0x07,
  0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x87, 0xFF, 0x7E, 0x7E, 0xFF,
  0xE7, 0xE7, 0xE7, 0xE3, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xFE, 0xFF, 0xE7,
  0xE7, 0xE7, 0xE7, 0xE7, 0xE7, 0xE7, 0xE7, 0xE7, 0xE7, 0xE7, 0xE7, 0xFF,
  0x7E, 0xFF, 0xFF, 0xE0, 0xE0, 0x70, 0x38, 0x1C, 0x0E, 0x07, 0x07, 0x83,
  0x83, 0x81, 0xC1, 0xC0, 0xE0, 0x70, 0x38, 0x1C, 0x0E, 0x07, 0x03, 0x81,
  0xC0, 0xE0, 0x70, 0x38, 0x1C, 0x0E, 0x0F, 0x80, 0x7E, 0xFF, 0xE7, 0xE7,
  0xE7, 0xE7, 0xE7, 0xE7, 0xE7, 0xE7, 0xE7, 0xFF, 0xFF, 0xE7, 0xE7, 0xE7,
  0xE7, 0xE7, 0xE7, 0xE7, 0xE7, 0xE7, 0xE7, 0xE7, 0xE7, 0xFF, 0x7E, 0x7E,
  0xFF, 0xE7, 0xE7, 0xE7, 0xE7, 0xE7, 0xE7, 0xE7, 0xE7, 0xE7, 0xE7, 0xE7,
  0xE7, 0xFF, 0x7F, 0x07, 0x07, 0x07, 0x07, 0x07, 0xC7, 0xE7, 0xE7, 0xE7,
  0xFF, 0x7E };

const GFXglyph br2_serif_score_22Glyphs[] PROGMEM = {
  {     0,   8,  27,  12,    0,  -26 },   // 0x30 '0'
  {    27,   8,  27,  12,    0,  -26 },   // 0x31 '1'
  {    54,   8,  27,  12,    0,  -26 },   // 0x32 '2'
  {    81,   8,  27,  12,    0,  -26 },   // 0x33 '3'
  {   108,   9,  27,  12,    0,  -26 },   // 0x34 '4'
  {   139,   8,  27,  12,    0,  -26 },   // 0x35 '5'
  {   166,   8,  27,  12,    0,  -26 },   // 0x36 '6'
  {   193,   9,  27,  12,    0,  -26 },   // 0x37 '7'
  {   224,   8,  27,  12,    0,  -26 },   // 0x38 '8'
  {   251,   8,  27,  12,    0,  -26 } }; // 0x39 '9'

const GFXfont br2_serif_score_22 PROGMEM = {
  (uint8_t  *)br2_serif_score_22Bitmaps,
  (GFXglyph *)br2_serif_score_22Glyphs,
  0x30, 0x39, 48 };

// Approx. 355 bytes
