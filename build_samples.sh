src_dir=samples
type=Release
dirs=$(find $src_dir -maxdepth 1 -type d|sort )
old_pwd=$PWD
count=1
args=$@
compile_and_build() {
    mdir=$1
    echo "compile and build ${mdir##*/}"
    cmake_dir=$mdir/proj/cmake
    if [ ! -d $cmake_dir ]; then
       echo "$cmake_dir does not exist"
       return
    fi
    cd $cmake_dir
    [[ ! -d build ]] && mkdir build
    cd build
    if [ ! -f CMakeCache.txt ];  then
        cmake -D CMAKE_BUILD_TYPE=$type ..
    else
        if [ -z ${args[1]} ]; then
            #rm CMakeCache.txt
            rm * -rf
            cmake -D CMAKE_BUILD_TYPE=$type ..
        fi
    fi
    make -j $(nproc) &
    cd $old_pwd
    echo "#################################"
}
for mdir in $dirs; do
    first_char=${mdir:${#src_dir}+1:1}
    if [[ $first_char =~ [A-Z] ]]; then
        str="$count:${mdir##*/}"
        len=$((100-${#str}))
        printf '=%.0s' {1..50} 
        echo -e "\n"$str
        compile_and_build $mdir 
        count=$((count+1))
    fi
done
wait
echo "Done"
