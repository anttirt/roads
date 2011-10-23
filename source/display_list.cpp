#include <initializer_list>
#include <vector>
#include <boost/variant.hpp>
#include <boost/variant/get.hpp>

#include "arrayvec.hpp"
#include "display_list.h"
#include "cache.h"
#include "glcore.h"
#include "geometry.h"
#include "dmacore.h"

namespace roads
{
    void display_list::draw() const
	{
		if(cmdlist.empty())
			return;

		if(dirty)
		{
			DC_FlushRange(&cmdlist[0], cmdlist.size() * 4);
			dirty = false;
		}
        
		// don't start DMAing while anything else
		// is being DMAed because FIFO DMA is touchy as hell
		//    If anyone can explain this better that would be great. -- gabebear
		while(
				(dma_reg<dma0_cr>() & dma_busy) ||
				(dma_reg<dma1_cr>() & dma_busy) ||
				(dma_reg<dma2_cr>() & dma_busy) ||
				(dma_reg<dma3_cr>() & dma_busy)
			 );
        
		// send the packed list asynchronously via DMA to the FIFO
		dma_reg<dma0_src>() = (uint32_t)&cmdlist[0];
		dma_reg<dma0_dest>() = 0x4000400;
		dma_reg<dma0_cr>() = dma_fifo | cmdlist.size();
		while(dma_reg<dma0_cr>() & dma_busy);
	}
}
