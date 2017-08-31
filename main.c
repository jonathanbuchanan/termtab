#include "draw.h"

int main(int argc, char **argv) {
    struct Window *w = init_window();
    sleep(5);
    kill_window(w);
    return 0;
}
