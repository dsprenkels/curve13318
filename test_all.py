#!/bin/echo Please execute with sage -python
# *-* encoding: utf-8 *-*

import ctypes
import io
import os
import unittest

from sage.all import *

P = 2**255 - 19
F = FiniteField(P)
E = EllipticCurve(F, [-3, 13318])

from hypothesis import assume, example, given, HealthCheck, note, seed, settings, strategies as st, unlimited

settings.register_profile("ci", settings(deadline=None, max_examples=10000))

if os.environ.get('CI', None) == '1':
    settings.load_profile("ci")

# Load shared libcurve13318 library
ref12 = ctypes.CDLL(os.path.join(os.path.abspath('.'), 'libref12.so'))

# Define C types
fe12_type = ctypes.c_double * 12
fe12_frozen_type = ctypes.c_double * 6
fe10_type = ctypes.c_uint64 * 10
fe51_type = ctypes.c_uint64 * 5
ge_type = fe12_type * 3
fe12x4_type = ctypes.c_double * 48

# Define functions
fe12_frombytes = ref12.crypto_scalarmult_curve13318_ref12_fe12_frombytes
fe12_frombytes.argtypes = [fe12_type, ctypes.c_ubyte * 32]
fe12_squeeze = ref12.crypto_scalarmult_curve13318_ref12_fe12_squeeze
fe12_squeeze.argtypes = [fe12_type]
fe12_mul_schoolbook = ref12.crypto_scalarmult_curve13318_ref12_fe12_mul_schoolbook
fe12_mul_schoolbook.argtypes = [fe12_type, fe12_type, fe12_type]
fe12_mul_karatsuba = ref12.crypto_scalarmult_curve13318_ref12_fe12_mul_karatsuba
fe12_mul_karatsuba.argtypes = [fe12_type, fe12_type, fe12_type]
fe12_square_karatsuba = ref12.crypto_scalarmult_curve13318_ref12_fe12_square_karatsuba
fe12_square_karatsuba.argtypes = [fe12_type, fe12_type]
fe10_tobytes = ref12.crypto_scalarmult_curve13318_ref12_fe10_tobytes
fe10_tobytes.argtypes = [ctypes.c_ubyte * 32, fe10_type]
fe10_mul = ref12.crypto_scalarmult_curve13318_ref12_fe10_mul
fe10_mul.argtypes = [fe10_type, fe10_type, fe10_type]
fe10_carry = ref12.crypto_scalarmult_curve13318_ref12_fe10_carry
fe10_carry.argtypes = [fe10_type]
fe10_square = ref12.crypto_scalarmult_curve13318_ref12_fe10_square
fe10_square.argtypes = [fe10_type, fe10_type]
fe10_invert = ref12.crypto_scalarmult_curve13318_ref12_fe10_invert
fe10_invert.argtypes = [fe10_type, fe10_type]
fe10_reduce = ref12.crypto_scalarmult_curve13318_ref12_fe10_reduce
fe10_reduce.argtypes = [fe51_type, fe10_type]
convert_fe12_to_fe10 = ref12.crypto_scalarmult_curve13318_ref12_convert_fe12_to_fe10
convert_fe12_to_fe10.argtypes = [fe10_type, fe12_type]
convert_fe12_to_fe51 = ref12.crypto_scalarmult_curve13318_ref12_convert_fe12_to_fe51
convert_fe12_to_fe51.argtypes = [fe51_type, fe12_type]
ge_frombytes = ref12.crypto_scalarmult_curve13318_ref12_ge_frombytes
ge_frombytes.argtypes = [ge_type, ctypes.c_ubyte * 64]
ge_tobytes = ref12.crypto_scalarmult_curve13318_ref12_ge_tobytes
ge_tobytes.argtypes = [ctypes.c_ubyte * 64, ge_type]
ge_add = ref12.crypto_scalarmult_curve13318_ref12_ge_add
ge_add.argtypes = [ge_type] * 3
ge_add_c = ref12.crypto_scalarmult_curve13318_ref12_ge_add_c
ge_add_c.argtypes = [ge_type] * 3
ge_double = ref12.crypto_scalarmult_curve13318_ref12_ge_double
ge_double.argtypes = [ge_type] * 2
ge_double_c = ref12.crypto_scalarmult_curve13318_ref12_ge_double_c
ge_double_c.argtypes = [ge_type] * 2
scalarmult = ref12.crypto_scalarmult_curve13318_scalarmult
scalarmult.argtypes = [ctypes.c_ubyte * 64, ctypes.c_ubyte * 32, ctypes.c_ubyte * 64]
select = ref12.crypto_scalarmult_curve13318_ref12_select
select.argtypes = [ge_type, ctypes.c_ubyte, ge_type * 16]
fe12x4_squeeze = ref12.crypto_scalarmult_curve13318_ref12_fe12x4_squeeze
fe12x4_squeeze.argtypes = [fe12x4_type]
fe12x4_mul = ref12.crypto_scalarmult_curve13318_ref12_fe12x4_mul_nosqueeze
fe12x4_mul.argtypes = [fe12x4_type, fe12x4_type, fe12x4_type]


