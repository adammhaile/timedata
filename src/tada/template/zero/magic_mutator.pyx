### comment

"""These are zero-operator "magic" arithmetic operators that have a
   corresponding mutator of the same name."""

### declare
    {class_cpp} {name}({class_py}&)

### define
    def __{name}__({class_py} self):
        """{documentation}"""
        cdef {class_py} s = {class_py}()
        s.cdata = {name}(self.cdata)
        return s

    cpdef {class_py} {name}({class_py} self):
        """{documentation}"""
        self.cdata = {name}(self.cdata)
        return self
