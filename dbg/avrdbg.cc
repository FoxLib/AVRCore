#include "avrdbg.h"

int main() {

    AVRDBG app;

    app.ds_update();

    while (app.poll()) {

    }

    return 0;
}
