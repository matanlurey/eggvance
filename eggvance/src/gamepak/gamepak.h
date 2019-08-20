#pragma once

#include <memory>
#include <string>
#include <vector>

#include "save.h"

class GamePak
{
public:
    struct Header
    {
        std::string title;
        std::string code;
        std::string maker;
    };

    GamePak(const std::string& file);

    u8  readByte(u32 addr);
    u16 readHalf(u32 addr);
    u32 readWord(u32 addr);

    std::size_t size() const;

    bool valid;
    Header header;
    std::unique_ptr<Save> save;

private:
    template<typename T>
    T read(u32 addr);

    static std::string toSaveFile(const std::string& file);
    static std::string makeString(u8* data, int size);

    void parseHeader();
    Save::Type parseSaveType();

    std::vector<u8> data;
};

