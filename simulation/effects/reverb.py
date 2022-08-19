"""!
@brief      Defines the reverb test class for sound effects.
            Implemented reverb test class for testing reverb.c
            implementation using ctypes.

@file       reverb.py
@author     Marcin Sosnowski (marcin.sosnow@gmail.com)
@date       2022-08-19

@copyright  copyright GNU Public License.
"""

import struct
import numpy

from ctypes import *

class Reverb():
    """ Class for testing reverb effect implemented in C using ctypes.
    """
    def delay(self, samples, samples_n, amp, ndel):
        """ The simplest python test to check how works the delay module.
            One comb module with the same gain and delay have to give the same result.
            For compare pursposes only.
        """
        str_sample = struct.unpack('{n}h'.format(n = samples_n), samples)

        int_sample = numpy.empty(samples_n,dtype = numpy.int16)
        for idx in range(samples_n):
            int_sample[idx] = int(str_sample[idx])

        out_samples = int_sample
        out_samples = numpy.pad(int_sample, (0,ndel), 'constant')
        tmp_samples = amp*numpy.pad(int_sample, (ndel,0), 'constant')
        tmp_samples = numpy.round(tmp_samples).astype('int16')
        out_samples += (tmp_samples)

        return bytes(out_samples)

    def test_reverb(self, samples, samples_n, amp, ndel, ap_amp, ap_ndel):
        """ Using reverb.so library method run the effect.
        """
        str_sample = struct.unpack('{n}h'.format(n = samples_n), samples)
        libreverb = CDLL("../build/libreverb.so")

        out_samples = numpy.empty(samples_n,dtype = numpy.int16)

        # print(libreverb.reverb_init(5))
        print("Reverb init result: {}".format(libreverb.reverb_init(ndel[0])))
        for idx in range(samples_n):
        # for idx in range(10):
            int_sample = int(str_sample[idx])
            out_samples[idx]=libreverb.reverb(c_int16(int_sample), c_float(amp[0]), c_float(amp[1]), c_int16(ndel[1]),
                                    c_float(amp[2]), c_int16(ndel[2]), c_float(amp[3]), c_int16(ndel[3]),
                                    c_float(ap_amp[0]), c_int16(ap_ndel[0]),
                                    c_float(ap_amp[1]), c_int16(ap_ndel[1]),
                                    c_float(ap_amp[2]), c_int16(ap_ndel[2])
                                    )

        print("Reverb deinit result: {}".format(libreverb.reverb_deinit()))
        return bytes(out_samples)
