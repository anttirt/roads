#include "unit_config.h"

#if RUN_UNIT_TESTS == 1

#include "unit_test.h"
#include "fixed16.h"
#include "vector.h"
#include "display_list.h"
#include "disp_writer.h"

#include <nds.h>
#include <stdio.h>
#include <nds/arm9/videoGL.h>

//---------------------------------------------------------------------------------
int main(void) {
    lcdMainOnTop();
    videoSetMode(MODE_0_3D);
    videoSetModeSub(MODE_0_2D);
    vramSetBankI(VRAM_I_SUB_BG_0x06208000);
    consoleInit(NULL, 0, BgType_Text4bpp, BgSize_T_256x256, 23, 2, false, true);
    using namespace roads;
    unit_test_suite suite;
    create_tests<f16>(suite);
    create_tests<vector3f16>(suite);
    //create_tests<display_list>(suite);
    create_tests<disp_writer>(suite);

    suite.run_tests();

	while(1) {
		swiWaitForVBlank();
	}

	return 0;
}

#else // RUN_UNIT_TESTS

#include <stdexcept>
#include <nds.h>

#include "level.h"
#include "variant_access.hpp"
#include "collide.h"
#include "geometry.h"
#include "disp_writer.h"

extern const unsigned char level_data_test0[15182];
static const char headerText[] = "DSRoads Level file v0.003\n";

roads::grid_t make_level_data() {
    using namespace roads;

    constexpr size_t gravity_offset = countof(headerText) - 1;
    constexpr size_t oxygen_leak_offset = gravity_offset + 2;
    constexpr size_t palette_offset = oxygen_leak_offset + 2;
    constexpr size_t grid_offset = palette_offset + (16 * sizeof(rgb));
    constexpr size_t cell_count = (countof(level_data_test0) - grid_offset) / 2;
    constexpr size_t row_count = cell_count / 7;

    memcpy(cell::palette, level_data_test0 + palette_offset, sizeof(rgb) * 16);
    grid_t grid(row_count);
    memcpy(&grid[0], level_data_test0 + grid_offset, countof(level_data_test0) - grid_offset);

    return std::move(grid);
}

int corr, fell, none;

void check_collisions(roads::vector3f32 const& ship_position, roads::vector3f32 const& velocity, roads::grid_t const& grid) {
    using namespace roads;

    auto collide_result = collide(ship_position, velocity, grid);
    visit<void>(collide_result,
        [](collision::correction cor) {
            iprintf("\x1b[3;2H"
                    "corr: %d",
                    corr++
                    );
        },
        [](collision::fell_off) {
            iprintf("\x1b[4;2H"
                    "fell: %d",
                    fell++
                    );
        },
        [](collision::none) {
            iprintf("\x1b[5;2H"
                    "none: %d",
                    none++
                    );
        });
}

void draw_last_bounds() {
    using namespace roads;

    int count = 0;
    glMaterialf(GL_DIFFUSE, make_rgb(31, 31, 31));
    glMaterialf(GL_AMBIENT, make_rgb(31, 31, 31));
    glColor(make_rgb(31, 31, 31));
    for(aabb const& box : last_bounds) {
        ++count;
        glPushMatrix();
        vector3f32 pos = box.min;
        vector3f16 max = (box.max - box.min).convert<f16>();
        int16_t x = raw(max.x), y0 = raw(geometry::draw::block_size * f16(0.5)), y = raw(max.y) + y0, z = raw(max.z);
        glTranslatef32(raw(pos.x), raw(pos.y), raw(pos.z));
        glBegin(GL_TRIANGLES);

        glVertex3v16(0, y0, 0); glVertex3v16(x, y0, 0); glVertex3v16(x, y0, 0);
        glVertex3v16(0, y0, 0); glVertex3v16(0, y, 0); glVertex3v16(0, y, 0);
        glVertex3v16(0, y0, 0); glVertex3v16(0, y0, z); glVertex3v16(0, y0, z);
                                                                            
        glVertex3v16(x, y, z); glVertex3v16(0, y, z); glVertex3v16(0, y, z);
        glVertex3v16(x, y, z); glVertex3v16(x, y0, z); glVertex3v16(x, y0, z);
        glVertex3v16(x, y, z); glVertex3v16(x, y, 0); glVertex3v16(x, y, 0);

        glVertex3v16(x, y0, 0); glVertex3v16(x, y0, z); glVertex3v16(x, y0, z);
        glVertex3v16(x, y0, 0); glVertex3v16(x, y, 0); glVertex3v16(x, y, 0);
                                                                            
        glVertex3v16(0, y, 0); glVertex3v16(x, y, 0); glVertex3v16(x, y, 0);
        glVertex3v16(0, y, 0); glVertex3v16(0, y, z); glVertex3v16(0, y, z);
                                                                            
        glVertex3v16(0, y0, z); glVertex3v16(0, y, z); glVertex3v16(0, y, z);
        glVertex3v16(0, y0, z); glVertex3v16(x, y0, z); glVertex3v16(x, y0, z);
                                                                             
        glEnd();
        glPopMatrix(1);
    }
    aabb const& smp = last_bounds.back();
    printf("\x1b[14;0H"
            "%.3f,%.3f,%.3f\n"
            "%.3f,%.3f,%.3f\n",
            smp.min.x.to_float(),smp.min.y.to_float(),smp.min.z.to_float(),
            smp.max.x.to_float(),smp.max.y.to_float(),smp.max.z.to_float()
            );
}

