#!/bin/bash

defines="-DENGINE"
warnings="-Wno-writable-strings -Wno-format-security -Wno-deprecated-declarations -Wno-switch"
includes="-Ithird_party -Ithird_party/Include"

timestamp=$(date +%s)

if [[ "$(uname)" == "Linux" ]]; then
    echo "Running on Linux"
    libs="-lX11 -lGL -lfreetype"
    outputFile=app

    rm -f game_*
    clang++ -g "src/game.cpp" -shared -fPIC -o game_$timestamp.so $warnings $defines
    mv game_$timestamp.so game.so
fi

clang++ $includes -g src/main.cpp -o$outputFile $libs $warnings $defines
