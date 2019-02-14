#include "registers.h"

#include "common/memory_map.h"

void DisplayControl::write(int pos, u8 byte)
{
    switch (pos)
    {
    case 0:
        mode = byte & 0x7;
        gbc_mode = byte >> 3 & 0x1;
        bitmap_addr = (byte >> 4 & 0x1) ? MAP_VRAM : (MAP_VRAM + 0xA000);
        process_hblank = (byte >> 5 & 0x1) ? true : false;
        sprite_dim = byte >> 6 & 0x1;
        force_blank = byte >> 7 & 0x1;
        break;

    case 1:
        bg0_enable = byte & 0x1;
        bg1_enable = byte >> 1 & 0x1;
        bg2_enable = byte >> 2 & 0x1;
        bg3_enable = byte >> 3 & 0x1;
        oam_enable = byte >> 4 & 0x1;
        win0_enable = byte >> 5 & 0x1;
        win1_enable = byte >> 6 & 0x1;
        sprite_win_enable = byte >> 7 & 0x1;
        break;
    }
}

void DisplayStatus::write(int pos, u8 byte)
{
    switch (pos)
    {
    case 0:
        v_status = byte & 0x1;
        h_status = byte >> 1 & 0x1;
        vcount_triggered = (byte >> 2 & 0x1) ? true : false;
        vblank_irq_enable = (byte >> 3 & 0x1) ? true : false;
        hblank_irq_enable = (byte >> 4 & 0x1) ? true : false;
        vcount_irq_enable = (byte >> 5 & 0x1) ? true : false;
        break;

    case 1:
        vcount_trigger = byte;
        break;
    }
}

void BackgroundControl::write(int pos, u8 byte)
{
    switch (pos)
    {
    case 0:
        priority = byte & 0x3;
        data_addr = MAP_VRAM + 0x4000 * (byte >> 2 & 0x3);
        mosaic = (byte >> 6 & 0x1) ? true : false;
        palette_type = byte >> 7 & 0x1;
        break;

    case 1:
        map_addr = MAP_VRAM + 0x800 * (byte & 0x1F);
        screen_over = byte >> 5 & 0x1;
        map_size = byte >> 6 & 0x3;
        break;
    }
}
