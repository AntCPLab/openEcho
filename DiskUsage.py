import os
import pathlib

bin_files = pathlib.Path("Player-Data").glob("echo-bin-prep-data-*")
ring_files = pathlib.Path("Player-Data/3-echo-R-64")
ring_files = ring_files.rglob("*")

size = 0

for bin_file in bin_files:
    size += bin_file.stat().st_size

for ring_file in ring_files:
    size += ring_file.stat().st_size

print(size / 1024 / 1024 / 1024, "GB")