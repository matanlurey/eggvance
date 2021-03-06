#include "ppu.h"

#include <algorithm>

#include "base/constants.h"
#include "base/macros.h"
#include "mmu/mmu.h"
#include "platform/videodevice.h"

#define MISSING_PIXEL 0x6C3F

void PPU::collapse(int begin, int end)
{
    std::vector<BGLayer> layers;
    layers.reserve(end - begin);

    for (int bg = begin; bg < end; ++bg)
    {
        if (io.dispcnt.layers & (1 << bg))
        {
            layers.emplace_back(
                io.bgcnt[bg].priority,
                backgrounds[bg].data(),
                1 << bg
            );
        }
    }

    std::sort(layers.begin(), layers.end(), std::less<BGLayer>());

    if (objects_exist)
        collapse<1>(layers);
    else
        collapse<0>(layers);
}

template<int obj_master>
void PPU::collapse(const std::vector<BGLayer>& layers)
{
    int windows = io.dispcnt.win0 || io.dispcnt.win1 || io.dispcnt.winobj;
    int effects = io.bldcnt.mode != BlendControl::Mode::DISABLED || objects_alpha;

    switch ((effects << 1) | (windows << 0))
    {
    case 0b00: collapseNN<obj_master>(layers); break;
    case 0b01: collapseNW<obj_master>(layers); break;
    case 0b10: collapseBN<obj_master>(layers); break;
    case 0b11: collapseBW<obj_master>(layers); break;

    default:
        UNREACHABLE;
        break;
    }
}

template<int obj_master>
void PPU::collapseNN(const std::vector<BGLayer>& layers)
{
    u32* scanline = &video_device->buffer[kScreenW * io.vcount];

    for (int x = 0; x < kScreenW; ++x)
    {
        scanline[x] = argb(upperLayer<obj_master>(layers, x));
    }
}

template<int obj_master>
void PPU::collapseNW(const std::vector<BGLayer>& layers)
{
    switch (possibleWindows<obj_master>())
    {
    case 0b000: collapseNW<obj_master, 0b000>(layers); break;
    case 0b001: collapseNW<obj_master, 0b001>(layers); break;
    case 0b010: collapseNW<obj_master, 0b010>(layers); break;
    case 0b011: collapseNW<obj_master, 0b011>(layers); break;
    case 0b100: collapseNW<obj_master, 0b100>(layers); break;
    case 0b101: collapseNW<obj_master, 0b101>(layers); break;
    case 0b110: collapseNW<obj_master, 0b110>(layers); break;
    case 0b111: collapseNW<obj_master, 0b111>(layers); break;

    default:
        UNREACHABLE;
        break;
    }
}

template<int obj_master, int win_master>
void PPU::collapseNW(const std::vector<BGLayer>& layers)
{
    u32* scanline = &video_device->buffer[kScreenW * io.vcount];

    for (int x = 0; x < kScreenW; ++x)
    {
        const auto& window = activeWindow<win_master>(x);

        scanline[x] = argb(upperLayer<obj_master>(layers, x, window.flags));
    }
}

template<int obj_master>
void PPU::collapseBN(const std::vector<BGLayer>& layers)
{
    switch (io.bldcnt.mode)
    {
    case 0: collapseBN<obj_master, 0>(layers); break;
    case 1: collapseBN<obj_master, 1>(layers); break;
    case 2: collapseBN<obj_master, 2>(layers); break;
    case 3: collapseBN<obj_master, 3>(layers); break;

    default:
        UNREACHABLE;
        break;
    }
}

