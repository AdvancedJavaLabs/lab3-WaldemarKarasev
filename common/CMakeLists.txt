cmake_minimum_required(VERSION 3.10)
project(library CXX)

add_library(common-func 
    thread_pool/thread_pool.cpp
)