# Custom testing strategies

st_fe12_unsqueezed = st.lists(
    st.integers(-0.99 * 2**53, 0.99 * 2**53), min_size=12, max_size=12)
st_fe12_squeezed_0 = st.tuples(
    st.integers(int(-1.01 * 2**22), int(1.01 * 2**22)),
    st.integers(int(-1.01 * 2**21), int(1.01 * 2**21)),
    st.integers(int(-1.01 * 2**21), int(1.01 * 2**21)),
    st.integers(int(-1.01 * 2**21), int(1.01 * 2**21)),
    st.integers(int(-1.01 * 2**22), int(1.01 * 2**22)),
    st.integers(int(-1.01 * 2**21), int(1.01 * 2**21)),
    st.integers(int(-1.01 * 2**21), int(1.01 * 2**21)),
    st.integers(int(-1.01 * 2**21), int(1.01 * 2**21)),
    st.integers(int(-1.01 * 2**22), int(1.01 * 2**22)),
    st.integers(int(-1.01 * 2**21), int(1.01 * 2**21)),
    st.integers(int(-1.01 * 2**21), int(1.01 * 2**21)),
    st.integers(int(-1.01 * 2**21), int(1.01 * 2**21)))
st_fe12_squeezed_1 = st.tuples(
    st.integers(int(-1.01 * 2**23), int(1.01 * 2**23)),
    st.integers(int(-1.01 * 2**22), int(1.01 * 2**22)),
    st.integers(int(-1.01 * 2**22), int(1.01 * 2**22)),
    st.integers(int(-1.01 * 2**22), int(1.01 * 2**22)),
    st.integers(int(-1.01 * 2**23), int(1.01 * 2**23)),
    st.integers(int(-1.01 * 2**22), int(1.01 * 2**22)),
    st.integers(int(-1.01 * 2**22), int(1.01 * 2**22)),
    st.integers(int(-1.01 * 2**22), int(1.01 * 2**22)),
    st.integers(int(-1.01 * 2**23), int(1.01 * 2**23)),
    st.integers(int(-1.01 * 2**22), int(1.01 * 2**22)),
    st.integers(int(-1.01 * 2**22), int(1.01 * 2**22)),
    st.integers(int(-1.01 * 2**22), int(1.01 * 2**22)))
st_fe10_carried_0 = st.tuples(st.integers(0, 2**26), st.integers(0, 2**25),
                              st.integers(0, 2**26), st.integers(0, 2**25),
                              st.integers(0, 2**26), st.integers(0, 2**25),
                              st.integers(0, 2**26), st.integers(0, 2**25),
                              st.integers(0, 2**26), st.integers(0, 2**25))
st_fe10_carried_1 = st.tuples(st.integers(0, 2**27), st.integers(0, 2**26),
                              st.integers(0, 2**27), st.integers(0, 2**26),
                              st.integers(0, 2**27), st.integers(0, 2**26),
                              st.integers(0, 2**27), st.integers(0, 2**26),
                              st.integers(0, 2**27), st.integers(0, 2**26))
st_fe10_carried_2 = st.tuples(st.integers(0, 2**28), st.integers(0, 2**27),
                              st.integers(0, 2**28), st.integers(0, 2**27),
                              st.integers(0, 2**28), st.integers(0, 2**27),
                              st.integers(0, 2**28), st.integers(0, 2**27),
                              st.integers(0, 2**28), st.integers(0, 2**27))
st_fe10_uncarried = st.lists(st.integers(0, 2**63), min_size=10, max_size=10)


class TestFE12(unittest.TestCase):
    @given(st.lists(st.integers(0, 255), min_size=32, max_size=32))
    def test_frombytes(self, s):
        expected = sum(F(x) * 2**(8*i) for i,x in enumerate(s))
        s_c = (ctypes.c_ubyte * 32)(*s)
        _, z_c = make_fe12()
        fe12_frombytes(z_c, s_c)
        actual = sum(F(int(x)) for x in z_c)
        self.assertEqual(actual, expected)

    @given(st_fe12_unsqueezed)
    def test_squeeze(self, limbs):
        expected, z_c = make_fe12(limbs)
        fe12_squeeze(z_c)

        # Are all limbs reduced?
        exponent = 0
        for i, limb in enumerate(z_c):
            # Check theorem 2.4
            assert int(limb) % 2**exponent == 0, (i, hex(int(limb)), exponent)
            exponent += 22 if i % 4 == 0 else 21
            assert abs(int(limb)) <= 2**(exponent-1), (i, hex(int(limb)), exponent)
        # Decode the value
        actual = sum(F(int(x)) for x in z_c)
        self.assertEqual(actual, expected)

    @given(st_fe12_squeezed_0, st_fe12_squeezed_1, st.booleans())
    def test_mul_schoolbook(self, f_limbs, g_limbs, swap):
        self.do_test_mul(fe12_mul_schoolbook)(self, f_limbs, g_limbs, swap)

    @given(st_fe12_squeezed_0, st_fe12_squeezed_1, st.booleans())
    def test_mul_karatsuba(self, f_limbs, g_limbs, swap):
        self.do_test_mul(fe12_mul_karatsuba)(self, f_limbs, g_limbs, swap)

    @staticmethod
    def do_test_mul(fn):
        def do_test_mul_inner(self, f_limbs, g_limbs, swap):
            if swap:
                f_limbs, g_limbs = g_limbs, f_limbs
            f, f_c = make_fe12(f_limbs)
            g, g_c = make_fe12(g_limbs)
            _, h_c = make_fe12()
            expected = f * g
            fn(h_c, f_c, g_c)
            note("Multiplying {} and {}".format(f, g))
            note("Expected {}".format(expected))
            actual = F(fe12_val(h_c))
            self.assertEqual(actual, expected)
        return do_test_mul_inner

    @given(st_fe12_squeezed_0)
    def test_square_karatsuba(self, f_limbs):
        f, f_c = make_fe12(f_limbs)
        _, h_c = make_fe12()
        expected = f**2
        fe12_square_karatsuba(h_c, f_c)
        actual = F(fe12_val(h_c))
        self.assertEqual(actual, expected)

