@echo off

clang-format -style=file -i .\cli\*.cpp
clang-format -style=file -i .\cli\*.hpp
clang-format -style=file -i .\compiler\*.cpp
clang-format -style=file -i .\compiler\*.hpp
clang-format -style=file -i .\include\fla\*.h
