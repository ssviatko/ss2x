#include <iostream>
#include <iomanip>

#include "log.h"

int main(int argc, char **argv)
{
	std::cout << "standard PC colors    ";
	std::cout << ss::color_rgb_blend("                                              ", 255, 232, 32, 240, 48, 255, true);
	std::cout << "     Standard GS colors    ";
	std::cout << ss::color_rgb_blend("                                              ", 10, 255, 255, 144, 95, 0, true);	
	std::cout << std::endl;
	for (int i = 0;  i < 16; ++i) {
		std::cout << ss::color[i] << " * " << ss::COLOR_DEFAULT;
	}
	std::cout << "                         ";
	for (unsigned int i = 0;  i < 16; ++i) {
		std::cout << ss::color_gs(i) << " * " << ss::COLOR_DEFAULT;
	}
	std::cout << std::endl;
	for (int i = 0;  i < 16; ++i) {
		std::cout << ss::color_bg[i] << ss::color[15] << " * " << ss::COLOR_DEFAULT;
	}
	std::cout << "                         ";
	for (unsigned int i = 0;  i < 16; ++i) {
		std::cout << ss::color_gs_bg(i) << ss::color_gs(15) << " * " << ss::COLOR_DEFAULT;
	}
	std::cout << std::endl;
	std::cout << ss::COLOR_RED <<          "this is a nice shade of dark red      " << ss::COLOR_DEFAULT;
	std::cout << ss::COLOR_BG_RED << ss::COLOR_BLACK <<          "black on dark red background     " << ss::COLOR_DEFAULT << "  ";
	std::cout << ss::color_gs(ss::color_gs_name("DEEPRED")) <<    "This is a nice shade of deep red    " << ss::COLOR_DEFAULT;
	std::cout << ss::color_gs_bg(ss::color_gs_name("DEEPRED")) << ss::color_gs(0) <<    "This is a nice shade of deep red    " << ss::COLOR_DEFAULT << std::endl;
	std::cout << ss::COLOR_GREEN <<        "this is a nice shade of dark green    " << ss::COLOR_DEFAULT;
	std::cout << ss::COLOR_BG_GREEN << ss::COLOR_BLACK <<        "black on dark green background   " << ss::COLOR_DEFAULT << "  ";
	std::cout << ss::color_gs(ss::color_gs_name("DARKBLUE")) <<   "This is a nice shade of dark blue   " << ss::COLOR_DEFAULT;
	std::cout << ss::color_gs_bg(ss::color_gs_name("DARKBLUE")) << ss::color_gs(0) <<   "This is a nice shade of dark blue   " << ss::COLOR_DEFAULT << std::endl;
	std::cout << ss::COLOR_YELLOW <<       "this is a nice shade of yellow        " << ss::COLOR_DEFAULT;
	std::cout << ss::COLOR_BG_YELLOW << ss::COLOR_BLACK <<       "black on dark yellow background  " << ss::COLOR_DEFAULT << "  ";
	std::cout << ss::color_gs(ss::color_gs_name("PURPLE")) <<     "This is a nice shade of purple      " << ss::COLOR_DEFAULT;
	std::cout << ss::color_gs_bg(ss::color_gs_name("PURPLE")) << ss::color_gs(0) <<     "This is a nice shade of purple      " << ss::COLOR_DEFAULT << std::endl;
	std::cout << ss::COLOR_BLUE <<         "this is a nice shade of dark blue     " << ss::COLOR_DEFAULT;
	std::cout << ss::COLOR_BG_BLUE << ss::COLOR_BLACK <<         "black on dark blue background    " << ss::COLOR_DEFAULT << "  ";
	std::cout << ss::color_gs(ss::color_gs_name("DARKGREEN")) <<  "This is a nice shade of dark green  " << ss::COLOR_DEFAULT;
	std::cout << ss::color_gs_bg(ss::color_gs_name("DARKGREEN")) << ss::color_gs(0) <<  "This is a nice shade of dark green  " << ss::COLOR_DEFAULT << std::endl;
	std::cout << ss::COLOR_MAGENTA <<      "this is a nice shade of dark magenta  " << ss::COLOR_DEFAULT;
	std::cout << ss::COLOR_BG_MAGENTA << ss::COLOR_BLACK <<      "black on dark magenta background " << ss::COLOR_DEFAULT << "  ";
	std::cout << ss::color_gs(ss::color_gs_name("DARKGRAY")) <<   "This is a nice shade of dark gray   " << ss::COLOR_DEFAULT;
	std::cout << ss::color_gs_bg(ss::color_gs_name("DARKGRAY")) << ss::color_gs(0) <<   "This is a nice shade of dark gray   " << ss::COLOR_DEFAULT << std::endl;
	std::cout << ss::COLOR_CYAN <<         "this is a nice shade of dark cyan     " << ss::COLOR_DEFAULT;
	std::cout << ss::COLOR_BG_CYAN << ss::COLOR_BLACK <<         "black on dark cyan background    " << ss::COLOR_DEFAULT << "  ";
	std::cout << ss::color_gs(ss::color_gs_name("MEDIUMBLUE")) << "This is a nice shade of medium blue " << ss::COLOR_DEFAULT;
	std::cout << ss::color_gs_bg(ss::color_gs_name("MEDIUMBLUE")) << ss::color_gs(0) << "This is a nice shade of medium blue " << ss::COLOR_DEFAULT << std::endl;
	std::cout << ss::COLOR_LIGHTGRAY <<    "this is a nice shade of light gray    " << ss::COLOR_DEFAULT;
	std::cout << ss::COLOR_BG_LIGHTGRAY << ss::COLOR_BLACK <<    "black on light gray background   " << ss::COLOR_DEFAULT << "  ";
	std::cout << ss::color_gs(ss::color_gs_name("LIGHTBLUE")) <<  "This is a nice shade of light blue  " << ss::COLOR_DEFAULT;
	std::cout << ss::color_gs_bg(ss::color_gs_name("LIGHTBLUE")) << ss::color_gs(0) <<  "This is a nice shade of light blue  " << ss::COLOR_DEFAULT << std::endl;
	std::cout << ss::COLOR_DARKGRAY <<     "this is a nice shade of dark gray     " << ss::COLOR_DEFAULT;
	std::cout << ss::COLOR_BG_DARKGRAY << ss::COLOR_BLACK <<     "black on dark gray background    " << ss::COLOR_DEFAULT << "  ";
	std::cout << ss::color_gs(ss::color_gs_name("BROWN")) <<      "This is a nice shade of brown       " << ss::COLOR_DEFAULT;
	std::cout << ss::color_gs_bg(ss::color_gs_name("BROWN")) << ss::color_gs(0) <<      "This is a nice shade of brown       " << ss::COLOR_DEFAULT << std::endl;
	std::cout << ss::COLOR_LIGHTRED <<     "this is a nice shade of light red     " << ss::COLOR_DEFAULT;
	std::cout << ss::COLOR_BG_LIGHTRED << ss::COLOR_BLACK <<     "black on light red background    " << ss::COLOR_DEFAULT << "  ";
	std::cout << ss::color_gs(ss::color_gs_name("ORANGE")) <<     "This is a nice shade of orange      " << ss::COLOR_DEFAULT;
	std::cout << ss::color_gs_bg(ss::color_gs_name("ORANGE")) << ss::color_gs(0) <<     "This is a nice shade of orange      " << ss::COLOR_DEFAULT << std::endl;
	std::cout << ss::COLOR_LIGHTGREEN <<   "this is a nice shade of light green   " << ss::COLOR_DEFAULT;
	std::cout << ss::COLOR_BG_LIGHTGREEN << ss::COLOR_BLACK <<   "black on light green background  " << ss::COLOR_DEFAULT << "  ";
	std::cout << ss::color_gs(ss::color_gs_name("LIGHTGRAY")) <<  "This is a nice shade of light gray  " << ss::COLOR_DEFAULT;
	std::cout << ss::color_gs_bg(ss::color_gs_name("LIGHTGRAY")) << ss::color_gs(0) <<  "This is a nice shade of light gray  " << ss::COLOR_DEFAULT << std::endl;
	std::cout << ss::COLOR_LIGHTYELLOW <<  "this is a nice shade of light yellow  " << ss::COLOR_DEFAULT;
	std::cout << ss::COLOR_BG_LIGHTYELLOW << ss::COLOR_BLACK <<  "black on light yellow background " << ss::COLOR_DEFAULT << "  ";
	std::cout << ss::color_gs(ss::color_gs_name("PINK")) <<       "This is a nice shade of pink        " << ss::COLOR_DEFAULT;
	std::cout << ss::color_gs_bg(ss::color_gs_name("PINK")) << ss::color_gs(0) <<       "This is a nice shade of pink        " << ss::COLOR_DEFAULT << std::endl;
	std::cout << ss::COLOR_LIGHTBLUE <<    "this is a nice shade of light blue    " << ss::COLOR_DEFAULT;
	std::cout << ss::COLOR_BG_LIGHTBLUE << ss::COLOR_BLACK <<    "black on light blue background   " << ss::COLOR_DEFAULT << "  ";
	std::cout << ss::color_gs(ss::color_gs_name("LIGHTGREEN")) << "This is a nice shade of light green " << ss::COLOR_DEFAULT;
	std::cout << ss::color_gs_bg(ss::color_gs_name("LIGHTGREEN")) << ss::color_gs(0) << "This is a nice shade of light green " << ss::COLOR_DEFAULT << std::endl;
	std::cout << ss::COLOR_LIGHTMAGENTA << "this is a nice shade of light magenta " << ss::COLOR_DEFAULT;
	std::cout << ss::COLOR_BG_LIGHTMAGENTA << ss::COLOR_BLACK << "black on light magenta background" << ss::COLOR_DEFAULT << "  ";
	std::cout << ss::color_gs(ss::color_gs_name("YELLOW")) <<     "This is a nice shade of yellow      " << ss::COLOR_DEFAULT;
	std::cout << ss::color_gs_bg(ss::color_gs_name("YELLOW")) << ss::color_gs(0) <<     "This is a nice shade of yellow      " << ss::COLOR_DEFAULT << std::endl;
	std::cout << ss::COLOR_LIGHTCYAN <<    "this is a nice shade of light cyan    " << ss::COLOR_DEFAULT;
	std::cout << ss::COLOR_BG_LIGHTCYAN << ss::COLOR_BLACK <<    "black on light cyan background   " << ss::COLOR_DEFAULT << "  ";
	std::cout << ss::color_gs(ss::color_gs_name("AQUAMARINE")) << "This is a nice shade of aquamarine  " << ss::COLOR_DEFAULT;
	std::cout << ss::color_gs_bg(ss::color_gs_name("AQUAMARINE")) << ss::color_gs(0) << "This is a nice shade of aquamarine  " << ss::COLOR_DEFAULT << std::endl;
	std::cout << ss::COLOR_WHITE <<        "this is a nice shade of white         " << ss::COLOR_DEFAULT;
	std::cout << ss::COLOR_BG_WHITE << ss::COLOR_BLACK <<        "black on white background        " << ss::COLOR_DEFAULT << "  ";
	std::cout << ss::color_gs(ss::color_gs_name("WHITE")) <<      "This is a nice shade of white       " << ss::COLOR_DEFAULT;
	std::cout << ss::color_gs_bg(ss::color_gs_name("WHITE")) << ss::color_gs(0) <<      "This is a nice shade of white       " << ss::COLOR_DEFAULT << std::endl;
	
	std::cout << "256 colors    ";
	std::cout << ss::color_rgb_blend("This is a cool demonstration of a gradual blend from color #90 (a tasty shade of purple) to color #214 (a luscious shade of orange)",
									102, 0, 102, 255, 153, 0) << std::endl;
	for (unsigned int j = 0; j < 8; ++j) {
		for (unsigned int i = 0; i < 32; ++i) {
			std::cout << ss::color_256(i + (32 * j)) << " " << std::setw(3) << std::setfill('0') << (i + (32 * j))  << " " << ss::COLOR_DEFAULT;
		}
		std::cout << std::endl;
	}
	for (unsigned int j = 0; j < 8; ++j) {
		for (unsigned int i = 0; i < 32; ++i) {
			std::cout << ss::color_256_bg(i + (32 * j)) << ss::color_256(15) << " " << std::setw(3) << std::setfill('0') << i + (32 * j) << " " << ss::COLOR_DEFAULT;
		}
		std::cout << std::endl;
	}
	
	std::cout << ss::color_rgb_blend("RGB colors (abridged to 216) with another demonstration of blending", 255, 0, 0, 0, 0, 255);
	std::cout << "      ";
	std::cout << ss::color_rgb_blend("Another demonstration of blending from a deep yellow to a rich green", 234, 255, 0, 0, 166, 32);
	std::cout << std::endl;
	for (unsigned int r = 0; r <= 0xff; r += 51) {
		for (unsigned int g = 0; g <= 0xff; g += 51) {
			for (unsigned int b = 0; b <= 0xff; b += 51) {
				std::cout << ss::color_rgb(r, g, b) << "@@" << ss::COLOR_DEFAULT;
			}
		}
		std::cout << " ";
		for (unsigned int g = 0; g <= 0xff; g += 51) {
			for (unsigned int b = 0; b <= 0xff; b += 51) {
				std::cout << ss::color_rgb_bg(r, g, b) << "  " << ss::COLOR_DEFAULT;
			}
		}
		std::cout << std::endl;
	}
// check fix for "off by one" error in blend algorithm
//	std::cout << ss::color_rgb_blend("%%", 255, 0, 0, 0, 0, 255);
//	std::cout << ss::color_rgb_blend("%%%", 255, 0, 0, 0, 0, 255);
//	std::cout << ss::color_rgb_blend("%%%%", 255, 0, 0, 0, 0, 255);
//	std::cout << ss::color_rgb_blend("%%%%%", 255, 0, 0, 0, 0, 255);
//	std::cout << ss::color_rgb_blend("%%%%%%", 255, 0, 0, 0, 0, 255);
	return 0;
}