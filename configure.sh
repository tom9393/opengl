#!/bin/bash

cmake -S . -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

path="/home/tom/Coding/c++/opengl/compile_commands.json"

if [[ -L ${path} ]]; then
	echo "Link already exists, skipping.."
else
	ln -s build/compile_commands.json
	echo "Link created"
fi
