#include "avrio.cc"

class graphics {
protected:

    byte mode;

public:

    void screen(byte _mode) { mode = _mode; outp(VIDEOMODE, _mode); }
};
