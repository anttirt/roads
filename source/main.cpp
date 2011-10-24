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

extern const unsigned char level_data_test0[15182];
static const char headerText[] = "DSRoads Level file v0.003\n";

int tunnel_count;

inline roads::cell unpack(unsigned char first, unsigned char second) {
    using roads::cell;
    cell c {};
    c.tile_color = first & 0x0F;
    c.block_color = (first & 0xF0) >> 4;
    c.altitude = (second & 0x70) >> 4;
    c.flags = cell::none;
    if(second & 0x01) { c.flags |= cell::tile; }
    if(second & 0x02) { c.flags |= cell::tunnel; ++tunnel_count; }
    if(second & 0x04) { c.flags |= cell::low; }
    if(second & 0x08) { c.flags |= cell::high; }
    if(second & 0x80) { c.flags |= cell::end; }
    return c;
}

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
	
	gluLookAt(	0.0, 0.25, 0.3,		//camera position 
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

    f32 const move_unit = -0.03;
    f32 move_z = 0;
	
	while(1)
	{
		glPushMatrix();
				
        glTranslatef32(0, 0, raw(move_z));
		
		scanKeys();
		u16 keys = keysHeld();
		if(!(keys & KEY_UP)) move_z += move_unit;
		if(!(keys & KEY_DOWN)) move_z -= move_unit;
		
        // updating the level's draw lists should be delayed a little so that rows
        // don't disappear while they're still on screen
        lvl.update(clamp(move_z - f32(0.3), f32(0), f32(INT_MAX, roads::raw_tag)));
        lvl.draw();

        iprintf("\x1b[1;2H"
                "grid size: %d\n"
                "drawq size: %d\n"
                "drawp size: %d\n"
                "dist: %d\n",
                lvl.grid.size(),
                lvl.draw_queue.size(),
                lvl.draw_pool.size(),
                (lvl.visible_end - lvl.visible_start));

		glPopMatrix(1);
			
		glFlush(0);
	}

	return 0;
}
#endif
