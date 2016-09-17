#! /bin/sh

if [ -z "$CXX" ]
    then CXX="g++"
fi

if [ "$1" = "clean" ]; then rm -r obj rpl; exit; fi

SOURCES=$(echo src/*.cpp)
CXXFLAGS="-std=c++11 -Wall -Wextra -pedantic"

if [ "$CXX" = "clang++" ]
    then CXXFLAGS="$CXXFLAGS -Wno-nested-anon-types"
    else CXXFLAGS="$CXXFLAGS"
fi

mkdir -p obj
for source in $SOURCES; do $CXX $CXXFLAGS -c $source \
    -o $(echo $source | sed "s:src/\(.*\)\.cpp:obj/\1.o:") || exit; done

$CXX $CXXFLAGS obj/*.o -o rpl
