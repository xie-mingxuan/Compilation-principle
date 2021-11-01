cmake_minimum_required(VERSION 3.20)
project(C)

set(CMAKE_CXX_STANDARD 14)

add_executable(C
        main.cpp
        headers/antiPolandExp.h headers/grammarAnalysis.h headers/judgeLetter.h headers/lexicalAnalysis.h
        impl/antiPolandExp.cpp impl/grammarAnalysis.cpp impl/judgeLetter.cpp impl/lexicalAnalysis.cpp)
