# APEX Pipeline Simulator v2.0
A template for APEX Out-of-Order Pipeline


# MEMBERS
Mrugank Jadhav
Amitesh Dubey
Rujuta Vaidya

## Notes:

 - This code is a simple implementation template of a working 5-Stage APEX Out-of-order Pipeline
 - Implementation is in `C` language
 - Stages: Fetch -> Decode/Rename1 -> Decode/Dispatch -> Issue Queue -> FU units -> D-Cache -> Commit
 - You can read, modify and build upon given code-base to add other features as required in project description
 - You are also free to write your own implementation from scratch
 - All the stages have latency of one cycle expect MUL FU which has a latency of 4 cycles
 - Logic to check data dependencies has been included
 - Includes logic for `ADD`, `ADDL`, `SUB`, `SUBL`, `MUL`, `DIV`, `LOAD`, `STORE`, `LDR`,  `STR`, `JUMP`, `CMP`, `NOP` and`HALT` instructions
 - On fetching `HALT` instruction, fetch stage stop fetching new instructions
 - When `HALT` instruction is in commit stage, simulation stops

## Files:

 - `Makefile`
 - `file_parser.c` - Functions to parse input file
 - `apex_cpu.h` - Data structures declarations
 - `issue_queue.h` - Issue Queue functions
 - `lsq.h` - Load Store functions
 - `rob.h` - Reorder Buffer functions
 - `renaming.h` - Renaming logic functions
 - `forwarding.h` - Early broadcast and Forwarding bus functions
 - `.h` - Branch Target Buffer functions
 - `BTB.h` - Branch Target Buffer functions
 - `BTS.h` - Branch Target Buffer functions
 - `apex_cpu.c` - Implementation of APEX cpu
 - `apex_macros.h` - Macros used in the implementation
 - `main.c` - Main function which calls APEX CPU interface
 - `input.asm` - Sample project 3 test case 2
- `input2.asm` - Sample project 3 test case

## How to compile and run

 Go to terminal, `cd` into project directory and type:
```
 make
```
 Run as follows:
```
Simulator functions:
...
 ./apex_sim <input_file_name>
```
 ./apex_sim <input_file_name> simulate
```

 ./apex_sim <input_file_name> simulate <cycles>
```

 ./apex_sim <input_file_name> display
```
 ./apex_sim <input_file_name> display <cycles>
```
## Author

 - Copyright (C) Gaurav Kothari (gkothar1@binghamton.edu)
 - State University of New York, Binghamton

## Bugs

 - Please contact your TAs for any assistance or query
 - Report bugs at: gkothar1@binghamton.edu