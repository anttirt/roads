#if defined(DSR_CELL_H) && defined(DSR_VECTOR_H_INCLUDED) && defined(DSR_FIXED16_H)

#ifndef ROADS_CELLVECTOR_H
#define ROADS_CELLVECTOR_H

namespace roads {
    struct disp_writer;

    struct draw_cell {
        cell c;
        vector3f16 position;
        vector3f16 scale;
    };

    disp_writer& operator<<(disp_writer& writer, draw_cell const& drc);
}

#endif // ROADS_CELLVECTOR_H

#endif

