#include <fat.cc>
#include <screen3.cc>

FAT     fat;
screen3 D;

int main() {

    D.init(); D.cls(7);
    D.println("Disk info");

    for (int i = 0; i < 4; i++) {

        D.print(i);
        D.print(" | "); D.print(fat.partitions[i].type, 16, 1);
        D.print(" | "); D.print(fat.partitions[i].lba,  16, 4);
        D.print(" | "); D.print(fat.partitions[i].size>>11 );
        D.print("\n");
    }

    for (;;);
}
