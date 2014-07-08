Introduction
------------

This program will simply output a sine wave of the desired parameters.  It can
use SDL to output to your sound card, or dump to a file.

See the manpage for full details.

Quick Overview
--------------

Some examples::

  # Play e, e#, f
  tune --start e --end f --loop
  # Play e, f, g, a'.
  tune --start e --number 4 --step 2
  # Play starting at A, one octave up from concert pitch.
  tune --start a+
  # Play a, b, c
  tune --loop a b c

Convert raw samples like::

  aplay -c 1 -f S16_[LE|BE] -r 44100 sinewave.raw

And encode them with ffmpeg::

  ffmpeg -f u16le -acodec pcm_s16le -i sinewave.raw -acodec pcm_s16le -ac 1 output.wav

Installation
------------

Use cmake as normal.  Dragons here since I never updated the buildsystem for
quite some time::

  $ sudo aptitude install libsdl1.2-dev libboost-program-options-dev cmake
  $ mkdir _build
  $ cd _build
  $ cmake ..
  $ make

There is currently no install target.
