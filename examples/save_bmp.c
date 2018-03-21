#include "../graphics.h"

int main(void) {
	surface_t* out = surface(100, 100);
	fill(out, BLACK);
	print(out, 2, 2, WHITE, "hello");
	save_bmp(out, "save_bmp.bmp");
	return 0;
}
