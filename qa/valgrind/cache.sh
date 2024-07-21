#!/bin/sh
valgrind --tool=cachegrind --cache-sim=yes ./glos