template<int obj_master, int blend_mode>
void PPU::collapseBN(const std::vector<BGLayer>& layers)
{
    constexpr int flags = 0xFFFF;

    u32* scanline = &video_device->buffer[kScreenW * io.vcount];

    for (int x = 0; x < kScreenW; ++x)
    {
        u16 upper = MISSING_PIXEL;
        u16 lower = MISSING_PIXEL;

        const auto& object = objects[x];

        if (obj_master && object.alpha && findBlendLayers<obj_master>(layers, x, flags, upper, lower))
        {
            upper = io.bldalpha.blendAlpha(upper, lower);
        }
        else
        {
            switch (blend_mode)
            {
            case BlendControl::Mode::ALPHA:
                if (findBlendLayers<obj_master>(layers, x, flags, upper, lower))
                    upper = io.bldalpha.blendAlpha(upper, lower);
                break;

            case BlendControl::Mode::WHITE:
                if (findBlendLayers<obj_master>(layers, x, flags, upper))
                    upper = io.bldy.blendWhite(upper);
                break;

            case BlendControl::Mode::BLACK:
                if (findBlendLayers<obj_master>(layers, x, flags, upper))
                    upper = io.bldy.blendBlack(upper);
                break;

            case BlendControl::Mode::DISABLED:
                upper = upperLayer<obj_master>(layers, x);
                break;

            default:
                UNREACHABLE;
                break;
            }
        }
        scanline[x] = argb(upper);
    }
}

template<int obj_master>
void PPU::collapseBW(const std::vector<BGLayer>& layers)
{
    switch (io.bldcnt.mode)
    {
    case 0: collapseBW<obj_master, 0>(layers); break;
    case 1: collapseBW<obj_master, 1>(layers); break;
    case 2: collapseBW<obj_master, 2>(layers); break;
    case 3: collapseBW<obj_master, 3>(layers); break;

    default:
        UNREACHABLE;
        break;
    }
}

template<int obj_master, int blend_mode>
void PPU::collapseBW(const std::vector<BGLayer>& layers)
{
    switch (possibleWindows<obj_master>())
    {
    case 0b000: collapseBW<obj_master, blend_mode, 0b000>(layers); break;
    case 0b001: collapseBW<obj_master, blend_mode, 0b001>(layers); break;
    case 0b010: collapseBW<obj_master, blend_mode, 0b010>(layers); break;
    case 0b011: collapseBW<obj_master, blend_mode, 0b011>(layers); break;
    case 0b100: collapseBW<obj_master, blend_mode, 0b100>(layers); break;
    case 0b101: collapseBW<obj_master, blend_mode, 0b101>(layers); break;
    case 0b110: collapseBW<obj_master, blend_mode, 0b110>(layers); break;
    case 0b111: collapseBW<obj_master, blend_mode, 0b111>(layers); break;

    default:
        UNREACHABLE;
        break;
    }
}

template<int obj_master, int blend_mode, int win_master>
void PPU::collapseBW(const std::vector<BGLayer>& layers)
{
    u32* scanline = &video_device->buffer[kScreenW * io.vcount];

    for (int x = 0; x < kScreenW; ++x)
    {
        u16 upper = MISSING_PIXEL;
        u16 lower = MISSING_PIXEL;

        const auto& object = objects[x];
        const auto& window = activeWindow<win_master>(x);

        if (obj_master && object.alpha && findBlendLayers<obj_master>(layers, x, window.flags, upper, lower))
        {
            upper = io.bldalpha.blendAlpha(upper, lower);
        }
        else if (window.blend)
        {
            switch (blend_mode)
            {
            case BlendControl::Mode::ALPHA:
                if (findBlendLayers<obj_master>(layers, x, window.flags, upper, lower))
                    upper = io.bldalpha.blendAlpha(upper, lower);
                break;

            case BlendControl::Mode::WHITE:
                if (findBlendLayers<obj_master>(layers, x, window.flags, upper))
                    upper = io.bldy.blendWhite(upper);
                break;

            case BlendControl::Mode::BLACK:
                if (findBlendLayers<obj_master>(layers, x, window.flags, upper))
                    upper = io.bldy.blendBlack(upper);
                break;

            case BlendControl::Mode::DISABLED:
                upper = upperLayer<obj_master>(layers, x, window.flags);
                break;

            default:
                UNREACHABLE;
                break;
            }
        }
        else
        {
            if (!(obj_master && object.alpha))
                upper = upperLayer<obj_master>(layers, x, window.flags);
        }
        scanline[x] = argb(upper);
    }
}

