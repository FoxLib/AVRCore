#include "avrdbg.h"

int main() {

    AVRDBG app;

    app.ds_update();

    while (app.poll()) {

        if (app.indebug()) {

            if (int k = app.keydown()) {


                printf("%c[%x] ", k, k);

            }
        }

    }

    return 0;
}
