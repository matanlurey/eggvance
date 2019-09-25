#pragma once

#include "common/integer.h"
#include "constants.h"

enum LayerFlag
{
    LF_BG0 = 1 << 0,
    LF_BG1 = 1 << 1,
    LF_BG2 = 1 << 2,
    LF_BG3 = 1 << 3,
    LF_OBJ = 1 << 4,
    LF_BDP = 1 << 5
};

struct Layer
{
    Layer(int priority) 
        : priority(priority)
    { 
    
    }

    bool operator<(const Layer& other) const
    {
        return priority < other.priority;
    }

    bool operator<=(const Layer& other) const
    {
        return priority <= other.priority;
    }

    int priority;
};

struct BackgroundLayer : public Layer
{
    BackgroundLayer(int id, u16* data, int priority)
        : Layer(priority)
        , id(id)
        , data(data)
        , flag(1 << id)
    {

    }

    u16 color(int x) const
    {
        return data[x];
    }

    bool opaque(int x) const
    {
        return color(x) != TRANSPARENT;
    }

    int  id;
    u16* data;
    int  flag;
};

struct ObjectLayer : public Layer
{
    ObjectLayer()
        : Layer(4)
        , color(TRANSPARENT)
        , opaque(false)
        , window(false)
        , alpha(false)
    {

    }

    u16  color;
    bool opaque;
    bool window;
    bool alpha;
};