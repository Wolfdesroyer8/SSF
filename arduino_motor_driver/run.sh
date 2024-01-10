#!/bin/sh

set -xe

arduino-cli compile
arduino-cli upload

minicom -D /dev/ttyUSB0 -b 9600
