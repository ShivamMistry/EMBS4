#include "toplevel.h"

int main(void) {
    hls::stream<u32> to_hw, from_hw;

	to_hw.write(10);
	to_hw.write(10);
	to_hw.write(4);
	to_hw.write(4);
	to_hw.write(9);
	to_hw.write(0);
	to_hw.write(4);
	to_hw.write(7);
	to_hw.write(2);
	to_hw.write(6);
	to_hw.write(4);
	to_hw.write(0);
	to_hw.write(0);
	to_hw.write(0);
	to_hw.write(0);
	to_hw.write(0);
	to_hw.write(0);
	to_hw.write(0);
	to_hw.write(0);
	to_hw.write(0);
	to_hw.write(0);
	to_hw.write(0);
	to_hw.write(0);
	to_hw.write(0);
	to_hw.write(0);
	to_hw.write(0);
	to_hw.write(0);
	to_hw.write(0);
	to_hw.write(0);
	to_hw.write(0);
	to_hw.write(0);
	to_hw.write(0);
	to_hw.write(0);
	to_hw.write(1);
	to_hw.write(1);
	to_hw.write(1);
	to_hw.write(1);
	to_hw.write(0);
	to_hw.write(0);
	to_hw.write(0);
	to_hw.write(0);
	to_hw.write(0);
	to_hw.write(0);
	to_hw.write(0);
	to_hw.write(0);
	to_hw.write(0);
	to_hw.write(0);
	to_hw.write(0);
	to_hw.write(0);
	to_hw.write(0);
	to_hw.write(0);
	to_hw.write(0);
	to_hw.write(0);
	to_hw.write(0);
	to_hw.write(0);
	to_hw.write(1);
	to_hw.write(0);
	to_hw.write(0);
	to_hw.write(0);
	to_hw.write(0);
	to_hw.write(0);
	to_hw.write(0);
	to_hw.write(0);
	to_hw.write(0);
	to_hw.write(0);
	to_hw.write(1);
	to_hw.write(0);
	to_hw.write(0);
	to_hw.write(0);
	to_hw.write(1);
	to_hw.write(1);
	to_hw.write(0);
	to_hw.write(0);
	to_hw.write(1);
	to_hw.write(1);
	to_hw.write(0);
	to_hw.write(0);
	to_hw.write(0);
	to_hw.write(0);
	to_hw.write(0);
	to_hw.write(0);
	to_hw.write(0);
	to_hw.write(0);
	to_hw.write(0);
	to_hw.write(1);
	to_hw.write(0);
	to_hw.write(0);
	to_hw.write(0);
	to_hw.write(0);
	to_hw.write(0);
	to_hw.write(0);
	to_hw.write(0);
	to_hw.write(0);
	to_hw.write(0);
	to_hw.write(1);
	to_hw.write(0);
	to_hw.write(0);
	to_hw.write(0);
	to_hw.write(0);
	to_hw.write(1);
	to_hw.write(0);
	to_hw.write(0);
	to_hw.write(0);
	to_hw.write(0);
	to_hw.write(1);
	to_hw.write(0);
	to_hw.write(0);
	to_hw.write(0);
	to_hw.write(0);
	to_hw.write(0);
	to_hw.write(0);

    toplevel(to_hw, from_hw);
    int cost = 0;
    while (1) {
        int size = from_hw.read();
        if (size == 0) break;
        cost += size;
        printf("Size: %d\r\n", size);
        for (int i = 0; i < size; i++) {
        	int x = from_hw.read();
        	int y = from_hw.read();
            printf("(%d,%d), ", x,y);

        }
        printf("\r\n");
    }
    printf("Cost: %d\r\n", cost);
	return 0;
}
