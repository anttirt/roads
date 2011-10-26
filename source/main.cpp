#include "unit_config.h"

#if RUN_UNIT_TESTS == 1

#include "unit_test.h"
#include "fixed16.h"
#include "vector.h"
#include "display_list.h"
#include "disp_writer.h"
#include "collide.h"

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
    create_tests<collide_result_t>(suite);

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

namespace roads {
    constexpr f32 move_unit = 0.0005;
}

extern const unsigned char level_data_test0[15182];
extern const unsigned char level_data_test2[6278];
static const char headerText[] = "DSRoads Level file v0.003\n";

#define LEVEL_NAME level_data_test2

roads::grid_t make_level_data() {
    using namespace roads;

    constexpr size_t gravity_offset = countof(headerText) - 1;
    constexpr size_t oxygen_leak_offset = gravity_offset + 2;
    constexpr size_t palette_offset = oxygen_leak_offset + 2;
    constexpr size_t grid_offset = palette_offset + (16 * sizeof(rgb));
    constexpr size_t cell_count = (countof(LEVEL_NAME) - grid_offset) / 2;
    constexpr size_t row_count = cell_count / 7;

    memcpy(cell::palette, LEVEL_NAME + palette_offset, sizeof(rgb) * 16);
    grid_t grid(row_count);
    memcpy(&grid[0], LEVEL_NAME + grid_offset, countof(LEVEL_NAME) - grid_offset);

    return std::move(grid);
}

int already, corr, fell, none;

enum class collision_result {
    error,
    death,
    fell_off,
    none
};

volatile bool loop_forever = true;

collision_result check_collisions(roads::vector3f32& ship_position, roads::vector3f32& velocity, roads::vector3f32& acceleration, bool& can_jump, roads::grid_t const& grid) {
    using namespace roads;

    auto collide_result = collide(ship_position, velocity, grid);
    return visit<collision_result>(collide_result,
        [](collision::already const& a) -> collision_result {
            vector3f32 vel = a.prev_velocity;
            iprintf("\x1b[0;0H"
                    "box[%d,%d,%d,%d,%d,%d],ship[%d,%d,%d,%d,%d,%d],vel[%d,%d,%d]",
                    raw(a.box.min.x), raw(a.box.min.y), raw(a.box.min.z), 
                    raw(a.box.max.x), raw(a.box.max.y), raw(a.box.max.z), 
                    raw(a.ship.min.x), raw(a.ship.min.y), raw(a.ship.min.z), 
                    raw(a.ship.max.x), raw(a.ship.max.y), raw(a.ship.max.z), 
                    raw(vel.x), raw(vel.y), raw(vel.z));
            return collision_result::error;
        },
        [&](collision::correction cor) -> collision_result {
            iprintf("\x1b[4;2H"
                    "corr: %d",
                    corr++
                    );
            // adjust velocity depending on what happened
            vector3f32 offset0 = velocity * cor.time;
            switch(cor.dim) {
            case dimension::x:
                velocity.x = 0;
                break;

            case dimension::y:
                velocity.y = -velocity.y * f32(0.5);
                can_jump = true;
                if(velocity.y < f32(0.001) && velocity.y > f32(-0.001))
                    velocity.y = 0;
                break;
                
            case dimension::z:
                //if(velocity.z > f32(0.001))
                    return collision_result::death;
                //else {
                //    velocity.z = 0;
                //}
            }
            vector3f32 offset1 = velocity * (f32(1) - cor.time);
            for(int d = 0; d < 2; ++d) {
                if(offset0[d] > f32(0)) {
                    offset0[d] -= f32(1, raw_tag);
                }
                else if(offset0[d] < f32(0)) {
                    offset0[d] += f32(1, raw_tag);
                }
            }
            ship_position += (offset0 + offset1);
            //ship_position += velocity;
            velocity += acceleration;
            velocity.z = clamp(velocity.z, move_unit * -20, move_unit * 20);
            return collision_result::none;
        },
        [&](collision::fell_off) -> collision_result {
            iprintf("\x1b[5;2H"
                    "fell: %d",
                    fell++
                    );
            ship_position += velocity;
            return collision_result::fell_off;
        },
        [&](collision::none) -> collision_result {
            ship_position += velocity;
            velocity += acceleration;
            return collision_result::none;
        });
}

