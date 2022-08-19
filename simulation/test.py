#!/usr/bin/env python3
"""!
@brief      Defines the test class for sound effects.
            Implemented reverb effect.

@file       test.py
@author     Marcin Sosnowski (marcin.sosnow@gmail.com)
@date       2022-08-19

@copyright  copyright GNU Public License.
"""

from playsound import playsound
import wave
import datetime
import os
import struct
from optparse import OptionParser
import numpy

from effects.reverb import Reverb

class Test():
    """ Class to test effect functions

    Class read the wav source sound, apply the filter,
    save the result and play (if enabled).
    """
    def __init__(self,audio_name, effect):
        """ Read the audio file and get the frames samples.
        """
        now_time = datetime.datetime.now()
        self.date_suffix = "{}-{}-{}_{:02d}.{:02d}.{:02d}".format(now_time.year, now_time.month, now_time.day, now_time.hour, now_time.minute, now_time.second)

        self.effect = effect
        self.audio_src=audio_name
        src = wave.open(self.audio_src, 'rb')
        self.audio_nframes = src.getnframes()
        self.audio_nchannels = src.getnchannels()
        self.audio_framerate=src.getframerate()
        self.audio_sampwidth = src.getsampwidth()
        self.audio_comptype = src.getcomptype()
        self.audio_compname = src.getcompname()

        self.sample = src.readframes(self.audio_nframes)
        src.close()

    def info(self):
        """ Print test audio wav file details.
        """
        print('Audio sampwidth: {}'.format(self.audio_sampwidth))
        print('Audio comptype: {}'.format(self.audio_comptype))
        print('Audio compname: {}'.format(self.audio_compname))
        print('Audio frames: {}'.format(self.audio_nframes))
        print('Audio channel type: {}'.format('mono' if self.audio_nchannels == 1 else stero if self.audio_nchannels == 2 else 'unknown'))
        print('Audio frame rate: {}'.format(self.audio_framerate))

    def get_sample(self):
        """ Return samples (byte format) from input file.
        """
        return self.sample

    def play_source(self):
        """ Play source audio file.
        """
        playsound(self.audio_src)

    def delay(self, play=False):
        """ The simplest python test to check how works the delay module.
            One comb module with the same gain and delay have to give the same result.
            For compare pursposes only.
        """
        dest_name='{}_out.wav'.format(self.date_suffix)
        dest = wave.open(dest_name, 'w')
        dest.setparams((self.audio_nchannels, self.audio_sampwidth, self.audio_framerate, self.audio_nframes, 'NONE', 'NONE'))

        dest.writeframes(self.effect.delay(self.sample,(self.audio_nchannels * self.audio_nframes), 0.697, 5801))

        dest.close()
        if play is True:
            playsound(dest_name)

    def reverb(self, play=False):
        """ Method call the reverb test with specified filter parameter.
        """
        dest_name='{}_comb.wav'.format(self.date_suffix)
        dest = wave.open(dest_name, 'w')
        dest.setparams((self.audio_nchannels, self.audio_sampwidth, self.audio_framerate, self.audio_nframes, 'NONE', 'NONE'))

        dest.writeframes(self.effect.test_reverb(self.sample,(self.audio_nchannels * self.audio_nframes),
                        [0.697, 0.715, 0.733, 0.742], [5801, 5399, 4999,4799],
                        [0.7, 0.7, 0.7],[1051, 337, 113]))

        dest.close()
        if play is True:
            playsound(dest_name)

if __name__ == "__main__":
    parser = OptionParser()
    parser.add_option("--source", action="store", dest="source", help="source")
    (options, args) = parser.parse_args()
    # print("%s"%options)

    if options.source is None:
        parser.error("parameter source [--source] is required")

    reverb = Reverb()

    effect=Test(options.source, reverb)
    effect.info()
    # effect.play_source()
    effect.delay(play=False)
    effect.reverb(play=True)

    exit(0)

