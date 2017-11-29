
Data Quality Management (DQM) viewer [version 0]
------------------------------------------------

1. INTRO:

The DQM viewer allows you to compare quickly two ADST periods, 
respectively called the REFerence period and the DQM period. 

The package consists of two parts:
   * Part 1: in DQMadst/, you'll find a program that creates the 
             period files (in regular ROOT format) from the ADST. 
             Just run ./CreatePeriodFile without argument for usage 
             instructions.
   * Part 2: in DQMgui/, the program that reads the period files 
             created in Part 1. Once again, simply run ./DQMviewer
             without argument for usage instructions.


2. COMPILATION:

Each directory contains a Makefile that should allow for easy compilation
of the programs, provided that you have the right paths and standard 
variables defined. However, to compile the GUI, you need to do one more
step, which is to create a dictionary for signal / slot. Since the GUI
does not use ADST libraries, it can be run using either ROOT 5 or ROOT 6.

The command to compile the dictionary however are different.
   * If you use ROOT 5, the command is:
      rootcint -f DQMviewerdict.cc -c DQMviewer.h LinkDef.h
   * If you use ROOT 6, the command is:
      rootcling -f DQMviewerdict.cc -c DQMviewer.h LinkDef.h

If you run into issues, first delete all the DQMviewerdict files and perform
a make clean.


3. CUSTOMIZATION:

You can add histograms to the GUI easily by editing the CreatePeriodFile.cc
file. Simply follow the way histograms are being created, filled and saved 
throughout the program. This should be straightforward as long as you follow
the way the histograms are called, etc...  The GUI works out of a list of 
objects saved in the ROOT files... it will automatically add or remove tabs 
when histograms are added or removed. 

If you want to do something fancier (like the average LDFs for example) that 
requires special treatment in the GUI, you will need to develop those methods 
in the GUI itself. See code.


4. VERSION 0:

This is a working version that needs some fine-tuning and a little bit of 
cleaning. Objective is to release v1 in September or so.


5. COMMENTS / QUESTIONS:

Fred Sarazin, Colorado School of Mines, fsarazin@mines.edu


