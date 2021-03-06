#!/usr/bin/env python3

"""Routines for comparing timedata performance with pure Python - but since that
was discovered early on to be more than an order of magnitude slower and more
than half an order of magnitude bigger in memory, benchmarking has more been
about comparing versions of timedata with itself, so this file is not being
maintained....

"""
import benchmark
import timedata, timeit

SIZE = 10240

CLASSIC = [0] * (3 * SIZE)
TRIPLES = [(0, 0, 0)] * SIZE

def make_list(maker):
    m = maker()
    m.resize(SIZE)
    return m

TIMEDATA = make_list(timedata.ColorList)

def scale_functional(colors):
    return [c * 2 for c in colors]

def scale_classic(colors):
    for i in range(len(colors)):
        colors[i] *= 2

def scale_triples(colors):
    for i, c in enumerate(colors):
        r, g, b = c
        colors[i] = (2 * r, 2 * g, 2 * b)

def scale_timedata(colors):
    colors.add(colors)

def run(command, value, number=200):
    return 100 * timeit.Timer(lambda: command(value)).timeit(number=number)

print('scale\n')
print('func:     ', run(scale_functional, CLASSIC))
print('classic:  ', run(scale_classic, CLASSIC))
print('triples:  ', run(scale_triples, TRIPLES))
print('timedata:     ', run(scale_timedata, TIMEDATA))


def gamma_classic(colors):
    for i in range(len(colors)):
        colors[i] **= 1.2

def gamma_triples(colors):
    for i, c in enumerate(colors):
        r, g, b = c
        colors[i] = (r ** 1.2, g ** 1.2, g ** 1.2)

def gamma_timedata(colors):
    colors.pow(1.2)

# From https://github.com/scottjgibson/PixelPi/blob/master/pixelpi.py
LPD8806 = [int(pow(float(i) / 255.0, 2.5) * 255.0 + 0.5) for i in range(256)]
APA102 = LPD8806
WS2801 = [int(pow(float(i) / 255.0, 2.5) * 255.0) for i in range(256)]
SM16716 = [int(pow(float(i) / 255.0, 2.5) * 255.0) for i in range(256)]

# From http://rgb-123.com/ws2812-color-output/
WS2812B = NEOPIXEL = WS2812 = [int(pow(float(i) / 255.0, 1.0 / 0.45) * 255.0) for i in range(256)]

def gamma_classic_table(colors):
    for i in range(len(colors)):
        colors[i] = LPD8806[colors[i]]


print('\n\ngamma\n')
print('classic:      ', run(gamma_classic, CLASSIC))

CLASSIC = [int(c) for c in CLASSIC] # clear it out!
print('classic table:', run(gamma_classic_table, CLASSIC))
print('triples:      ', run(gamma_triples, TRIPLES))
print('timedata:         ', run(gamma_timedata, TIMEDATA))


print('\n\nsizes\n')
print('classic:', benchmark.getsize(CLASSIC))
print('triples:', benchmark.getsize(TRIPLES))
print('timedata:   ', benchmark.getsize(TIMEDATA))
