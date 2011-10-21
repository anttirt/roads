#include "display_list.h"
#include "cache.h"
#include "glcore.h"
#include "geometry.h"

namespace roads
{
    struct disp_gen::impl_
    {
		void push_cmd(gfx_offset_t cmd);
		void push_cmd(gfx_offset_t cmd, uint32_t param0);
		void push_cmd(gfx_offset_t cmd, uint32_t param0, uint32_t param1);
        void push_cmd(disp_cmd const& cmd);
		void push_vertex(vector3 const& v);
		void push_vertex(vertex const& v);
		void push_vertices(vector3 const* const vertices, unsigned const count);

		std::vector<disp_cmd> commands;
    };

    void disp_gen::impl_::push_cmd(disp_cmd const& cmd)
    {
        commands.push_back(cmd);
    }

    void disp_gen::impl_::push_cmd(gfx_offset_t cmd)
	{
        commands.push_back({{0, 0}, cmd, 0});
	}

	void disp_gen::impl_::push_cmd(gfx_offset_t cmd, uint32_t param0)
	{
        commands.push_back({{param0, 0},cmd,1});
	}

	void disp_gen::impl_::push_cmd(gfx_offset_t cmd, uint32_t param0, uint32_t param1)
	{
        commands.push_back({{param0,param1},cmd,2});
	}

	void disp_gen::impl_::push_vertex(vector3 const& v)
	{
		push_cmd(gfx_vertex16, vertex_pack(v.x, v.y), vertex_pack(v.z, 0));
	}

    void disp_gen::impl_::push_vertex(vertex const& v) {
        push_cmd(gfx_tex_coord, TEXTURE_PACK(v.texcoord.x, v.texcoord.y));
        push_cmd(gfx_normal, geometry::normal_pack(v.normal));
        push_vertex(v.position);
    }

	void disp_gen::impl_::push_vertices(vector3 const* const vertices, unsigned const count)
	{
		for(unsigned i = 0; i < count; ++i)
		{
			push_vertex(vertices[i]);
		}
	}
    disp_gen::disp_gen()
        : impl(new impl_)
    {
    }
    disp_gen::~disp_gen()
    {
    }

    void disp_gen::swap(disp_gen& rhs)
    {
        impl_* my_p = impl.release();
        impl_* rhs_p = rhs.impl.release();
        impl.reset(rhs_p);
        rhs.impl.reset(my_p);
    }

    disp_gen& disp_gen::cmd(disp_cmd const& cmd)
    {
        impl->push_cmd(cmd);
        return *this;
    }

    constexpr uint32_t pack_material(rgb dif, rgb amb, bool flag) {
        return dif | (amb << 16) | ((flag ? 0 : 1) << 15);
    }

    disp_gen& disp_gen::diffuse_ambient(rgb diffuse, rgb ambient, bool set_vertex_color) {
        impl->push_cmd(gfx_diffuse_ambient, pack_material(diffuse, ambient, set_vertex_color));
        return *this;
    }
    disp_gen& disp_gen::specular_emission(rgb specular, rgb emission, bool enable_shininess_table) {
        impl->push_cmd(gfx_specular_emission, pack_material(specular, emission, enable_shininess_table));
        return *this;
    }

    disp_gen& disp_gen::tri(vertex const& v0, vertex const& v1, vertex const& v2) {
        impl->push_cmd(gfx_begin, gl_triangles);
		impl->push_vertex(v0);
		impl->push_vertex(v1);
		impl->push_vertex(v2);
		return *this;
    }

    disp_gen& disp_gen::quad(vertex const& v0, vertex const& v1, vertex const& v2, vertex const& v3) {
        impl->push_cmd(gfx_begin, gl_quads);
		impl->push_vertex(v0);
		impl->push_vertex(v1);
		impl->push_vertex(v2);
		impl->push_vertex(v3);
		return *this;
    }

