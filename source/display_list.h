#ifndef DSR_DISPLAYLIST_H
#define DSR_DISPLAYLIST_H

#include <vector>
#include <memory>
#include <algorithm>
#include <stdint.h>
#include "vector.h"
#include "cell.h"
#include "fixed16.h"
#include "glcore.h"
#include "container_wrapper.h"

// This comes from sys/types.h on arm-eabi
#undef quad

namespace roads
{
    template <typename DispList>
    struct DispGetFn
    {
        std::vector<uint32_t>& operator()(DispList& disp) const { return disp.cmdlist; }
        std::vector<uint32_t> const& operator()(DispList const& disp) const { return disp.cmdlist; }
    };

	struct display_list
        : sequence_wrapper_base<display_list, std::vector<uint32_t>, DispGetFn<display_list>, DispGetFn<display_list> >
	{
        template <typename> friend struct DispGetFn;

		display_list()
			: dirty(false) {}
		display_list(display_list&& rhs)
            : cmdlist(std::move(rhs.cmdlist)), dirty(true) { rhs.dirty = false; }
        display_list& operator=(display_list&& rhs) {
            cmdlist = std::move(rhs.cmdlist);
            dirty = rhs.dirty;
            rhs.dirty = false;
            return *this;
        }

        uint32_t* data() {
            return &cmdlist[0];
        }

        void resize(size_t count) {
            cmdlist.resize(count);
            dirty = true;
        }

        size_t size() const {
            return cmdlist.size();
        }

		void draw() const;
		void swap(display_list& rhs)
		{
            using std::swap;
            swap(cmdlist, rhs.cmdlist);
            swap(dirty, rhs.dirty);
		}
		void clear()
		{
			cmdlist.clear();
			dirty = false;
		}

        std::vector<uint32_t> dbg_getlist() const
        {
            std::vector<uint32_t> ret(1, cmdlist.size());
            ret.insert(ret.end(), cmdlist.begin(), cmdlist.end());
            return std::move(ret);
        }

	private:
		display_list(display_list const& rhs) = delete;
		display_list& operator=(display_list const& rhs) = delete;

    public:
		void push_back(uint32_t packed_cmd)
		{
			dirty = true;
			cmdlist.push_back(packed_cmd);
		}
    private:

		std::vector<uint32_t> cmdlist;
		mutable bool dirty;
	};

    /*
    - drawing the level:
        - short on allowed quads/tris
        - hardware supports (roughly) max 1500 quads or 2000 tris per frame

        - needs:
            - reserve a bunch for ship, maybe effects? (haha right)
            - max for level around 1.2k? (some detailed ship huh)

        - methods
            - try to combine
                - generic algorithm? doesn't look like it's gonna happen
                    - most likely not a huge benefit and
                      much easier and faster to implement and run
                      with some naive combinations
                - usual patterns?
                    - tile | high | low: check at least top surfaces:
                      single-width, side to side, and back to front
                    - tunnels: HUGE potential savings;
                      check back to front where flags & (tunnel & ~(high | low))
            - cull invisible
                - obvious: outer, bottom, far-from-camera edges
                - hidden by other blocks: check for two cells that are
                  adjacent along Z with face_required(nearcell, farcell)

        - guesstimates:
            - optimal case? (just a bunch of tunnels)
                - 25 * 7 cells drawn at max 7 quads + 10 tris(strip) + 3 quads ~= 20 quads per cell, 17 * 25 * 7 = 3k quads without combining, YEAOWCH
                - combines into about 150 total
                    - here we have ideal combination circumstances, though
            - average case?
                - probably around 30% of visible grid is used up -> around 60 cells used
                - can combine around 50% of flat cells and tunnels, down to around 20 in good condition and 10 in pathological condition: 20 * 3 + 10 * 17 = 230
                    - not bad at all
            - pathological cases?
                - could still reach 2k easily, impossible to optimize
                    - no choice but to design levels around that
                    - would have to be a pretty fucking retarded level though

        - conclusions
            - probably no huge obstacles
            - need to maintain counts while testing levels to avoid extreme situations
    */
}
namespace std
{
    template <>
    inline
    void swap<roads::display_list>(roads::display_list& lhs, roads::display_list& rhs) throw()
	{
		lhs.swap(rhs);
	}

}

#endif // DSR_DISPLAYLIST_H