class TestFE12x4(unittest.TestCase):
    @given(st_fe12_unsqueezed, st.integers(0, 3))
    def test_squeeze(self, limbs, lane):
        expected, vz_c = make_fe12x4(limbs, lane)
        ret = fe12x4_squeeze(vz_c)

        # Are all limbs reduced?
        exponent = 0
        for i, limb in enumerate(vz_c[lane::4]):
            # Check theorem 2.4
            assert int(limb) % 2**exponent == 0, (i, hex(int(limb)), exponent)
            exponent += 22 if i % 4 == 0 else 21
            assert abs(int(limb)) <= 2**(exponent), (i, hex(int(limb)), exponent)
        # Decode the value
        actual = sum(F(int(x)) for x in vz_c[lane::4])
        self.assertEqual(actual, expected)

    @given(st_fe12_squeezed_0, st_fe12_squeezed_1, st.integers(0,3), st.booleans())
    def test_mul(self, f_limbs, g_limbs, lane, swap):
        fhex = lambda x: [x.hex() for x in list(x)]

        if swap:
            f_limbs, g_limbs = g_limbs, f_limbs

        f, f_c = make_fe12x4(f_limbs, lane)
        g, g_c = make_fe12x4(g_limbs, lane)
        _, h_c = make_fe12x4([], lane)
        expected = f * g
        fe12x4_mul(h_c, f_c, g_c)

        note("f_c: {}".format(fhex(list(f_c)[lane::4])))
        note("g_c: {}".format(fhex(list(g_c)[lane::4])))
        note("h_c: {}".format(fhex(list(h_c)[lane::4])))

        actual = F(fe12x4_val(h_c, lane))
        self.assertEqual(actual, expected)


class TestFE10(unittest.TestCase):
    @given(st_fe10_carried_0)
    def test_tobytes(self, limbs):
        expected, z_c = make_fe10(limbs)
        c_bytes = (ctypes.c_ubyte * 32)(0)
        fe10_tobytes(c_bytes, z_c)
        actual = sum(x * 2**(8*i) for i,x in enumerate(c_bytes))
        self.assertEqual(actual, expected)

    @given(st_fe10_carried_2, st_fe10_carried_2)
    def test_mul(self, f_limbs, g_limbs):
        f, f_c = make_fe10(f_limbs)
        g, g_c = make_fe10(g_limbs)
        _, h_c = make_fe10()
        expected = f * g
        fe10_mul(h_c, f_c, g_c)
        actual = 0
        exponent = 0
        actual = fe10_val(h_c)
        self.assertEqual(actual, expected)

    @given(st_fe10_carried_2)
    def test_square(self, limbs):
        f, f_c = make_fe10(limbs)
        expected = F(f**2)
        _, h_c = make_fe10()
        fe10_square(h_c, f_c)
        actual = F(fe10_val(h_c))
        self.assertEqual(actual, expected)

    @given(st_fe10_uncarried)
    def test_carry(self, limbs):
        expected, z_c = make_fe10(limbs)
        fe10_carry(z_c)
        actual = fe10_val(z_c)
        assert(actual < 2**256)
        self.assertEqual(F(actual), expected)
        for limb in z_c:
            assert(0 <= limb <= 2**26)

    @given(st_fe10_carried_2)
    def test_invert(self, limbs):
        f, f_c = make_fe10(limbs)
        expected = F(f)**-1 if f != 0 else 0
        _, h_c = make_fe10()
        fe10_invert(h_c, f_c)
        actual = F(fe10_val(h_c))
        self.assertEqual(actual, expected)

    @given(st_fe10_carried_0)
    # Value that that is in [p, 2^255⟩
    @example([2**26 -19, 2**25, 2**26, 2**25, 2**26,
              2**25,     2**26, 2**25, 2**26, 2**25 ])
    def test_reduce(self, limbs):
        z, z_c = make_fe10(limbs)
        z_frozen = fe51_type(0)
        expected = F(z)
        fe10_reduce(z_frozen, z_c)
        actual = sum(x * 2**(51*i) for i,x in enumerate(z_frozen))
        self.assertEqual(actual, expected)
        assert(0 <= actual < 2**255 - 19)


