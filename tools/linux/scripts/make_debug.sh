#! /bin/bash
maker() {

    if [[ $1 =~ "debug" ]]; then
        type=Debug
    else
        type=Release
    fi
    echo $1
    echo "type=$type"
    [[ ! -d build_$type ]] && mkdir build_$type
    cmake -Bbuild_$type -DCMAKE_BUILD_TYPE=$type -DOpenGL_GL_PREFERENCE=GLVND
    cmake --build build_$type -j $(nproc)

}

maker $0
ln -sf build_$type build
