cdef extern from "fcolor/Insert.h" namespace "fcolor":
    FColorList insert(FColorList fcl,
                      int offset, unsigned int length, bool rollover, int skip)

def insert_color_list(object fcl,
                      int offset, unsigned int length, bool rollover, int skip):
    cdef ColorList source
    cdef ColorList result

    source = toColorList(fcl)
    result = ColorList()
    result.thisptr.copy(insert(source.thisptr[0], offset, length, rollover, skip))
    return result
