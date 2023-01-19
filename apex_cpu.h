/*
 * apex_cpu.h
 * Contains APEX cpu pipeline declarations
 *
 * Author:
 * Copyright (c) 2020, Gaurav Kothari (gkothar1@binghamton.edu)
 * State University of New York at Binghamton
 */
#ifndef _APEX_CPU_H_
#define _APEX_CPU_H_

#include "apex_macros.h"

/* Format of an APEX instruction  */
typedef struct APEX_Instruction
{
    char opcode_str[128];
    int opcode;
    int rd;
    int rs1;
    int rs2;
    int rs3;
    int imm;
    int number;
} APEX_Instruction;

/* Model of CPU stage latch */
typedef struct CPU_Stage
{
    int pc;
    char opcode_str[128];
    int opcode;
    int rs1;
    int rs2;
    int rs3;
    int rd;
    int prs1;
    int prs2;
    int prs3;
    int prd;
    int imm;
    int number;
    int rs1_value;
    int rs2_value;
    int rs3_value;
    int result_buffer;
    int memory_address;
    int cycle; /* To track number of cycles in multiplier and load/store FU*/
    int iq_index;
    int lsq_index;
    int rob_index;
    int branch_predit;
    int branch_identifier;
    int has_insn;
} CPU_Stage;

// typedef struct Issue_Queue
// {
//     int pc;
//     int opcode;
//     int counter;
//     int alloc; /*free =0 allocated =1*/
//     int fu_type_op;
//     int imm;
//     /*scr 1 fields*/
//     int scr1_valid;
//     int scr1_tag;
//     int scr1_value;
//     int cc_flag;
//     /*scr 2 fields*/
//     int scr2_valid;
//     int scr2_tag;
//     int scr2_value;
//     int cc2_flag;

//     int dest_type;   /*LSQ entry = 0, phy reg = 1*/
//     int destination; /*LSQ_entry_head or phy reg address*/
// } Issue_Queue;

typedef struct Issue_Queue
{
    int pc;
    int alloc; /*free =0 allocated =1*/
    int dispatch_counter;
    int fu_type_op;
    /*scr 1 fields*/
    int scr1_valid;
    /*scr 2 fields*/
    int scr2_valid;
    /*scr 3 fields*/
    int scr3_valid;
    int issued;
    CPU_Stage data;
} Issue_Queue;

typedef struct ROB
{
    int ROB_entry;
    //  int instruction_type;
    //  int pc_value;
    //  int dest_phy_reg;
    //  int overwritten_rename_table_entry;
    //  int dest_arch_reg;
    //  int LSQ_index;
    //  int mem_error_code;
    CPU_Stage data;
    int commit_ready;
} ROB;

typedef struct Load_Store_Queue
{
    int LSQ_entry;
    int LOS; /*Load = 1 Store =0 */
    /*scr 1 fields*/
    int scr1_valid;
    /*scr 2 fields*/
    int scr2_valid;
    /*scr 3 fields*/
    int scr3_valid;
    int valid_bit_mem_address;
    int mem_address_field;
    int valid_source_bit;
    CPU_Stage data;
    // int load_dest_reg_address;
    // int src_data_valid;
    // int src_tag;
    // int store_src_value;
    int done_bit;
    int ROB_index;
} Load_Store_Queue;

typedef struct Fwd_Bus
{
    int tag;
    int value;
    int valid;
} Fwd_Bus;

typedef struct Broadcast
{
    int tag;
    int fu_type_op;
    int valid;
} Broadcast;

typedef struct Register
{
    int value;
    int CC_flag;
} Register;

typedef struct BTB
{
    int pc;
    int prediction;
    int target_address;

} BTB;

typedef struct BTS
{
    int pc;
    Register phy_regs[PHY_REG_FILE_SIZE];
    int mapping[REG_FILE_SIZE + 1];
    int free_list_Rear;
    int free_list_Front;

} BTS;

/* Model of APEX CPU */
typedef struct APEX_CPU
{
    int pc;                            /* Current program counter */
    int clock;                         /* Clock cycles elapsed */
    int insn_completed;                /* Instructions retired */
    int regs[REG_FILE_SIZE];           /* Integer register file */
    int state_regs[PHY_REG_FILE_SIZE]; /* State of the registers*/
    int code_memory_size;              /* Number of instruction in the input file */
    APEX_Instruction *code_memory;     /* Code Memory */
    int data_memory[DATA_MEMORY_SIZE]; /* Data Memory */
    int single_step;                   /* Wait for user input after every cycle */
    int zero_flag;                     /* {TRUE, FALSE} Used by BZ and BNZ to branch */
    int fetch_from_next_cycle;
    int command;
    int command_2;
    int instruction_queue[QUEUE_SIZE]; /* Queue to hold instruction*/
    int Rear;
    int Front;
    int free_list[500];
    Register phy_regs[PHY_REG_FILE_SIZE];
    int mapping[REG_FILE_SIZE + 1];
    int free_list_Rear;
    int free_list_Front;

    int ROB_Rear;
    int ROB_Front;

    int LSQ_Rear;
    int LSQ_Front;

    int BTB_Rear;
    int BTB_Front;

    int BTS_Rear;
    int BTS_Front;

    Fwd_Bus fwd_bus[2];
    Broadcast broadcast[2];

    Broadcast broadcast_select[4];
    BTB btb[4];
    BTS bts[2];
    int int_ind;
    int int_tag;
    int logical_ind;
    int logical_tag;
    int mul_ind;
    int mul_tag;
    int load_ind;
    int load_tag;
    int forward;
    int flushed;

    /* Pipeline stages */
    CPU_Stage fetch;
    CPU_Stage decode;
    CPU_Stage dispatch;
    CPU_Stage integer;
    CPU_Stage multiplier;
    CPU_Stage logical;
    CPU_Stage load_store;

    Issue_Queue iq[8];
    ROB rob[12];
    Load_Store_Queue lsq[4];

} APEX_CPU;

APEX_Instruction *create_code_memory(const char *filename, int *size);
APEX_CPU *APEX_cpu_init(const char *filename);
// int APEX_cpu_simulator(APEX_CPU *cpu, const char *command, const char *command2, const char *command3, const char *command4);
int APEX_cpu_simulator(const char *command);
void APEX_cpu_run(APEX_CPU *cpu);
void APEX_cpu_stop(APEX_CPU *cpu);
#endif
