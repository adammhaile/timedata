include "tdsp/FColorList.pyx"

import math
import six

def _make_list(object value):
    try:
        len(value)
    except:
        value = list(value)
    return value

cdef ColorList toColorList(object value):
    if isinstance(value, ColorList):
        return <ColorList> value
    else:
        return ColorList(value)

def to_color_list(object x):
    return toColorList(x)

cdef class ColorList:
    cdef FColorList* thisptr
    cdef int _columns

    def __cinit__(self, colors=None, columns=0):
        self.thisptr = new FColorList()
        self.extend(colors)
        self._columns = getattr(colors, 'columns', 0)
        if columns:
            self.columns = columns

    def __dealloc__(self):
        del self.thisptr

    property columns:
        def __get__(self):
            return self._columns

        def __set__(self, unsigned int cols):
            recolumn(self.thisptr, self._columns, cols)
            self._columns = cols

    property size:
        def __get__(self):
            return len(self)

        def __set__(self, unsigned int new_size):
            self.thisptr.resize(new_size)

    def append(self, object item):
        cdef FColor c
        if fill_color(item, &c):
            self.thisptr.push_back(c)
        else:
            raise ValueError('Don\'t understand color value %s' % item)

    def combine(self, *items):
        cdef ColorList cl
        for other in items:
            if isinstance(other, ColorList):
                if mustRecolumn(self.columns, other.columns):
                    if other.columns < self.columns:
                        cl = ColorList(other, columns=self.columns)
                    else:
                        cl = <ColorList> other
                        self.columns = cl.columns
                else:
                    cl = <ColorList> other
            else:
                cl = ColorList(other, columns=self.columns)

            self.thisptr.combine(cl.thisptr[0])
            if not self.columns:
                self.columns = cl.columns

    def count(self, object item):
        cdef FColor c
        if not fill_color(item, &c):
            raise ValueError('Don\'t understand color value %s' % item)
        return self.thisptr.count(c)

    def extend(self, object colors, bool return_errors=False):
        if not colors:
            return

        if hasattr(colors, 'getdata'):
            colors = colors.getdata()

        if isinstance(colors, ColorList):
            self.thisptr.extend((<ColorList> colors).thisptr[0])
            return

        original_length = len(self)
        colors = list(colors)
        new_length = len(colors)
        error_colors = []

        self.thisptr.reserve(original_length + new_length)
        for color in colors:
            try:
                self.append(color)
            except:
                if return_errors:
                    error_colors.append(color)
                    self.append(None)
                else:
                    self.thisptr.resize(original_length)
                    raise
        return return_errors and error_colors

    def gamma(self, float f):
        self.thisptr.gamma(f)

    def index(self, object color):
        cdef FColor c
        if not fill_color(color, &c):
            raise ValueError('Don\'t understand color value %s' % color)
        index = self.thisptr.index(c)
        if index >= 0:
            return index
        raise ValueError('%s is not in ColorList' % color)

    def insert(self, int index, object item):
        self[index:index] = [item]

    def interpolate(self, color_list, float fader, unsigned int smooth=0):
        cdef ColorList cl = toColorList(color_list)
        cdef ColorList result = ColorList(columns=self.column or cl.columns)
        result.thisptr[0] = self.thisptr.interpolate(
            cl.thisptr[0], fader, smooth)
        return result

    def pop(self, int index=-1):
        index = self._check_key(index)
        item = self[index]
        del self[index]
        return item

    def remove(self, object item):
        del self[self.index(item)]

    def reverse(self):
        self.thisptr.reverse()

    def scale(self, float s):
        self.thisptr.scale(s)

    def set_all(self, Color c):
        self.thisptr.setAll(c.thisptr[0])

    def sort(self):
        self.thisptr.sort()

    def __add__(self, object other):
        cl = ColorList(self, columns=getattr(self, 'columns', 0))
        cl.extend(other)
        return cl

    def __contains__(self, object item):
        try:
            self.index(item)
            return True
        except:
            return False

    def __delitem__(self, key):
        self.thisptr.eraseOne(self._check_key(key))

    def __getitem__(self, object key):
        if isinstance(key, slice):
            indices = range(*key.indices(len(self)))
            cl = ColorList()
            cl.thisptr.resize(len(indices))
            i = 0
            for j in indices:
                cl.thisptr.set(i, self.thisptr.get(j))
                i += 1
            return cl

        else:
            key = self._check_key(key)
            color = Color()
            color.thisptr[0] = self.thisptr.get(key)
            return color

    def __iadd__(self, object other):
        self.extend(other)
        return self

    def __imul__(self, int mult):
        length = len(self)
        self.thisptr.reserve(mult * length)
        for i in range(1, mult):
            self.thisptr.insertRange(i * length, self.thisptr[0], 0, length)
        return self

    def __len__(self):
        return self.thisptr.size()

    def __mul__(self, object mult):
        if not isinstance(self, ColorList):
            self, mult = mult, self
        cl = ColorList(self, columns=self.columns)
        cl *= mult
        return cl

    def __radd__(self, object other):
        return ColorList(other) + self

    def __repr__(self):
        return 'ColorList(%s)' % self.__str__()

    def __richcmp__(ColorList self, ColorList other, int comparer):
        if other is None:
            other = ColorList()
        if self.columns != other.columns:
            return compare_ints(self.columns, other.columns, comparer)

        for i in range(max(len(self), len(other))):
            if not richcmpColors(
                &self.thisptr.get(i), &other.thisptr.get(i), comparer):
                return False
        return True

    def __reversed__(self):
        cl = ColorList(self)
        cl.reverse()
        return cl

    def __rmul__(self, int other):
        return self * other

    def __setitem__(self, object key, object value):
        if isinstance(key, slice):
            cl = toColorList(value)
            length = len(cl)
            indices = key.indices(len(self))
            pieces = range(*indices)
            slice_length = len(pieces)

            if slice_length != length:
                if indices[2] != 1:
                    raise ValueError('attempt to assign sequence of size %s '
                                     'to extended slice of size %d' %
                                     (length, slice_length))

                if slice_length > length:
                    self.thisptr.eraseRange(length, slice_length)
                else:
                    self.thisptr.insertRange(indices[0] + slice_length,
                                             cl.thisptr[0], slice_length, length)

            i = 0
            for j in pieces:
                self.thisptr.set(j, cl.thisptr.get(i))

        else:
            key = self._check_key(key)
            if not fill_color(value, &self.thisptr.at(key)):
                raise ValueError('Don\'t understand color value %s' % value)

    def __sizeof__(self):
        return super(ColorList, self).__sizeof__() + 4 * len(self)

    def __str__(self):
        if not self._columns:
            return '[%s]' % ', '.join(str(c) for c in self)
        result = []
        for c in xrange(0, len(self), self._columns):
            result.append(str(self[c:c + self._columns]))
        joined = ',\n '.join(result)
        return '[%s]' % (('\n ' + joined) if joined else joined)

    def _check_key(self, int key):
        if key >= 0:
            if key < len(self):
                return key
        else:
            if -key <= len(self):
                return len(self) + key
        raise IndexError('ColorList index out of range')

    def _set_item(self, int i, object item):
        cdef FColor c
        if fill_color(item, &c):
            self.thisptr.set(i, c)
        else:
            raise ValueError('Don\'t understand color value %s' % item)

def color_list_with_errors(colors=None):
    if isinstance(colors, six.string_types):
        colors = [colors]
    cl = ColorList()
    return cl, cl.extend(colors, return_errors=True)

def combine_color_lists(list data, int columns=0):
    cdef ColorList result = ColorList()
    result.combine(*data)
    if columns:
        result.columns = columns
    return result
