#! /bin/bash

if [ %# != 2 ]
then
  echo "without input gyp tools path"
  echo "usage: $0 gyp_tools_path"
else
  GYP_TOOLS=$1
  ${GYP_TOOLS} --depth=. -f make wave_trans.gyp
fi
