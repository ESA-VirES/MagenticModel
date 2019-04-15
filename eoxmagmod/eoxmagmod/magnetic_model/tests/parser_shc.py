#-------------------------------------------------------------------------------
#
#  SHC format parser - test
#
# Author: Martin Paces <martin.paces@eox.at>
#
#-------------------------------------------------------------------------------
# Copyright (C) 2018 EOX IT Services GmbH
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies of this Software or works derived from this Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
#-------------------------------------------------------------------------------
# pylint: disable=missing-docstring

from unittest import TestCase, main
from numpy import abs as aabs
from eoxmagmod.magnetic_model.parser_shc import parse_shc_file
from eoxmagmod.data import (
    CHAOS6_CORE_LATEST, CHAOS6_STATIC,
    IGRF12, SIFM,
)

class TestSHCParser(TestCase):

    @staticmethod
    def parse(filename):
        with open(filename) as file_in:
            return parse_shc_file(file_in)

    def _assert_valid(self, data, expected_data):
        tested_data = {
            key: data[key] for key in expected_data
        }
        self.assertEqual(tested_data, expected_data)
        self.assertEqual(data["t"].size, data["gh"].shape[1])
        self.assertEqual(data["nm"].shape[0], data["gh"].shape[0])
        self.assertEqual(data["nm"].shape[1], 2)
        self.assertEqual(data["nm"][..., 0].min(), data["degree_min"])
        self.assertEqual(data["nm"][..., 0].max(), data["degree_max"])
        self.assertTrue(aabs(data["nm"][..., 1]).max() <= data["degree_max"])

    def test_parse_shc_file_sifm(self):
        data = self.parse(SIFM)
        self._assert_valid(data, {
            "degree_min": 1,
            "degree_max": 70,
            "spline_order": 2,
            "ntime": 2,
            "nstep": 1,
        })

    def test_parse_shc_file_igrf12(self):
        data = self.parse(IGRF12)
        self._assert_valid(data, {
            "degree_min": 1,
            "degree_max": 13,
            "spline_order": 2,
            "ntime": 25,
            "nstep": 1,
        })


    def test_parse_shc_file_chaos6core_latest(self):
        data = self.parse(CHAOS6_CORE_LATEST)
        self._assert_valid(data, {
            "degree_min": 1,
            "degree_max": 20,
            "spline_order": 6,
            "ntime": 227,
            "nstep": 5,
        })

    def test_parse_shc_file_chaos6static(self):
        data = self.parse(CHAOS6_STATIC)
        self._assert_valid(data, {
            "degree_min": 21,
            "degree_max": 110,
            "spline_order": 1,
            "ntime": 1,
            "nstep": 1,
        })

if __name__ == "__main__":
    main()
