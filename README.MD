# Reader for .csv files written on C

# Structure of project
* main.c - entry point, cmd line arguments handling;
* csv.c - file reading and table write logic;
* eval.c - expression parsing and calculation logic;
* csv.h - definitions structures and prototypes;
* tests/ - folder with test CSV files;
* test.ps1 - automated testing script (Windows PS)
* test.sh - automated testing script (Linux)

# Compilation
``gcc main.c eval.c csv.c -o csvreader.exe``

# Usage

* on Linux:
``./csvreader tests/main.csv``

* on Windows:
``.\csvreader.exe tests\main.csv``
