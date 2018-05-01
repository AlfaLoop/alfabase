import unittest
import binascii

import nestutil

class TestNestUtilMethods(unittest.TestCase):
    def test_nest_command_pack(self):
        x = nestutil.nest_command_pack([2,5,8,12,255,43], nestutil.OP_BFTP_INIT)
        self.assertEqual(binascii.hexlify(x), '6001d7060205080cff2b00000000000000000000')

    def test_nest_command_unpack(self):
        x = nestutil.nest_command_pack([2,5,8,12,255,43], nestutil.OP_BFTP_INIT)
        cmd = nestutil.nest_command_unpack(x)
        self.assertEqual(cmd.data_len, 6)
        self.assertEqual(cmd.data, bytearray([2,5,8,12,255,43]))
        self.assertEqual(cmd.opcode, nestutil.OP_BFTP_INIT)


if __name__ == '__main__':
    unittest.main()
