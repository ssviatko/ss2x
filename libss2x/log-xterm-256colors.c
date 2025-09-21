#include <stdio.h>
#include <stdint.h>

const uint8_t g_standard_colors[][3] = {
    { 0x00, 0x00, 0x00 }, // Black
    { 0xdd, 0x00, 0x33 }, // Deep Red
    { 0x00, 0x00, 0x99 }, // Dark Blue
    { 0xdd, 0x22, 0xdd }, // Purple
    { 0x00, 0x77, 0x22 }, // Dark Green
    { 0x55, 0x55, 0x55 }, // Dark Gray
    { 0x22, 0x22, 0xff }, // Medium Blue
    { 0x66, 0xaa, 0xff }, // Light Blue
    { 0x88, 0x55, 0x00 }, // Brown
    { 0xff, 0x66, 0x00 }, // Orange
    { 0xaa, 0xaa, 0xaa }, // Light Gray
    { 0xff, 0x99, 0x88 }, // Pink
    { 0x11, 0xdd, 0x00 }, // Light Green
    { 0xff, 0xff, 0x00 }, // Yellow
    { 0x44, 0xff, 0x99 }, // Aquamarine
    { 0xff, 0xff, 0xff }  // White
};

void set_gs_fg(unsigned int a_color)
{
	printf("\033[38;2;%d;%d;%dm", g_standard_colors[a_color][0], g_standard_colors[a_color][1], g_standard_colors[a_color][2]);
}

void set_gs_bg(unsigned int a_color)
{
	printf("\033[48;2;%d;%d;%dm", g_standard_colors[a_color][0], g_standard_colors[a_color][1], g_standard_colors[a_color][2]);
}

void set_default()
{
	printf("\033[39m\033[49m");
}

int main(int argc, char **argv)
{
	int i, j, color;
	for (i = 0; i < 8; ++i) {
		for (j = 0; j < 32; ++j) {
			color = ((i * 32) + j);
			printf("\033[38;5;%dm(%03d)", color, color);
		}
		printf("\n");
	}
	printf("\n");
	for (i = 0; i < 8; ++i) {
		for (j = 0; j < 32; ++j) {
			color = ((i * 32) + j);
			printf("\033[48;5;%dm\033[38;5;0m(%03d)", color, color);
		}
		printf("\033[0m\n");
	}
	printf("\n");
	for (i = 0; i < 8; ++i) {
		for (j = 0; j < 32; ++j) {
			color = ((i * 32) + j);
			printf("\033[48;5;%dm\033[38;5;15m(%03d)", color, color);
		}
		printf("\033[0m\n");
	}
	printf("\n");
	printf("GS Colors:\n");
	for (i = 0; i < 16; ++i) {
		set_gs_bg(0);
		set_gs_fg(i);
		printf(" GS color #%02d abcdefgjijklmnopqrstuvwxyz ", i);
		set_gs_bg(10);
		printf(" GS color #%02d abcdefghijklmnopqrstuvwxyz ", i);
		set_default();
		printf(" ");
		set_gs_bg(i);
		set_gs_fg(0);
		printf(" GS color #%02d abcdefghijklmnopqrstuvwxyz ", i);
		set_default();
		printf("\n");
	};
	set_default();
	return 0;
}

