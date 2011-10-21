#ifndef DSR_DMACORE_H_
#define DSR_DMACORE_H_

#include <stdint.h>

namespace roads
{
    enum { dma_address_base = 0x04000000 };

    enum dma_offset_t
    {
        dma0_src  = 0xB0,
        dma0_dest = 0xB4,
        dma0_cr   = 0xB8,

        dma1_src  = 0xBC,
        dma1_dest = 0xC0,
        dma1_cr   = 0xC4,

        dma2_src  = 0xC8,
        dma2_dest = 0xCC,
        dma2_cr   = 0xD0,

        dma3_src  = 0xD4,
        dma3_dest = 0xD8,
        dma3_cr   = 0xDC,
    };

    inline dma_offset_t dma_src(uint8_t n)
    { return dma_offset_t(dma0_src + n * 12); }

    inline dma_offset_t dma_dest(uint8_t n)
    { return dma_offset_t(dma0_dest + n * 12); }

    inline dma_offset_t dma_cr(uint8_t n)
    { return dma_offset_t(dma0_cr + n * 12); }

    enum dma_flag_t
    {
        dma_enable         = 1 << 31,
        dma_busy           = 1 << 31,
        dma_irq_req        = 1 << 30,

        dma_start_now      = 0,
        dma_start_card     = 5 << 27,

        dma_start_hbl      = 1 << 28,
        dma_start_vbl      = 1 << 27,
        dma_start_fifo     = 7 << 27,
        dma_disp_fifo      = 4 << 27,

        dma_16_bit         = 0,
        dma_32_bit         = 1 << 26,

        dma_repeat         = 1 << 25,

        dma_src_inc        = 0,
        dma_src_dec        = 1 << 23,
        dma_src_fix        = 1 << 24,

        dma_dst_inc        = 0,
        dma_dst_dec        = 1 << 21,
        dma_dst_fix        = 1 << 22,
        dma_dst_reset      = 3 << 21,

        dma_copy_words     = dma_enable | dma_32_bit | dma_start_now,
        dma_copy_halfwords = dma_enable | dma_16_bit | dma_start_now,
        dma_fifo           = dma_enable | dma_32_bit | dma_dst_fix | dma_start_fifo
    };

    template <dma_offset_t Offset>
    uint32_t volatile& dma_reg()
    {
        uint32_t const addr_val = dma_address_base + Offset;
        uint32_t volatile* addr = reinterpret_cast<uint32_t volatile*>(addr_val);
        return *addr;
    }
}

#endif // DSR_DMACORE_H_