class TestConvert(unittest.TestCase):
    @given(st_fe12_squeezed_0)
    def test_convert_fe12_to_fe10(self, limbs):
        expected, z12_c = make_fe12(limbs)
        _, z10_c = make_fe10()
        convert_fe12_to_fe10(z10_c, z12_c)
        actual = F(fe10_val(z10_c))
        self.assertEqual(actual, expected)
        for limb in z10_c:
            assert(0 <= limb <= 2**26)

    @given(st_fe12_squeezed_0)
    def test_convert_fe12_to_fe51(self, limbs):
        expected, z12_c = make_fe12(limbs)
        _, z51_c = make_fe51()
        convert_fe12_to_fe51(z51_c, z12_c)
        actual = F(fe51_val(z51_c))
        self.assertEqual(actual, expected)
        for limb in z51_c:
            assert(0 <= limb <= 2**52)

class TestGE(unittest.TestCase):
    @staticmethod
    def encode_point(x, y, z):
        """Encode a point in its C representation"""
        shift = 0
        x_c = fe12_type(0)
        y_c = fe12_type(0)
        z_c = fe12_type(0)
        for i in range(12):
            mask_width = 22 if i % 4 == 0 else 21
            x_c[i] = float((2**mask_width - 1) & x.lift() >> shift) * 2**shift
            y_c[i] = float((2**mask_width - 1) & y.lift() >> shift) * 2**shift
            z_c[i] = float((2**mask_width - 1) & z.lift() >> shift) * 2**shift
            shift += mask_width
        fe12_squeeze(x_c)
        fe12_squeeze(y_c)
        fe12_squeeze(z_c)
        return ge_type(x_c, y_c, z_c)

    @staticmethod
    def decode_point(point):
        x = fe12_val(point[0])
        y = fe12_val(point[1])
        z = fe12_val(point[2])
        return (x, y, z)

    @staticmethod
    def decode_bytes(c_bytes):
        x, y = F(0), F(0)
        for i, b in enumerate(c_bytes[0:32]):
            x += b * 2**(8*i)
        for i, b in enumerate(c_bytes[32:64]):
            y += b * 2**(8*i)
        return x, y

    @staticmethod
    def point_to_bytes(x, y):
        """Encode the numbers as byte input"""
        # Encode the numbers as byte input
        c_bytes = (ctypes.c_ubyte * 64)(0)
        for i in range(32):
            c_bytes[i] = (x >> (8*i)) & 0xFF
        for i in range(32):
            c_bytes[32+i] = (y >> (8*i)) & 0xFF
        return c_bytes

    @given(st.integers(0, 2**256 - 1), st.integers(0, 2**256 - 1),
           st.sampled_from([1, -1]))
    @example(0, 0, 1) # point at infinity
    @example(0, P, 1)
    @example(0, 2*P, 1)
    def test_frombytes(self, x, y_suggest, sign):
        x = F(x)
        try:
            x, y = (sign * E(x, y_suggest)).xy()
            y_in = y
            expected = 0
        except TypeError:
            # `sqrt` failed
            if F(x) == 0 and F(y_suggest) == 0:
                # Point at infinity
                y_in, y = F(0), F(1)
                z = F(0)
                expected = 0
            else:
                # Invalid input
                y = F(y_suggest)
                y_in = y
                z = F(1)
                expected = -1

        c_bytes = self.point_to_bytes(x.lift(), y_in.lift())
        c_point = ge_type(fe12_type(0))
        ret = ge_frombytes(c_point, c_bytes)
        actual_x, actual_y, actual_z = self.decode_point(c_point)
        self.assertEqual(ret, expected)
        if ret != 0: return
        self.assertEqual(actual_x, x)
        self.assertEqual(actual_y, y)
        self.assertEqual(actual_z, z)

    @given(st.integers(0, 2**256 - 1), st.integers(0, 2**256 - 1),
           st.sampled_from([1, -1]))
    @example(0, 0, 1) # a point at infinity
    @example(0, P, 1)
    @example(0, 2*P, 1)
    def test_tobytes(self, x, z, sign):
        (x, y, z), point = make_ge(x, z, sign)
        c_point = self.encode_point(x, y, z)
        c_bytes = (ctypes.c_ubyte * 64)(0)
        ge_tobytes(c_bytes, c_point)

        if z != 0:
            expected_x, expected_y = point.xy()
        else:
            expected_x, expected_y = F(0), F(0)

        actual_x, actual_y = self.decode_bytes(c_bytes)
        self.assertEqual(actual_x, expected_x)
        self.assertEqual(actual_y, expected_y)

    @given(st.integers(0, 2**256 - 1), st.integers(0, 2**256 - 1),
           st.sampled_from([1, -1]),   st.integers(0, 2**256 - 1),
           st.integers(0, 2**256 - 1), st.sampled_from([1, -1]))
    @example(0, 0, 1, 0, 0, 1)
    @example(0, 1, 1, 0, 0, 1)
    @example(0, 1, -1, 0, 0, 1)
    @settings(suppress_health_check=[HealthCheck.filter_too_much])
    def test_add(self, x1, z1, sign1, x2, z2, sign2):
        self.do_test_add(ge_add)(x1, z1, sign1, x2, z2, sign2)

    @given(st.integers(0, 2**256 - 1), st.integers(0, 2**256 - 1),
           st.sampled_from([1, -1]),   st.integers(0, 2**256 - 1),
           st.integers(0, 2**256 - 1), st.sampled_from([1, -1]))
    @example(0, 0, 1, 0, 0, 1)
    @example(0, 1, 1, 0, 0, 1)
    @example(0, 1, -1, 0, 0, 1)
    @settings(suppress_health_check=[HealthCheck.filter_too_much])
    def test_add_c(self, x1, z1, sign1, x2, z2, sign2):
        self.do_test_add(ge_add_c)(x1, z1, sign1, x2, z2, sign2)

    def do_test_add(self, fn):
        def do_test_add_inner(x1, z1, sign1, x2, z2, sign2):
            (x1, y1, z1), point1 = make_ge(x1, z1, sign1)
            (x2, y2, z2), point2 = make_ge(x2, z2, sign2)
            c_point1 = self.encode_point(x1, y1, z1)
            c_point2 = self.encode_point(x2, y2, z2)
            c_point3 = ge_type(fe12_type(0))
            fn(c_point3, c_point1, c_point2)
            x3, y3, z3 = self.decode_point(c_point3)
            expected = point1 + point2
            note("Expected: {}".format(expected))
            note("Actual: ({} : {} : {})".format(x3, y3, z3))
            if expected == E(0):
                self.assertEqual(F(z3), 0)
                return
            actual = E([F(x3), F(y3), F(z3)])
            self.assertEqual(actual, expected)
        return do_test_add_inner

    @example(0, 0, 1)
    @example(0, 1, 1)
    @example(0, 1, -1)
    @example(5, 26250914708855074711006248540861075732027942443063102939584266239L, 1)
    @given(st.integers(0, 2**256 - 1), st.integers(0, 2**256 - 1),
    st.sampled_from([1, -1]))
    @settings(suppress_health_check=[HealthCheck.filter_too_much])
    def test_double(self, x, z, sign):
        self.do_test_double(ge_double)(x, z, sign)

    @example(0, 0, 1)
    @example(0, 1, 1)
    @example(0, 1, -1)
    @example(5, 26250914708855074711006248540861075732027942443063102939584266239L, 1)
    @given(st.integers(0, 2**256 - 1), st.integers(0, 2**256 - 1),
           st.sampled_from([1, -1]))
    @settings(suppress_health_check=[HealthCheck.filter_too_much])
    def test_double_c(self, x, z, sign):
        self.do_test_double(ge_double_c)(x, z, sign)

    def do_test_double(self, fn):
        def do_test_double_inner(x, z, sign):
            (x, y, z), point = make_ge(x, z, sign)
            c_point = self.encode_point(x, y, z)
            c_point3 = ge_type(fe12_type(0))
            fn(c_point3, c_point)
            x3, y3, z3 = self.decode_point(c_point3)
            expected = 2*point
            note("Expected: {}".format(expected))
            note("Actual: ({} : {} : {})".format(x3, y3, z3))
            if expected == E(0):
                self.assertEqual(F(z3), 0)
                return
            actual = E([F(x3), F(y3), F(z3)])
            self.assertEqual(actual, expected)
        return do_test_double_inner

    @given(st.integers(0, 2**256 - 1), st.integers(0, 2**256 - 1),
           st.sampled_from([1, -1]),   st.integers(0, 2**256 - 1),
           st.integers(0, 2**256 - 1), st.sampled_from([1, -1]))
    @example(0, 0, 1, 0, 0, 1)
    @example(0, 1, 1, 0, 0, 1)
    @example(0, 1, -1, 0, 0, 1)
    @example(5, 26250914708855074711006248540861075732027942443063102939584266239L, 1)
    @settings(suppress_health_check=[HealthCheck.filter_too_much])
    def test_add_ref(self, x1, z1, sign1, x2, z2, sign2):
        (x1, y1, z1), point1 = make_ge(x1, z1, sign1)
        (x2, y2, z2), point2 = make_ge(x2, z2, sign2)
        note("testing: {} + {}".format(point1, point2))
        note("locals(): {}".format(locals()))
        x1, y1, z1 = F(x1), F(y1), F(z1)
        x2, y2, z2 = F(x2), F(y2), F(z2)
        b = 13318
        t0 = x1 * x2;       t1 = y1 * y2;       t2 = z1 * z2
        t3 = x1 + y1;       t4 = x2 + y2;       t3 = t3 * t4
        t4 = t0 + t1;       t3 = t3 - t4;       t4 = y1 + z1
        x3 = y2 + z2;       t4 = t4 * x3;       x3 = t1 + t2
        t4 = t4 - x3;       x3 = x1 + z1;       y3 = x2 + z2
        x3 = x3 * y3;       y3 = t0 + t2;       y3 = x3 - y3
        z3 =  b * t2;       x3 = y3 - z3;       z3 = x3 + x3
        x3 = x3 + z3;       z3 = t1 - x3;       x3 = t1 + x3
        y3 =  b * y3;       t1 = t2 + t2;       t2 = t1 + t2
        y3 = y3 - t2;       y3 = y3 - t0;       t1 = y3 + y3
        y3 = t1 + y3;       t1 = t0 + t0;       t0 = t1 + t0
        t0 = t0 - t2;       t1 = t4 * y3;       t2 = t0 * y3
        y3 = x3 * z3;       y3 = y3 + t2;       x3 = x3 * t3
        x3 = x3 - t1;       z3 = z3 * t4;       t1 = t3 * t0
        z3 = z3 + t1
        self.assertEqual(E([x3, y3, z3]), point1 + point2)

    @example(0, 0, 1)
    @example(0, 1, 1)
    @example(0, 1, -1)
    @example(5, 26250914708855074711006248540861075732027942443063102939584266239L, 1)
    @given(st.integers(0, 2**256 - 1), st.integers(0, 2**256 - 1),
           st.sampled_from([1, -1]))
    @settings(suppress_health_check=[HealthCheck.filter_too_much])
    def test_double_ref(self, x, z, sign):
        (x, y, z), point = make_ge(x, z, sign)
        note("testing: 2*{}".format(point))
        note("locals(): {}".format(locals()))
        x, y, z = F(x), F(y), F(z)
        b = 13318
        t0 =  x *  x;       t1 =  y *  y;       t2 =  z *  z
        t3 =  x *  y;       t3 = t3 + t3;       z3 =  x *  z
        z3 = z3 + z3;       y3 =  b * t2;       y3 = y3 - z3
        x3 = y3 + y3;       y3 = x3 + y3;       x3 = t1 - y3
        y3 = t1 + y3;       y3 = x3 * y3;       x3 = x3 * t3
        t3 = t2 + t2;       t2 = t2 + t3;       z3 =  b * z3
        z3 = z3 - t2;       z3 = z3 - t0;       t3 = z3 + z3
        z3 = z3 + t3;       t3 = t0 + t0;       t0 = t3 + t0
        t0 = t0 - t2;       t0 = t0 * z3;       y3 = y3 + t0
        t0 =  y *  z;       t0 = t0 + t0;       z3 = t0 * z3
        x3 = x3 - z3;       z3 = t0 * t1;       z3 = z3 + z3
        z3 = z3 + z3;
        self.assertEqual(E([x3, y3, z3]), 2*point)

    @unittest.skip('only for debugging')
    @given(st.integers(0, 2**256 - 1), st.integers(0, 2**256 - 1),
           st.sampled_from([1, -1]),   st.integers(0, 2**256 - 1),
           st.integers(0, 2**256 - 1), st.sampled_from([1, -1]))
    @example(0, 0, 1, 0, 0, 1)
    @example(0, 1, 1, 0, 0, 1)
    @example(0, 1, -1, 0, 0, 1)
    @example(5, 26250914708855074711006248540861075732027942443063102939584266239L, 1)
    @settings(suppress_health_check=[HealthCheck.filter_too_much])
    def test_add_asmref(self, x1, z1, sign1, x2, z2, sign2):
        (x1, y1, z1), point1 = make_ge(x1, z1, sign1)
        (x2, y2, z2), point2 = make_ge(x2, z2, sign2)
        note("testing: {} + {}".format(point1, point2))

        c_point1 = self.encode_point(x1, y1, z1)
        c_point2 = self.encode_point(x2, y2, z2)
        c_point3 = ge_type(fe12_type(0))
        ge_add(c_point3, c_point1, c_point2)
        x3_asm, y3_asm, z3_asm = self.decode_point(c_point3)
        x3_asm, y3_asm, z3_asm = F(x3_asm), F(y3_asm), F(z3_asm)

        note("c_point1: {}".format(list(list(y.hex() for y in x) for x in c_point1)))
        note("c_point2: {}".format(list(list(y.hex() for y in x) for x in c_point2)))
        note("c_point3: {}".format(list(list(y.hex() for y in x) for x in c_point3)))

        b = 13318

        x3_ref, y3_ref, z3_ref = 3 * ['nil']

        t0 = x1 * x2;       t1 = y1 * y2;       t2 = z1 * z2
        t3 = x1 + y1;       t4 = x2 + y2;       t3 = t3 * t4
        t4 = t0 + t1;       t3 = t3 - t4;       t4 = y1 + z1

        x3 = y2 + z2;       t4 = t4 * x3;       x3 = t1 + t2
        t4 = t4 - x3;       x3 = x1 + z1;       y3 = x2 + z2
        x3 = x3 * y3;       y3 = t0 + t2;       y3 = x3 - y3

        z3 =  b * t2;       x3 = y3 - z3;       z3 = x3 + x3
        x3 = x3 + z3;       z3 = t1 - x3;       x3 = t1 + x3
        y3 =  b * y3;       t1 = t2 + t2;       t2 = t1 + t2

        y3 = y3 - t2;       y3 = y3 - t0;       t1 = y3 + y3
        y3 = t1 + y3;       t1 = t0 + t0;       t0 = t1 + t0
        t0 = t0 - t2;       t1 = t4 * y3;       t2 = t0 * y3

        y3 = x3 * z3;       y3 = y3 + t2;       x3 = x3 * t3
        x3 = x3 - t1;       z3 = z3 * t4;       t1 = t3 * t0
        z3 = z3 + t1

        x3_ref = x3
        y3_ref = y3
        z3_ref = z3

        note("[x3_asm, y3_asm, z3_asm]: {}".format([x3_asm, y3_asm, z3_asm]))
        note("[x3_ref, y3_ref, z3_ref]: {}".format([x3_ref, y3_ref, z3_ref]))
        self.assertEqual(x3_asm, x3_ref)
        self.assertEqual(y3_asm, y3_ref)
        self.assertEqual(z3_asm, z3_ref)

    @unittest.skip('only for debugging')
    @given(st.integers(0, 2**256 - 1), st.integers(0, 2**256 - 1),
           st.sampled_from([1, -1]))
    @example(0, 0, 1)
    @example(0, 1, 1)
    @example(0, 1, -1)
    def test_double_asmref(self, x, z, sign):
        (x, y, z), point = make_ge(x, z, sign)
        note("testing: 2*{}".format(point))

        c_point = self.encode_point(x, y, z)
        c_point3 = ge_type(fe12_type(0))
        ge_double(c_point3, c_point)
        x3_asm, y3_asm, z3_asm = self.decode_point(c_point3)
        x3_asm, y3_asm, z3_asm = F(x3_asm), F(y3_asm), F(z3_asm)

        note("c_point: {}".format(list(list(y.hex() for y in x) for x in c_point)))
        note("c_point3: {}".format(list(list(y.hex() for y in x) for x in c_point3)))

        b = 13318

        x3_ref, y3_ref, z3_ref = 3 * ['nil']

        t0 =  x *  x;       t1 =  y *  y;       t2 =  z *  z
        t3 =  x *  y;       t3 = t3 + t3;       z3 =  x *  z
        z3 = z3 + z3;       y3 =  b * t2;       y3 = y3 - z3
        x3 = y3 + y3;       y3 = x3 + y3;       x3 = t1 - y3

        y3 = t1 + y3;       y3 = x3 * y3;       x3 = x3 * t3
        t3 = t2 + t2;       t2 = t2 + t3;       z3 =  b * z3
        z3 = z3 - t2;       z3 = z3 - t0;       t3 = z3 + z3
        z3 = z3 + t3;       t3 = t0 + t0;       t0 = t3 + t0

        t0 = t0 - t2;       t0 = t0 * z3;       y3 = y3 + t0
        t0 =  y *  z;       t0 = t0 + t0;       z3 = t0 * z3
        x3 = x3 - z3;       z3 = t0 * t1;       z3 = z3 + z3
        z3 = z3 + z3;

        x3_ref = x3
        y3_ref = y3
        z3_ref = z3

        note("[x3_asm, y3_asm, z3_asm]: {}".format([x3_asm, y3_asm, z3_asm]))
        note("[x3_ref, y3_ref, z3_ref]: {}".format([x3_ref, y3_ref, z3_ref]))
        self.assertEqual(x3_asm, x3_ref)
        self.assertEqual(y3_asm, y3_ref)
        self.assertEqual(z3_asm, z3_ref)


