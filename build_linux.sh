#! /bin/bash


GYP_TOOLS="/root/gyp/gyp"


${GYP_TOOLS} --depth=. -f make wave_trans.gyp
