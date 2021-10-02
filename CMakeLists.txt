cmake_minimum_required(VERSION 3.20)
project(C)

set(CMAKE_CXX_STANDARD 14)

add_executable(C main.cpp impl/judgeLetter.cpp headers/judgeLetter.h impl/lexicalAnalysis.cpp headers/lexicalAnalysis.h)