class TestScalarmult(unittest.TestCase):
    @staticmethod
    def encode_k(k):
        k_bytes = (ctypes.c_ubyte * 32)(0)
        for i in range(32):
            k_bytes[i] = (k >> (8*i)) & 0xFF
        return k_bytes

    @given(st.integers(0, 2**255 - 1), st.integers(0, 2**256 - 1),
           st.integers(0, 2**256 - 1), st.sampled_from([1, -1]))
    @example(0, 1, 0, 1)
    def test_scalarmult(self, k, x, z, sign):
        _, point = make_ge(x, z, sign)
        note('Initial point: ' + str(point))
        if point.is_zero():
            (x, y) = F(0), F(0)
        else:
            (x, y) = point.xy()
        c_bytes_in = TestGE.point_to_bytes(x.lift(), y.lift())
        k_bytes = self.encode_k(k)
        c_bytes_out = (ctypes.c_ubyte * 64)(0)

        ret = scalarmult(c_bytes_out, k_bytes, c_bytes_in)
        actual = [int(x) for x in c_bytes_out]

        expected_point = k * point
        if expected_point.is_zero():
            expected_x, expected_y = F(0), F(0)
        else:
            expected_x, expected_y = expected_point.xy()
        expected = TestGE.point_to_bytes(expected_x.lift(), expected_y.lift())
        expected = [int(x) for x in expected]
        note('actual:   ' + str(actual))
        note('expected: ' + str(expected))
        self.assertEqual(ret, 0)
        self.assertEqual(actual, expected)

    @given(st.integers(0, 2**255 - 1), st.integers(0, 2**256 - 1),
           st.integers(0, 2**256 - 1))
    @example(0, 0, 0)
    def test_scalarmult_invalid_point(self, k, x, y):
        if (x, y) in E or (x, y) == (0, 0):
            expected = 0
        else:
            expected = -1
        c_bytes_in = TestGE.point_to_bytes(x, y)
        k_bytes = self.encode_k(k)
        c_bytes_out = (ctypes.c_ubyte * 64)(0)
        ret = scalarmult(c_bytes_out, k_bytes, c_bytes_in)
        self.assertEqual(ret, expected)

    @given(st.integers(-1, 15), st.one_of(st.none(), st.data()))
    def test_select(self, idx, random_numbers):
        dest_c = allocate_aligned(ge_type, 32)
        ptable_c = allocate_aligned(ge_type * 16, 32)
        for i,_ in enumerate(ptable_c):
            for j,_ in enumerate(ptable_c[i]):
                for k,_ in enumerate(ptable_c[i][j]):
                    if random_numbers: 
                        ptable_c[i][j][k] = random_numbers.draw(st.integers(0, 2**53-1))

        if idx == -1:
            # Load neutral element
            expected = ge_type()
            expected[1][0] = 1.0
            idx = 31
        else:
            expected = ptable_c[idx]
        expected = list(list(x) for x in expected)

        select(dest_c, idx, ptable_c)
        actual = list(list(x) for x in dest_c)

        note('idx = %s' % idx)
        note('expected: %s' % expected)
        note('actual: %s' % actual)
        self.assertEqual(actual, expected)


