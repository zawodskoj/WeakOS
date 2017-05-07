#!/bin/bash

./build.sh && ./mkimg.sh && bochs -f conf.bxrc
