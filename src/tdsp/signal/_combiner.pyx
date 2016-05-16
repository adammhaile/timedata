# Automatically generated on 2016-05-13T19:42:09.188338
# by https://github.com/rec/make_pyx/make_pyx.py

cdef extern from "<tdsp/signal/combiner.h>" namespace "tdsp":
    struct Combiner:
        float scale, offset
        uint mute
        uint invert

cdef class _Combiner(_Wrapper):
    cdef Combiner _instance;

    def __cinit__(self):
        clearStruct(self._instance)

    def clear(self):
        clearStruct(self._instance)

    def __str__(self):
        return "(scale=%s, offset=%s, mute=%s, invert=%s)" % (
            self.scale, self.offset, self.mute, self.invert)

    property scale:
        def __get__(self):
            return self._instance.scale
        def __set__(self, float x):
            self._instance.scale = x

    property offset:
        def __get__(self):
            return self._instance.offset
        def __set__(self, float x):
            self._instance.offset = x

    property mute:
        def __get__(self):
            return self._instance.mute
        def __set__(self, uint x):
            self._instance.mute = x

    property invert:
        def __get__(self):
            return self._instance.invert
        def __set__(self, uint x):
            self._instance.invert = x