#include "unit_config.h"

#if RUN_UNIT_TESTS == 1

#include "unit_test.h"
#include "fixed16.h"
#include "vector.h"
#include "display_list.h"

#include <nds.h>
#include <stdio.h>

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
    create_tests<vector3>(suite);
    create_tests<display_list>(suite);

    suite.run_tests();

	while(1) {
		swiWaitForVBlank();
	}

	return 0;
}

#else // RUN_UNIT_TESTS

#include <nds.h>

#include "display_list.h"
#include "utility.h"

roads::display_list generate_list() {
    using namespace roads;

    disp_gen gen;

    //gen.diffuse_ambient(RGB15(31, 31, 31), RGB15(0, 0, 0), false);
    //gen.specular_emission(RGB15(0, 0, 0), RGB15(0, 0, 0), false);
    //gen.quad(RGB15(31, 31, 31), vector3(0, 0, -1), vector3(0, -1, 0), vector3(0, 0, 0), vector3(1, 0, 0), vector3(1, -1, 0));
    //gen.quad(RGB15(31, 31, 31), vector3(0, 0, 0), vector3(0, 1, 0), vector3(1, 1, 0), vector3(1, 0, 0));

    gen.diffuse_ambient(RGB15(24, 24, 24), RGB15(3, 3, 3), true);
    gen.specular_emission(RGB15(0, 0, 0), RGB15(0, 0, 0), false);
    vertex v0 { vector3(-1, -1, 0), vector3(0, 0, -1), texcoord_t(inttot16(0), inttot16(0)) };
    vertex v1 { vector3( 1, -1, 0), vector3(0, 0, -1), texcoord_t(inttot16(0), inttot16(0)) };
    vertex v2 { vector3( 0,  1, 0), vector3(0, 0, -1), texcoord_t(inttot16(0), inttot16(128)) };
    gen.tri(v0, v1, v2);

    return gen.create();
}

//uint32_t disp_lst[] = {
//	FIFO_COMMAND_PACK(FIFO_BEGIN, FIFO_COLOR, FIFO_VERTEX16, FIFO_COLOR),
//	GL_TRIANGLE,
//	RGB15(31,0,0),
//	VERTEX_PACK(inttov16(-1),inttov16(-1)), VERTEX_PACK(0,0),
//	RGB15(0,31,0),
//	FIFO_COMMAND_PACK(FIFO_VERTEX16, FIFO_COLOR, FIFO_VERTEX16, FIFO_END),
//	VERTEX_PACK(inttov16(1),inttov16(-1)), VERTEX_PACK(0,0),
//	RGB15(0,0,31),
//	VERTEX_PACK(inttov16(0),inttov16(1)), VERTEX_PACK(0,0),
//};


int main() {
	float rotateX = 0.0;
	float rotateY = 0.0;

	//set mode 0, enable BG0 and set it to 3D
	videoSetMode(MODE_0_3D);

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
	
	// setup the lighting
	//glLight(0, RGB15(31,31,31) , 0, floattov10(-.5), floattov10(-.85));

	glLight(0, RGB15(31,31,31) , 0,				  floattov10(-1.0),		 0);
	glLight(1, RGB15(31,0,31),   0,				  floattov10(1) - 1,			 0);
	glLight(2, RGB15(0,31,0) ,   floattov10(-1.0), 0,					 0);
	glLight(3, RGB15(0,0,31) ,   floattov10(1.0) - 1,  0,					 0);
	
	//not a real gl function and will likely change
	glPolyFmt(POLY_ALPHA(31) | POLY_CULL_BACK | POLY_FORMAT_LIGHT0 | POLY_FORMAT_LIGHT1 | 
			  POLY_FORMAT_LIGHT2 | POLY_FORMAT_LIGHT3 ) ;
	//glPolyFmt(POLY_ALPHA(31) | POLY_CULL_BACK | POLY_FORMAT_LIGHT0);

    roads::display_list list = generate_list();
    //roads::display_list list;

    //for(size_t i = 0; i < countof(disp_lst); ++i)
    //    list.push_back(disp_lst[i]);
	
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
		//glCallList((u32*)teapot_bin);	

		glPopMatrix(1);
			
		glFlush(0);
	}

	return 0;
}
#endif
