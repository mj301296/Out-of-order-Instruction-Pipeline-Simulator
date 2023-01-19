/*
 * apex_macros.h
 * Contains APEX cpu pipeline macros
 *
 * Author:
 * Copyright (c) 2020, Gaurav Kothari (gkothar1@binghamton.edu)
 * State University of New York at Binghamton
 */
#ifndef _MACROS_H_
#define _MACROS_H_

#define FALSE 0x0
#define TRUE 0x1

/* Integers */
#define DATA_MEMORY_SIZE 4096

/* Size of integer register file */
#define REG_FILE_SIZE 8
#define PHY_REG_FILE_SIZE 15
#define QUEUE_SIZE 100
#define IQ_SIZE 8
#define ROB_SIZE 12
#define LSQ_SIZE 4
#define BTB_SIZE 4
#define BTS_SIZE 2

/* Numeric OPCODE identifiers for instructions */
#define OPCODE_ADD 0x00
#define OPCODE_SUB 0x01
#define OPCODE_MUL 0x02
#define OPCODE_DIV 0x03
#define OPCODE_AND 0x04
#define OPCODE_OR 0x05
#define OPCODE_XOR 0x06
#define OPCODE_MOVC 0x07
#define OPCODE_LOAD 0x08
#define OPCODE_STORE 0x09
#define OPCODE_BZ 0x0a
#define OPCODE_BNZ 0x0b
#define OPCODE_HALT 0x0c
#define OPCODE_CMP 0x0d
#define OPCODE_ADDL 0x0e
#define OPCODE_SUBL 0x0f
#define OPCODE_NOP 0x10
#define OPCODE_LDR 0x20
#define OPCODE_STR 0x30
#define OPCODE_JMP 0x31

#define INITIALIZE 1
#define SIMULATE 2
#define SINGLE_STEP 3
#define DISPLAY 4
#define SHOWMEM 5
#define DATA_FORWARD 1

/* Set this flag to 1 to enable debug messages */
#define ENABLE_DEBUG_MESSAGES 0

/* Set this flag to 1 to enable cycle single-step mode */
#define ENABLE_SINGLE_STEP 1
#define ENABLE_MULTIPLE_STEP 0

#endif