	disp_gen& disp_gen::quad(rgb color, vector3 const& v0, vector3 const& v1, vector3 const& v2, vector3 const& v3)
	{
		impl->push_cmd(gfx_begin, gl_quads);
        impl->push_cmd(gfx_color, color);
		impl->push_vertex(v0);
		impl->push_vertex(v1);
		impl->push_vertex(v2);
		impl->push_vertex(v3);
		return *this;
	}

	disp_gen& disp_gen::quad(rgb color, vector3 const& normal, vector3 const& v0, vector3 const& v1, vector3 const& v2, vector3 const& v3)
	{
		impl->push_cmd(gfx_begin, gl_quads);
        impl->push_cmd(gfx_color, color);
        impl->push_cmd(gfx_normal, geometry::normal_pack(normal));
		impl->push_vertex(v0);
		impl->push_vertex(v1);
		impl->push_vertex(v2);
		impl->push_vertex(v3);
		return *this;
	}

	disp_gen& disp_gen::tri(rgb color, vector3 const& v0, vector3 const& v1, vector3 const& v2)
	{
		impl->push_cmd(gfx_begin, gl_triangles);
        impl->push_cmd(gfx_color, color);
		impl->push_vertex(v0);
		impl->push_vertex(v1);
		impl->push_vertex(v2);
		return *this;
	}
	disp_gen& disp_gen::tri(rgb color0, vector3 const& v0, rgb color1, vector3 const& v1, vector3 const& v2)
	{
		impl->push_cmd(gfx_begin, gl_triangles);
        impl->push_cmd(gfx_color, color0);
		impl->push_vertex(v0);
        impl->push_cmd(gfx_color, color1);
		impl->push_vertex(v1);
		impl->push_vertex(v2);
		return *this;
	}
	disp_gen& disp_gen::tri(rgb color0, vector3 const& v0, rgb color1, vector3 const& v1, rgb color2, vector3 const& v2)
	{
		impl->push_cmd(gfx_begin, gl_triangles);
        impl->push_cmd(gfx_color, color0);
		impl->push_vertex(v0);
        impl->push_cmd(gfx_color, color1);
		impl->push_vertex(v1);
        impl->push_cmd(gfx_color, color2);
		impl->push_vertex(v2);
		return *this;
	}

	disp_gen& disp_gen::quad_strip(rgb color, vector3 const* const vertices, unsigned const count)
	{
		impl->push_cmd(gfx_begin, gl_quad_strip);
        impl->push_cmd(gfx_color, color);
		impl->push_vertices(vertices, count);
		return *this;
	}

	disp_gen& disp_gen::triangle_strip(rgb color, vector3 const* const vertices, unsigned const count)
	{
		impl->push_cmd(gfx_begin, gl_triangle_strip);
        impl->push_cmd(gfx_color, color);
		impl->push_vertices(vertices, count);
		return *this;
	}

    void disp_gen::append_to(display_list& ret)
    {
        unsigned const lut[4] = { 0, 3, 2, 1 };
		std::size_t size = impl->commands.size();
        std::size_t nops = lut[size % 4];

        for(; nops != 0; --nops)
		{
            //impl->commands.push_back({{0,0},gfx_nop,0});
            impl->push_cmd(gfx_nop, 0);
		}

		for(unsigned i = 0; i < impl->commands.size(); i += 4)
		{
			gfx_offset_t const cmdpack[4] =
			{
				impl->commands[i].cmd,
				impl->commands[i + 1].cmd,
				impl->commands[i + 2].cmd,
				impl->commands[i + 3].cmd
			};

			ret.push_back(fifo_pack(cmdpack[0], cmdpack[1], cmdpack[2], cmdpack[3]));
			for(unsigned j = 0; j < 4; ++j)
			{
                std::size_t const param_count = impl->commands[i + j].param_count;
				for(unsigned k = 0; k < param_count; ++k)
				{
					ret.push_back(impl->commands[i + j].params[k]);
				}
			}
		}
    }

	display_list disp_gen::create()
	{
		display_list ret;
        append_to(ret);
		return ret;
	}

    void disp_gen::clear()
    {
        impl->commands.clear();
    }
}

//#include <nds/jtypes.h>
//#include <nds/dma.h>

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
