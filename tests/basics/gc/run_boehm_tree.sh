#!/bin/sh


echo >c1 "FREE_LEVEL=1  meant to cut root references that have an age of 1 iteration"
make a CFLAGS=-DCUT FL=1 >>c1

echo >c5 "FREE_LEVEL=5  meant to cut root references that have an age of 5 iteration"
make a CFLAGS=-DCUT FL=5 >>c5

echo >c10 "cut after 20 iter old obj."
make a CFLAGS=-DCUT FL=20 >>c10