def allocate_aligned(ty, align):
    """
    Python does not do any aligned allocations by default. At least, not
    most of the time. The does not seem to be a good API to force an
    allocation to be aligned, so this function implements one.

    We allocate the type and wait until its address value is divisible by
    the desired allocation which will happen if we try long enough.
    """
    stashed = []
    cval = ty()
    for _ in range(1000):
        if ctypes.addressof(cval) % align == 0:
            return cval

        # Try until we have a properly aligned array
        stashed.append(cval) # save the old one or else Python is going to be smart on us
        cval = ty()
    else:
        raise RuntimeError('failed to allocate an aligned piece of ram')

def make_fe12(limbs=[]):
    """Encode the number in its C representation"""
    z = F(0)
    z_c = (ctypes.c_double * 12)(0.0)
    exponent = 0
    for i, limb in enumerate(limbs):
        limb_val = 2**exponent * limb
        z += F(limb_val)
        z_c[i] = float(limb_val)
        exponent += 22 if i % 4 == 0 else 21
    return z, z_c

def make_fe12x4(limbs, lane):
    assert 0 <= lane < 4
    z, z_c = make_fe12(limbs)
    stashed = []
    vz_c = (ctypes.c_double * 48)(0.0)
    while ctypes.addressof(vz_c) % 32 != 0:
        # Try until we have a properly aligned array
        stashed.append(vz_c) # save the old one or else Python is going to be smart on us
        vz_c = (ctypes.c_double * 48)(0.0)
    for i, limb in enumerate(z_c):
        vz_c[4*i + lane] = limb
    return z, vz_c

