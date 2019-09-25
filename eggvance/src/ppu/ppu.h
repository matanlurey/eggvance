#pragma once

#include "mmu/mmu.h"
#include "backend.h"
#include "buffer.h"
#include "layers.h"

class PPU
{
public:
    PPU(MMU& mmu);

    void reset();

    void scanline();
    void hblank();
    void vblank();
    void next();

    void present();

    Backend backend;

private:
    enum WindowFlag
    {
        WF_WIN0   = 1 << 0,
        WF_WIN1   = 1 << 1,
        WF_WINOBJ = 1 << 2
    };
    enum Palette
    {
        P_16_16,  // 16 colors in 16 banks
        P_256_1   // 256 colors in 1 bank
    };
    MMU& mmu;
    MMIO& mmio;

    using RenderFunc = void(PPU::*)(int);

    void renderBg(RenderFunc func, int bg);
    void renderBgMode0(int bg);
    void renderBgMode2(int bg);
    void renderBgMode3(int bg);
    void renderBgMode4(int bg);
    void renderBgMode5(int bg);
    void renderObjects();

    void mosaic(int bg);
    bool mosaicAffected(int bg) const;
    bool mosaicDominant() const;

    void collapse(int begin, int end);
    template<int obj_master>
    void collapse(const std::vector<BackgroundLayer>& layers);

    template<int obj_master>
    void collapseNN(const std::vector<BackgroundLayer>& layers);
    template<int obj_master>
    void collapseNW(const std::vector<BackgroundLayer>& layers);
    template<int obj_master, int win_master>
    void collapseNW(const std::vector<BackgroundLayer>& layers);
    template<int obj_master>
    void collapseBN(const std::vector<BackgroundLayer>& layers);
    template<int obj_master, int blend_mode>
    void collapseBN(const std::vector<BackgroundLayer>& layers);
    template<int obj_master>
    void collapseBW(const std::vector<BackgroundLayer>& layers);
    template<int obj_master, int blend_mode>
    void collapseBW(const std::vector<BackgroundLayer>& layers);
    template<int obj_master, int blend_mode, int win_master>
    void collapseBW(const std::vector<BackgroundLayer>& layers);

    template<int obj_master>
    int possibleWindows() const;
    template<int win_master>
    const Window& activeWindow(int x) const;

    template<int obj_master>
    u16 upperLayer(const std::vector<BackgroundLayer>& layers, int x) const;
    template<int obj_master>
    u16 upperLayer(const std::vector<BackgroundLayer>& layers, int x, int flags) const;

    template<int obj_master>
    bool findBlendLayers(const std::vector<BackgroundLayer>& layers, int x, int flags, u16& upper) const;
    template<int obj_master>
    bool findBlendLayers(const std::vector<BackgroundLayer>& layers, int x, int flags, u16& upper, u16& lower) const;

    u16 blendAlpha(u16 a, u16 b) const;
    u16 blendWhite(u16 a) const;
    u16 blendBlack(u16 a) const;

    static u32 argb(u16 color);

    u16 readBgColor(int index, int bank);
    u16 readFgColor(int index, int bank);
    u16 readPixel(u32 addr, int x, int y, Palette palette);
       
    DoubleBuffer<u16> backgrounds[4];
    Buffer<ObjectLayer> objects;
    bool objects_exist;
    bool objects_alpha;

    std::array<s16*, 32> pas;
    std::array<s16*, 32> pbs;
    std::array<s16*, 32> pcs;
    std::array<s16*, 32> pds;
};

#include "collapse.inl"
