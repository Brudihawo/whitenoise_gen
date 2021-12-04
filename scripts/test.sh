#!/bin/bash

make && ./whgen

python ./plot_data.py