template<int obj_master>
int PPU::possibleWindows() const
{
    int windows = 0;
    if (io.dispcnt.win0 && io.winv[0].contains(io.vcount))
        windows |= WF_WIN0;
    if (io.dispcnt.win1 && io.winv[1].contains(io.vcount))
        windows |= WF_WIN1;
    if (io.dispcnt.winobj && obj_master)
        windows |= WF_WINOBJ;

    return windows;
}

template<int win_master>
const Window& PPU::activeWindow(int x) const
{
    if (win_master & WF_WIN0 && io.winh[0].contains(x))
        return io.winin.win0;

    if (win_master & WF_WIN1 && io.winh[1].contains(x))
        return io.winin.win1;

    if (win_master & WF_WINOBJ && objects[x].window)
        return io.winout.winobj;

    return io.winout.winout;
}

template<int obj_master>
u16 PPU::upperLayer(const std::vector<BGLayer>& layers, int x)
{
    const auto& object = objects[x];

    for (const auto& layer : layers)
    {
        if (obj_master && object.opaque() && object <= layer)
            return object.color;

        if (layer.opaque(x))
            return layer.color(x);
    }
    if (obj_master && object.opaque())
        return object.color;

    return mmu.palette.backdrop();
}

template<int obj_master>
u16 PPU::upperLayer(const std::vector<BGLayer>& layers, int x, int flags)
{    
    const auto& object = objects[x];

    for (const auto& layer : layers)
    {
        if (obj_master && flags & LF_OBJ && object.opaque() && object <= layer)
            return object.color;

        if (flags & layer.flag && layer.opaque(x))
            return layer.color(x);
    }
    if (obj_master && flags & LF_OBJ && object.opaque())
        return object.color;

    return mmu.palette.backdrop();
}

template<int obj_master>
bool PPU::findBlendLayers(const std::vector<BGLayer>& layers, int x, int flags, u16& upper)
{
    const auto& object = objects[x];

    int flags_upper = object.alpha ? LF_OBJ : io.bldcnt.upper;

    for (const auto& layer : layers)
    {
        if (obj_master && flags & LF_OBJ && object.opaque() && object <= layer)
        {
            upper = object.color;
            return flags_upper & LF_OBJ;
        }
        if (flags & layer.flag && layer.opaque(x))
        {
            upper = layer.color(x);
            return flags_upper & layer.flag;
        }
    }
    if (obj_master && flags & LF_OBJ && object.opaque())
    {
        upper = object.color;
        return flags_upper & LF_OBJ;
    }
    upper = mmu.palette.backdrop();
    return flags_upper & LF_BDP;
}

#define PROCESS_BLEND_LAYER(color, flag)  \
    if (upper_found)                      \
    {                                     \
        lower = color;                    \
        return flags_lower & flag;        \
    }                                     \
    else                                  \
    {                                     \
        upper = color;                    \
        upper_found = true;               \
        if (~flags_upper & flag)          \
            return false;                 \
    }

template<int obj_master>
bool PPU::findBlendLayers(const std::vector<BGLayer>& layers, int x, int flags, u16& upper, u16& lower)
{
    const auto& object = objects[x];

    int flags_upper = object.alpha ? LF_OBJ : io.bldcnt.upper;
    int flags_lower = io.bldcnt.lower;

    bool upper_found = false;
    bool object_used = false;

    for (const auto& layer : layers)
    {
        if (obj_master && flags & LF_OBJ && !object_used && object.opaque() && object <= layer)
        {
            PROCESS_BLEND_LAYER(object.color, LF_OBJ);
            object_used = true;
        }
        if (flags & layer.flag && layer.opaque(x))
        {
            PROCESS_BLEND_LAYER(layer.color(x), layer.flag);
        }
    }
    if (obj_master && flags & LF_OBJ && !object_used && object.opaque())
    {
        PROCESS_BLEND_LAYER(object.color, LF_OBJ);
    }
    if (upper_found)
    {
        lower = mmu.palette.backdrop();
        return flags_lower & LF_BDP;
    }
    else
    {
        upper = mmu.palette.backdrop();
        return false;
    }
}
