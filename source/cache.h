#ifndef DSR_CACHE_H_
#define DSR_CACHE_H_

#include <stdint.h>

extern "C"
{
    /*! \fn IC_InvalidateAll()
        \brief invalidate entire instruction cache.
    */
    void IC_InvalidateAll();

        
    /*! \fn IC_InvalidateRange(const void *base, u32 size)
        \brief invalidate the instruction cache for a range of addresses.
        \param base base address of the region to invalidate
        \param size size of the region to invalidate.
    */
    void IC_InvalidateRange(const void *base, uint32_t size);


    /*! \fn DC_FlushAll()
        \brief flush the entire data cache to memory.
    */
    void DC_FlushAll();


    /*! \fn DC_FlushRange(const void *base, u32 size)
        \brief flush the data cache for a range of addresses to memory.
        \param base base address of the region to flush.
        \param size size of the region to flush.
    */
    void DC_FlushRange(const void *base, uint32_t size);


    /*! \fn DC_InvalidateAll()
        \brief invalidate the entire data cache.
    */
    void DC_InvalidateAll();


    /*! \fn DC_InvalidateRange(const void *base, u32 size)
        \brief invalidate the data cache for a range of addresses.
        \param base base address of the region to invalidate
        \param size size of the region to invalidate.
    */
    void DC_InvalidateRange(const void *base, uint32_t size);

}

#endif // DSR_CACHE_H_
