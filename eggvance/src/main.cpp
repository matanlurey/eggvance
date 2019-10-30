#include <memory>

#include "emulator.h"

int main(int argc, char* argv[])
{
    auto emulator = std::make_unique<Emulator>();
    if (!emulator->init(argc > 1 ? argv[1] : ""))
        return 1;

    emulator->run();

    return 0;
}
