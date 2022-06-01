#include <stdio.h>
int foo(){
    printf("OK\n");
}

int bar(int x, int y){
    printf("%d\n", x + y);
}

int bar2(int x, int y, int z) {
    printf("%d\n", x+y+z);
}