int main() {
    lcdMainOnTop();
	//set mode 0, enable BG0 and set it to 3D
	videoSetMode(MODE_0_3D);

    videoSetModeSub(MODE_0_2D);
    vramSetBankI(VRAM_I_SUB_BG_0x06208000);
    consoleInit(NULL, 0, BgType_Text4bpp, BgSize_T_256x256, 23, 2, false, true);
	// initialize gl
	glInit();
	
	// enable antialiasing
	glEnable(GL_ANTIALIAS);
	
	// setup the rear plane
	glClearColor(0,0,0,31); // BG must be opaque for AA to work
	glClearPolyID(63); // BG must have a unique polygon ID for AA to work
	glClearDepth(0x7FFF);

	//this should work the same as the normal gl call
	glViewport(0,0,255,191);
	
	//any floating point gl call is being converted to fixed prior to being implemented
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(70, 256.0 / 192.0, 0.1, 40);
	
	gluLookAt(	0.0, 0.3, 0.02,		//camera position 
				0.0, 0.0, 0.0,		//look at
				0.0, 1.0, 0.0);		//up
	
    using roads::vector3d;
    constexpr vector3d ln = vector3d(1, -1, -0.2).normalized();

	glLight(0, roads::make_rgb(1.,1.,1.),
            floattov10(ln.x), floattov10(ln.y), floattov10(ln.z));
	glPolyFmt(POLY_ALPHA(31) | POLY_CULL_BACK | POLY_FORMAT_LIGHT0);

    roads::level lvl { make_level_data() };

    //roads::display_list list = generate_list();
    using roads::f32;

    f32 const move_unit = 0.005;
    roads::vector3f32 move { };

    roads::display_list ship;
    ship.resize(128);

    {
        using namespace roads;
        using geometry::draw::ship_size;
        move.x = ship_size.x * f32(-0.5);
        move.y = geometry::draw::tile_height;
        move.z = ship_size.z * f32(0.5) - f32(geometry::draw::block_size) * f32(0.5);
        disp_writer writer(ship, move, geometry::draw::scale);

        f16 x = f16(raw(ship_size.x), raw_tag);
        f16 y = f16(raw(ship_size.y), raw_tag);
        f16 z = -f16(raw(ship_size.z), raw_tag);

        writer
            << diffuse_ambient { make_rgb(10, 10, 10), make_rgb(0, 0, 0), false }
            << quad { { 0, 0, 0 }, { x, 0, 0 }, { x, y, 0 }, { 0, y, 0 } }
            << quad { { 0, 0, 0 }, { 0, y, 0 }, { 0, y, z }, { 0, 0, z } }
            << quad { { x, 0, 0 }, { x, 0, z }, { x, y, z }, { x, z, 0 } }
            << quad { { 0, y, 0 }, { x, y, 0 }, { x, y, z }, { 0, y, z } }
            << end;
        ship.resize(writer.write_count());
    }
	
	while(1)
	{
		glPushMatrix();
				
        glTranslatef32(0, 0, raw(-move.z));
		
		scanKeys();
		u16 keys = keysHeld();
        roads::vector3f32 velocity { 0, 0, 0 };
		if(keys & KEY_UP)    { velocity.z -= move_unit; }
		if(keys & KEY_DOWN)  { velocity.z += move_unit; }
		if(keys & KEY_LEFT)  { velocity.x -= move_unit; }
		if(keys & KEY_RIGHT) { velocity.x += move_unit; }
		
        using roads::raw_tag;
        check_collisions(move, velocity, lvl.grid);

        move += velocity;

        // updating the level's draw lists should be delayed a little so that rows
        // don't disappear while they're still on screen
        lvl.update(clamp(f32(0.3) + move.z, f32(INT_MIN, roads::raw_tag), f32(0)));
        lvl.draw();

        ship.data()[10] = raw(move.x);
        ship.data()[11] = raw(move.y);
        ship.data()[12] = raw(move.z);
        ship.draw();

        draw_last_bounds();

        //iprintf("\x1b[1;2H"
        //        "grid size: %d\n"
        //        "drawq size: %d\n"
        //        "drawp size: %d\n"
        //        "dist: %d\n",
        //        lvl.grid.size(),
        //        lvl.draw_queue.size(),
        //        lvl.draw_pool.size(),
        //        (lvl.visible_end - lvl.visible_start));

		glPopMatrix(1);
			
		glFlush(0);
	}

	return 0;
}
#endif
