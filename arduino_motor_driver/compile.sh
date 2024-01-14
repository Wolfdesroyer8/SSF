#!/bin/sh

set -xe

arduino-cli compile
arduino-cli upload
