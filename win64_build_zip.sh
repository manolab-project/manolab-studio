#!/bin/bash

echo "============  CREATE WIN64 DELIVERY  ============"
cp -R ./bin ./delivery/
cp -R ../workspace ./delivery/bin
cp ./src/build/Debug/manolab-studio.exe ./delivery/bin

