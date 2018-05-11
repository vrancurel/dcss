# coding: utf-8

from gdb.printing import RegexpCollectionPrettyPrinter


class UInt160Printer(object):
    """Print an UInt160 using the hex representation."""

    LIMB_COUNT = 5

    def __init__(self, val):
        self.limbs = val['m_limbs']

    def to_string(self):
        chunks = []
        for i in range(self.LIMB_COUNT):
            chunks.append('{:08X}'.format(int(self.limbs['_M_elems'][i])))
        return ''.join(chunks)

def pretty_printers():
    pp = RegexpCollectionPrettyPrinter("quadiron")
    pp.add_printer('UInt160',  '^kad::UInt160$',  UInt160Printer)
    return pp