def fe12_val(z):
    return sum(int(x) for x in z)

def fe12x4_val(z, lane):
    return sum(int(x) for x in z[lane::4])

def make_fe10(initial_value=[]):
    z = F(0)
    z_c = fe10_type(0)
    exponent = 0
    for i, limb in enumerate(initial_value):
        z += limb * 2**exponent
        z_c[i] = limb
        exponent += 26 if i % 2 == 0 else 25
    return z, z_c

def fe10_val(h):
    val = 0
    exponent = 0
    for i, limb in enumerate(h):
        val += limb * 2**exponent
        exponent += 26 if i % 2 == 0 else 25
    return val

def make_fe51(initial_value=[]):
    z = F(0)
    z_c = fe51_type(0)
    exponent = 0
    for i, limb in enumerate(initial_value):
        z += limb * 2**(51*i)
        z_c[i] = limb
    return z, z_c

def fe51_val(h):
    val = 0
    exponent = 0
    for i, limb in enumerate(h):
        val += limb * 2**(51*i)
    return val

def make_ge(x, z, sign):
    if F(z) != 0:
        try:
            point = sign * E.lift_x(F(x))
        except ValueError:
            assume(False)
        x, y = point.xy()
        z = F(z)
        x, y = z * x, z * y
    else:
        point = E(0)
        x, y, z = F(0), F(1), F(z)
    return (x, y, z), point


if __name__ == '__main__':
    unittest.main()
