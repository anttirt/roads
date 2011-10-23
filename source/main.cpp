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

#include <boost/array.hpp>
#include <vector>
#include <stdexcept>
#include <nds.h>

#include "display_list.h"
#include "disp_writer.h"
#include "utility.h"
#include "cell.h"
#include "vector.h"
#include "fixed16.h"
#include "geometry.h"

typedef boost::array<roads::cell, 7> row;
typedef std::vector<row> level_data;

level_data make_level_data() {
    using namespace roads;
    row r0 {
        cell { cell::life, cell::life, 1, cell::tile },
        cell { cell::life, cell::life, 1, cell::tile },
        cell { cell::fast, cell::life, 1, cell::tile },
        cell { cell::life, cell::life, 1, cell::tile },
        cell { cell::fast, cell::life, 1, cell::tile },
        cell { cell::life, cell::life, 1, cell::tile },
        cell { cell::life, cell::life, 1, cell::tile },
    };

    row r1 {
        cell { cell::life, cell::life, 1, cell::tile },
        cell { cell::fast, cell::life, 1, cell::tile },
        cell { cell::life, cell::life, 1, cell::tile },
        cell { cell::life, cell::life, 1, cell::tile },
        cell { cell::life, cell::life, 1, cell::tile },
        cell { cell::fast, cell::life, 1, cell::tile },
        cell { cell::life, cell::life, 1, cell::tile },
    };

    level_data data;
    for(size_t i = 0; i < 5; ++i) {
        for(size_t j = 0; j < 5; ++j) { data.push_back(r0); }
        data.push_back(r1);
        data.push_back(r0);
        for(size_t j = 0; j < 5; ++j) { data.push_back(r1); }
        data.push_back(r0);
        data.push_back(r1);
    }
    return std::move(data);
}

namespace roads {

}

roads::display_list generate_list() {
    using namespace roads;

    display_list lst;
    lst.resize(4096);

    disp_writer writer(lst, { 0, 0, -1 }, { 12, 12, 12 });

    cell c0 { cell::life, cell::death, 0, cell::tile | cell::high };
    cell c1 { cell::life, cell::death, 0, cell::tile | cell::low };
    cell c2 { cell::life, cell::death, 0, cell::tile };
    cell c3 { cell::ice, cell::death, 0, cell::tile };
    cell c4 { cell::fast, cell::fast, 0, cell::high };
    cell c5 { cell::fast, cell::fast, 0, cell::tunnel };
    cell c6 { cell::fast, cell::fast, 0, cell::tunnel | cell::high };
    cell c7 { cell::fast, cell::fast, 0, cell::tunnel | cell::low | cell::tile };

    writer
        << draw_cell { c0, { 0, 0, 0, }, { 1, 1, 5 } }
        << draw_cell { c1, { geometry::draw::block_size, 0, 0, }, { 1, 1, 5 } }
        << draw_cell { c2, { geometry::draw::block_size*2, 0, 0, }, { 1, 1, 5 } }
        << draw_cell { c3, { geometry::draw::block_size*3, 0, 0, }, { 1, 1, 5 } }
        << draw_cell { c4, { geometry::draw::block_size*-1, 0, 0, }, { 1, 1, 5 } }
        << draw_cell { c5, { geometry::draw::block_size*-2, 0, 0, }, { 1, 1, 5 } }
        << draw_cell { c6, { geometry::draw::block_size*-3, 0, 0, }, { 1, 1, 5 } }
        << draw_cell { c7, { geometry::draw::block_size*-4, 0, 0, }, { 1, 1, 5 } }
        << end;

    lst.resize(writer.write_count());

    iprintf("wrote 1 cell\ncount: %d\n", writer.write_count());
    iprintf("writer is %sokay", writer ? "" : "NOT ");

    return std::move(lst);
}

int main() {
	float rotateX = 0.0;
	float rotateY = 0.0;

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
	
	gluLookAt(	0.0, 0.0, 3.5,		//camera possition 
				0.0, 0.0, 0.0,		//look at
				0.0, 1.0, 0.0);		//up
	
    using roads::vector3d;
    constexpr vector3d ln = vector3d(1, -1, -1).normalized();

	glLight(0, roads::make_rgb(0.1,0.1,0.1),
            floattov10(ln.x), floattov10(ln.y), floattov10(ln.z));
	glPolyFmt(POLY_ALPHA(31) | POLY_CULL_BACK | POLY_FORMAT_LIGHT0);

    roads::display_list list = generate_list();
	
	while(1)
	{
		glPushMatrix();
				
		glRotateX(rotateX);
		glRotateY(rotateY);
		
		scanKeys();
		u16 keys = keysHeld();
		if(!(keys & KEY_UP)) rotateX += 3;
		if(!(keys & KEY_DOWN)) rotateX -= 3;
		if(!(keys & KEY_LEFT)) rotateY += 3;
		if(!(keys & KEY_RIGHT)) rotateY -= 3;
		
        list.draw();

		glPopMatrix(1);
			
		glFlush(0);
	}

	return 0;
}
#endif