void draw_box(roads::aabb const& box) {
    using namespace roads;
    glPushMatrix();
    vector3f32 pos = box.min;
    vector3f16 max = (box.max - box.min).convert<f16>();
    //int16_t x = raw(max.x), y0 = raw(geometry::draw::block_size * f16(0.5)), y = raw(max.y) + y0, z = raw(max.z);
    int16_t x = raw(max.x), y0 = 0, y = raw(max.y) + y0, z = raw(max.z);
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

void draw_last_bounds() {
    using namespace roads;

    glMaterialf(GL_DIFFUSE, make_rgb(31, 31, 31));
    glMaterialf(GL_AMBIENT, make_rgb(31, 31, 31));
    glColor(make_rgb(31, 31, 31));
    for(aabb const& box : last_bounds) {
        draw_box(box);
    }
    glColor(make_rgb(0, 31, 31));
    draw_box(last_ship_bounds);
    glColor(make_rgb(31, 31, 0));
    draw_box(last_next_ship_bounds);
    aabb const& smp = last_bounds.back();
    printf("\x1b[14;0H"
            "%.3f,%.3f,%.3f\n"
            "%.3f,%.3f,%.3f\n",
            smp.min.x.to_float(),smp.min.y.to_float(),smp.min.z.to_float(),
            smp.max.x.to_float(),smp.max.y.to_float(),smp.max.z.to_float()
            );
}

void game_over() {
    iprintf("\x1b[0;0H"
            "====================\n"
            "====================\n"
            "==== GAME  OVER ====\n"
            "====================\n"
            "====================\n");
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

    roads::level lvl { make_level_data() };
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(70, 256.0 / 192.0, 0.1, 40);
	
	gluLookAt(	0.0, 0.16, 0.19,		//camera position 
				0.0, 0.0, 0.0,		//look at
				0.0, 1.0, 0.0);		//up
	
    using roads::vector3d;
    constexpr vector3d ln = vector3d(1, -1, -0.2).normalized();

	glLight(0, roads::make_rgb(1.,1.,1.),
            floattov10(ln.x), floattov10(ln.y), floattov10(ln.z));
	glPolyFmt(POLY_ALPHA(31) | POLY_CULL_BACK | POLY_FORMAT_LIGHT0);

    while(1) {
        lvl.reset();
    //roads::display_list list = generate_list();
    using roads::f32;

    roads::vector3f32 move { };

    roads::display_list ship;
    ship.resize(128);

    {
        using namespace roads;
        using geometry::draw::ship_size;
        move.x = ship_size.x * f32(-0.5);
        move.y = geometry::draw::tile_height * 5;
        move.z = ship_size.z * f32(0.5) - f32(geometry::draw::block_size) * f32(0.5);
        disp_writer writer(ship, move, geometry::draw::scale);

        f16 x = f16(raw(ship_size.x), raw_tag);
        f16 y = f16(raw(ship_size.y), raw_tag);
        f16 z = f16(raw(ship_size.z), raw_tag);

        writer
            << diffuse_ambient { make_rgb(31, 0, 0), make_rgb(10, 10, 10), true }
            << quad { { 0, 0, z }, { x, 0, z }, { x, y, z }, { 0, y, z } }
            << quad { { 0, 0, z }, { 0, y, z }, { 0, y, 0 }, { 0, 0, 0 } }
            << quad { { 0, y, z }, { x, y, z }, { x, y, 0 }, { 0, y, 0 } }
            << quad { { x, 0, z }, { x, 0, 0 }, { x, y, 0 }, { x, y, z } }
            << end;
        ship.resize(writer.write_count());
    }

    using roads::move_unit;
    roads::vector3f32 acceleration { 0, -move_unit, 0 }; // gravity
    roads::vector3f32 velocity { 0, 0, 0 };

    bool can_jump = true;
    bool update_camera = true;
    bool game_on = true;
    iprintf("\x1b[0;0H"
            "                                \n"
            "                                \n"
            "                                \n"
            "                                \n"
            "                                \n"
            "                                \n"
            "                                \n"
            "                                \n"
            "                                \n"
            "                                \n"
            "                                \n"
            "                                \n"
            "                                \n"
            "                                \n"
            "                                \n"
            "                                \n"
            "                                \n"
            "                                \n"
            "                                \n"
            "                                \n"
            "                                \n");
	while(game_on)
	{
		glPushMatrix();
				
        if(update_camera)
            glTranslatef32(0, 0, raw(-move.z));
		
		scanKeys();
		u16 keys = keysHeld();
		if(keys & KEY_UP)    { acceleration.z = -move_unit; }
        else if(keys & KEY_DOWN)  { acceleration.z = move_unit; }
        else { acceleration.z = f32(0); }
		if(keys & KEY_LEFT)  { velocity.x = -move_unit * 4 + (velocity.z * f32(0.2)) ; }
        else if(keys & KEY_RIGHT) { velocity.x = move_unit * 4 + (velocity.z * f32(-0.2)); }
        else { velocity.x = 0; }
        if(keys & KEY_A) {
            if(can_jump) {
                if(velocity.y < (move_unit * 2) && velocity.y > (move_unit * -2)) {
                    velocity.y = move_unit * 18; // jump
                    can_jump = false;
                }
            }
        }
		
        bool break_loop = false;
        using roads::raw_tag;
        switch(check_collisions(move, velocity, acceleration, can_jump, lvl.grid)) {
        case collision_result::fell_off:
            update_camera = false;
            break;
        case collision_result::death:
            game_over();
            game_on = false;
            break;
        case collision_result::error:
            game_on = false;
            break;
        default:
            break;
        }


        if(move.y < f32(roads::geometry::draw::block_size) * -4) {
            game_over();
            game_on = false;
        }

        // updating the level's draw lists should be delayed a little so that rows
        // don't disappear while they're still on screen
        lvl.update(clamp(f32(0.3) + move.z, f32(INT_MIN, roads::raw_tag), f32(0)));
        lvl.draw();

        ship.data()[10] = raw(move.x);
        ship.data()[11] = raw(move.y);
        ship.data()[12] = raw(move.z);
        //ship.draw();

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

    while(1) {
		scanKeys();
		u16 keys = keysHeld();
        if(keys & KEY_A) {
            break;
        }
        else {
            swiWaitForVBlank();
        }
    }

    }

	return 0;
}
#endif
