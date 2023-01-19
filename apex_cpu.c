/*
 * apex_cpu.c
 * Contains APEX cpu pipeline implementation
 *
 * Author:
 * Copyright (c) 2020, Gaurav Kothari (gkothar1@binghamton.edu)
 * State University of New York at Binghamton
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "apex_cpu.h"
#include "apex_macros.h"
#include "issue_queue.h"
#include "renaming.h"
#include "forwarding.h"
#include "lsq.h"
#include "rob.h"
#include "BTB.h"
#include "BTS.h"
// static int stall = 0;

/* Converts the PC(4000 series) into array index for code memory
 *
 * Note: You are not supposed to edit this function
 */
static int
get_code_memory_index_from_pc(const int pc)
{
    return (pc - 4000) / 4;
}

static void
print_instruction(const CPU_Stage *stage)
{
    switch (stage->opcode)
    {
    case OPCODE_ADD:
    {
        printf("%s,R%d,R%d,R%d ", stage->opcode_str, stage->rd, stage->rs1, stage->rs2);
        break;
    }
    case OPCODE_ADDL:
    {
        printf("%s,R%d,R%d,#%d ", stage->opcode_str, stage->rd, stage->rs1, stage->imm);
        break;
    }
    case OPCODE_SUB:
    {
        printf("%s,R%d,R%d,R%d ", stage->opcode_str, stage->rd, stage->rs1, stage->rs2);
        break;
    }
    case OPCODE_SUBL:
    {
        printf("%s,R%d,R%d,#%d ", stage->opcode_str, stage->rd, stage->rs1, stage->imm);
        break;
    }
    case OPCODE_MUL:
    {
        printf("%s,R%d,R%d,R%d ", stage->opcode_str, stage->rd, stage->rs1, stage->rs2);
        break;
    }
    case OPCODE_DIV:
    {
        printf("%s,R%d,R%d,R%d ", stage->opcode_str, stage->rd, stage->rs1, stage->rs2);
        break;
    }
    case OPCODE_AND:
    {
        printf("%s,R%d,R%d,R%d ", stage->opcode_str, stage->rd, stage->rs1, stage->rs2);
        break;
    }
    case OPCODE_OR:
    {
        printf("%s,R%d,R%d,R%d ", stage->opcode_str, stage->rd, stage->rs1, stage->rs2);
        break;
    }
    case OPCODE_XOR:
    {
        printf("%s,R%d,R%d,R%d ", stage->opcode_str, stage->rd, stage->rs1,
               stage->rs2);
        break;
    }

    case OPCODE_CMP:
    {
        printf("%s,R%d,R%d ", stage->opcode_str, stage->rs1, stage->rs2);
        break;
    }

    case OPCODE_MOVC:
    {
        printf("%s,R%d,#%d ", stage->opcode_str, stage->rd, stage->imm);
        break;
    }

    case OPCODE_LOAD:
    {
        printf("%s,R%d,R%d,#%d ", stage->opcode_str, stage->rd, stage->rs1,
               stage->imm);
        break;
    }

    case OPCODE_STORE:
    {
        printf("%s,R%d,R%d,#%d ", stage->opcode_str, stage->rs1, stage->rs2,
               stage->imm);
        break;
    }

    case OPCODE_LDR:
    {
        printf("%s,R%d,R%d,R%d ", stage->opcode_str, stage->rd, stage->rs1,
               stage->rs2);
        break;
    }

    case OPCODE_STR:
    {
        printf("%s,R%d,R%d,R%d ", stage->opcode_str, stage->rs3, stage->rs1,
               stage->rs2);
        break;
    }

    case OPCODE_BZ:
    {
        printf("%s,#%d ", stage->opcode_str, stage->imm);
        break;
    }
    case OPCODE_BNZ:
    {
        printf("%s,#%d ", stage->opcode_str, stage->imm);
        break;
    }

    case OPCODE_NOP:
    {
        printf("%s", stage->opcode_str);
        break;
    }

    case OPCODE_HALT:
    {
        printf("%s", stage->opcode_str);
        break;
    }
    case OPCODE_JMP:
    {
        printf("%s,R%d,#%d ", stage->opcode_str, stage->rs1, stage->imm);
        break;
    }
    }
}

/* Debug function which prints the CPU stage content
 *
 * Note: You can edit this function to print in more detail
 */
static void
print_stage_content(const char *name, const CPU_Stage *stage)
{
    printf("%-15s: pc(%d) ", name, stage->pc);
    print_instruction(stage);
    printf("\n");
}

print_stage_content_mul(const char *name, const CPU_Stage *stage)
{
    printf("%-5s %d: pc(%d) ", name, stage->cycle, stage->pc);
    print_instruction(stage);
    printf("\n");
}

print_forward_bus(const APEX_CPU *cpu)
{

    printf("----------\n%s\n----------\n", "Forward Bus:");

    for (int i = 0; i < 2; ++i)
    {
        printf("----------\n%s:%d\n----------\n", "Bus", i + 1);
        printf("tag: %d ", cpu->fwd_bus[i].tag);
        printf("value: %d\n", cpu->fwd_bus[i].value);
    }

    printf("\n");
}

/* Debug function which prints the register file
 *
 * Note: You are not supposed to edit this function
 */
static void
print_reg_file(const APEX_CPU *cpu)
{
    int i;

    printf("----------\n%s\n----------\n", "Registers:");

    for (int i = 0; i < REG_FILE_SIZE / 2; ++i)
    {
        printf("R%-3d[%-3d] ", i, cpu->regs[i]);
    }

    printf("\n");

    for (i = (REG_FILE_SIZE / 2); i < REG_FILE_SIZE; ++i)
    {
        printf("R%-3d[%-3d] ", i, cpu->regs[i]);
    }

    printf("\n");
}

static void
print_phy_reg_file(const APEX_CPU *cpu)
{
    // int i;

    printf("----------\n%s\n----------\n", "Physical Registers:");

    for (int i = 0; i < PHY_REG_FILE_SIZE; ++i)
    {
        printf("P%-3d[%-3d] ", i, cpu->phy_regs[i].value);
    }

    // printf("\n");

    // for (i = (PHY_REG_FILE_SIZE / 2); i < REG_FILE_SIZE; ++i)
    // {
    //     printf("P%-3d[%-3d] ", i, cpu->phy_regs[i]);
    // }

    printf("\n");
}
static void
print_rename_table(const APEX_CPU *cpu)
{

    printf("----------\n%s\n----------\n", "Rename table:");

    for (int i = 0; i < REG_FILE_SIZE + 1; ++i)
    {
        printf("R%d = P%d ", i, cpu->mapping[i]);
    }

    printf("\n");
}
static void
print_memory_file(const APEX_CPU *cpu)
{
    int i = 0;
    int start = 0;
    int end = DATA_MEMORY_SIZE;

    printf("----------\n%s\n----------\n", "Data Memory:");
    while (end != 0)
    {
        for (i = start; i < start + 8; ++i)
        {
            if (cpu->data_memory[i] != 0)
            {
                printf("MEM[%-2d]=%-2d ", i, cpu->data_memory[i]);
            }
        }

        //   printf("\n");
        end = end - 8;
        start = start + 8;
    }
    printf("\n");
}
static void
print_zero_flag(const APEX_CPU *cpu)
{
    printf("----------\n%s\n----------\n", "Zero Flag:");
    printf("Z =%d\n", cpu->zero_flag);
}

/*
 * Fetch Stage of APEX Pipeline
 *
 * Note: You are free to edit this function according to your implementation
 */

static void
APEX_fetch(APEX_CPU *cpu)
{
    APEX_Instruction *current_ins;

    if (cpu->fetch.has_insn)
    {
        cpu->fetch.cycle = 0;
        /* This fetches new branch target instruction from next cycle */
        if (cpu->fetch_from_next_cycle == TRUE)
        {
            cpu->fetch_from_next_cycle = FALSE;

            /* Skip this cycle*/
            return;
        }

        /* Store current PC in fetch latch */
        cpu->fetch.pc = cpu->pc;

        /* Index into code memory using this pc and copy all instruction fields
         * into fetch latch  */
        current_ins = &cpu->code_memory[get_code_memory_index_from_pc(cpu->pc)];
        strcpy(cpu->fetch.opcode_str, current_ins->opcode_str);
        cpu->fetch.opcode = current_ins->opcode;
        cpu->fetch.rd = current_ins->rd;
        if ((cpu->fetch.opcode == OPCODE_BNZ) || (cpu->fetch.opcode == OPCODE_BZ))
        {
            cpu->fetch.rs1 = cpu->decode.rd;
        }
        else
        {
            cpu->fetch.rs1 = current_ins->rs1;
        }
        cpu->fetch.rs2 = current_ins->rs2;
        cpu->fetch.rs3 = current_ins->rs3;
        cpu->fetch.imm = current_ins->imm;
        cpu->fetch.number = current_ins->number;
        cpu->fetch.branch_identifier = cpu->bts[cpu->BTS_Front].pc;
        /* Update PC for next instruction */
        // cpu->pc += 4;

        /*Check BTB matchup*/
        if (cpu->flushed == 0)
        {
            cpu->pc = check_BTB_entry(cpu, cpu->pc);
        }
        else
        {
            cpu->pc += 4;
            cpu->flushed = 0;
        }
        /* Copy data from fetch latch to decode latch*/
        cpu->decode = cpu->fetch;

        if ((ENABLE_DEBUG_MESSAGES) || (cpu->command == DISPLAY) || (cpu->command == SINGLE_STEP) || (cpu->command == SIMULATE))
        {
            print_stage_content("Fetch", &cpu->fetch);
        }

        /* Stop fetching new instructions if HALT is fetched */
        if (cpu->fetch.opcode == OPCODE_HALT)
        {
            cpu->fetch.has_insn = FALSE;
        }
    }
}

/*
 * Decode Stage of APEX Pipeline
 *
 * Note: You are free to edit this function according to your implementation
 */
static void
APEX_decode(APEX_CPU *cpu)
{

    if (cpu->decode.has_insn)
    {
        if (cpu->dispatch.has_insn == TRUE)
        {
            cpu->fetch_from_next_cycle = TRUE;
            goto SKIP_DECODE;
        }

        /*MJXX simple scoreboarding logic*/
        /*set the status of registers which are currently being used as 1*/
        /*RENAMING LOGIC*/
        if (cpu->decode.rs1 != 99)
        {
            cpu->decode.prs1 = cpu->mapping[cpu->decode.rs1];
        }
        if (cpu->decode.rs2 != 99)
        {
            cpu->decode.prs2 = cpu->mapping[cpu->decode.rs2];
        }
        if (cpu->decode.rs3 != 99)
        {
            cpu->decode.prs3 = cpu->mapping[cpu->decode.rs3];
        }
        /*RENAMING LOGIC*/

        if ((cpu->decode.opcode != OPCODE_STORE) && (cpu->decode.opcode != OPCODE_STR) && (cpu->decode.opcode != OPCODE_NOP) && (cpu->decode.opcode != OPCODE_BZ) && (cpu->decode.opcode != OPCODE_BNZ) && (cpu->decode.opcode != OPCODE_HALT))
        {
            cpu->decode.prd = cpu->free_list[cpu->free_list_Front];
            //   free_list_enqueue(cpu, cpu->mapping[cpu->decode.rd]);
            free_list_dequeue(cpu);
            cpu->mapping[cpu->decode.rd] = cpu->decode.prd;
            cpu->phy_regs[cpu->decode.prd].CC_flag = 0;
        }
        if ((cpu->decode.opcode == OPCODE_BZ) || (cpu->decode.opcode == OPCODE_BNZ))
        {
            BTS bts_entry;
            bts_entry.pc = cpu->decode.pc;

            for (int i = 0; i < REG_FILE_SIZE; i++)
            {
                bts_entry.mapping[i] = cpu->mapping[i];
            }
            bts_entry.free_list_Front = cpu->free_list_Front;
            bts_entry.free_list_Rear = cpu->free_list_Rear;
            insert_BTS_data(cpu, bts_entry);
        }

        /* MJXX simple scoreboarding logic*/
        /* Set the destination register state indicator till the instruction execution is completed*/
        if ((cpu->decode.opcode != OPCODE_STORE) && (cpu->decode.opcode != OPCODE_STR) && (cpu->decode.opcode != OPCODE_NOP) && (cpu->decode.opcode != OPCODE_BZ) && (cpu->decode.opcode != OPCODE_BNZ))
        {
            cpu->state_regs[cpu->decode.prd] = 1;
        }
        /* Read operands from register file based on the instruction type */
        switch (cpu->decode.opcode)
        {
        case OPCODE_ADD:
        {
            cpu->decode.rs1_value = cpu->phy_regs[cpu->decode.prs1].value;
            cpu->decode.rs2_value = cpu->phy_regs[cpu->decode.prs2].value;
            break;
        }
        case OPCODE_ADDL:
        {
            cpu->decode.rs1_value = cpu->phy_regs[cpu->decode.prs1].value;
            break;
        }
        case OPCODE_SUB:
        {
            cpu->decode.rs1_value = cpu->phy_regs[cpu->decode.prs1].value;
            cpu->decode.rs2_value = cpu->phy_regs[cpu->decode.prs2].value;
            break;
        }
        case OPCODE_SUBL:
        {
            cpu->decode.rs1_value = cpu->phy_regs[cpu->decode.prs1].value;
            break;
        }
        case OPCODE_MUL:
        {
            cpu->decode.rs1_value = cpu->phy_regs[cpu->decode.prs1].value;
            cpu->decode.rs2_value = cpu->phy_regs[cpu->decode.prs2].value;
            break;
        }
        case OPCODE_DIV:
        {
            cpu->decode.rs1_value = cpu->phy_regs[cpu->decode.prs1].value;
            cpu->decode.rs2_value = cpu->phy_regs[cpu->decode.prs2].value;
            break;
        }
        case OPCODE_AND:
        {
            cpu->decode.rs1_value = cpu->phy_regs[cpu->decode.prs1].value;
            cpu->decode.rs2_value = cpu->phy_regs[cpu->decode.prs2].value;
            break;
        }
        case OPCODE_OR:
        {
            cpu->decode.rs1_value = cpu->phy_regs[cpu->decode.prs1].value;
            cpu->decode.rs2_value = cpu->phy_regs[cpu->decode.prs2].value;
            break;
        }
        case OPCODE_XOR:
        {
            cpu->decode.rs1_value = cpu->phy_regs[cpu->decode.prs1].value;
            cpu->decode.rs2_value = cpu->phy_regs[cpu->decode.prs2].value;
            break;
        }
        case OPCODE_CMP:
        {
            cpu->decode.rs1_value = cpu->phy_regs[cpu->decode.prs1].value;
            cpu->decode.rs2_value = cpu->phy_regs[cpu->decode.prs2].value;
            break;
        }

        case OPCODE_LOAD:
        {
            cpu->decode.rs1_value = cpu->phy_regs[cpu->decode.prs1].value;
            break;
        }
        case OPCODE_STORE:
        {
            cpu->decode.rs1_value = cpu->phy_regs[cpu->decode.prs1].value;
            cpu->decode.rs2_value = cpu->phy_regs[cpu->decode.prs2].value;
            break;
        }
        case OPCODE_LDR:
        {
            cpu->decode.rs1_value = cpu->phy_regs[cpu->decode.prs1].value;
            cpu->decode.rs2_value = cpu->phy_regs[cpu->decode.prs2].value;
            break;
        }
        case OPCODE_STR:
        {
            cpu->decode.rs3_value = cpu->phy_regs[cpu->decode.prs3].value;
            cpu->decode.rs1_value = cpu->phy_regs[cpu->decode.prs1].value;
            cpu->decode.rs2_value = cpu->phy_regs[cpu->decode.prs2].value;
            break;
        }
        case OPCODE_JMP:
        {
            cpu->decode.rs1_value = cpu->phy_regs[cpu->decode.prs1].value;
            break;
        }

        case OPCODE_MOVC:
        {
            /* MOVC doesn't have register operands */
            break;
        }
        }
        cpu->dispatch = cpu->decode;

        cpu->decode.has_insn = FALSE;
    SKIP_DECODE:;
        if ((ENABLE_DEBUG_MESSAGES) || (cpu->command == DISPLAY) || (cpu->command == SINGLE_STEP) || (cpu->command == SIMULATE))
        {
            print_stage_content("DR1", &cpu->decode);
        }
    }
}
static void
APEX_dispatch(APEX_CPU *cpu)
{
    if (cpu->dispatch.has_insn)
    {

        if (cpu->fwd_bus[0].valid == 1)
        {
            if (cpu->fwd_bus[0].tag == cpu->dispatch.prs1)
            {
                cpu->dispatch.rs1_value = cpu->fwd_bus[0].value;
            }
            if (cpu->fwd_bus[0].tag == cpu->dispatch.prs2)
            {
                cpu->dispatch.rs2_value = cpu->fwd_bus[0].value;
            }
            if (cpu->fwd_bus[0].tag == cpu->dispatch.prs3)
            {
                cpu->dispatch.rs3_value = cpu->fwd_bus[0].value;
            }
        }
        if (cpu->fwd_bus[1].valid == 1)
        {
            if (cpu->fwd_bus[1].tag == cpu->dispatch.prs1)
            {
                cpu->dispatch.rs1_value = cpu->fwd_bus[1].value;
            }
            if (cpu->fwd_bus[1].tag == cpu->dispatch.prs2)
            {
                cpu->dispatch.rs2_value = cpu->fwd_bus[1].value;
            }
            if (cpu->fwd_bus[1].tag == cpu->dispatch.prs3)
            {
                cpu->dispatch.rs3_value = cpu->fwd_bus[1].value;
            }
        }
        int IQ_index = check_free_IQ_index(cpu);
        int ROB_index = rob_enqueue(cpu, cpu->dispatch);

        if ((ROB_index != 99) && (IQ_index != 99))
        {
            cpu->dispatch.rob_index = ROB_index;
        }
        else
        {
            cpu->fetch_from_next_cycle = TRUE;
        }
        if ((cpu->dispatch.opcode == OPCODE_STR) || (cpu->dispatch.opcode == OPCODE_LDR) || (cpu->dispatch.opcode == OPCODE_LOAD) || (cpu->dispatch.opcode == OPCODE_STORE))
        {
            int LSQ_index = lsq_enqueue(cpu, cpu->dispatch);
            if ((LSQ_index != 99) && (IQ_index != 99) && (ROB_index != 99))
            {
                cpu->dispatch.lsq_index = LSQ_index;
                cpu->dispatch.has_insn = FALSE;
                if ((ENABLE_DEBUG_MESSAGES) || (cpu->command == DISPLAY) || (cpu->command == SINGLE_STEP) || (cpu->command == SIMULATE))
                {
                    print_stage_content("RD2", &cpu->dispatch);
                }
            }
            else
            {
                cpu->fetch_from_next_cycle = TRUE;
            }
            //   cpu->iq[IQ_index].fu_type_op = 1;
        }
        else
        {
            if (IQ_index != 99)
            {
                cpu->iq[IQ_index].alloc = 1;
                /*scr1 fields*/
                if ((cpu->dispatch.opcode != OPCODE_MOVC) && (cpu->dispatch.opcode != OPCODE_BNZ) && (cpu->dispatch.opcode != OPCODE_BZ) && (cpu->dispatch.opcode != OPCODE_JMP))
                {
                    if ((cpu->state_regs[cpu->dispatch.prs1] == 0) || (cpu->broadcast[0].tag == cpu->dispatch.prs1) || (cpu->broadcast[1].tag == cpu->dispatch.prs1))
                    {
                        cpu->iq[IQ_index].scr1_valid = 1;
                    }
                    else
                    {
                        cpu->iq[IQ_index].scr1_valid = 0;
                    }
                }
                else
                {
                    cpu->iq[IQ_index].scr1_valid = 1;
                }
                /*scr 2 fields*/
                if ((cpu->dispatch.opcode != OPCODE_ADDL) && (cpu->dispatch.opcode != OPCODE_SUBL) && (cpu->dispatch.opcode != OPCODE_MOVC) && (cpu->dispatch.opcode != OPCODE_BNZ) && (cpu->dispatch.opcode != OPCODE_BZ) && (cpu->dispatch.opcode != OPCODE_JMP))
                {
                    if ((cpu->state_regs[cpu->dispatch.prs2] == 0) || (cpu->broadcast[0].tag == cpu->dispatch.prs2) || (cpu->broadcast[1].tag == cpu->dispatch.prs2))
                    {
                        cpu->iq[IQ_index].scr2_valid = 1;
                    }
                    else
                    {
                        cpu->iq[IQ_index].scr2_valid = 0;
                    }
                }
                else
                {
                    cpu->iq[IQ_index].scr2_valid = 1;
                }
                /*scr 3 fields*/
                cpu->iq[IQ_index].scr3_valid = 1;
                cpu->dispatch.iq_index = IQ_index;
                if (cpu->dispatch.opcode == OPCODE_MUL)
                {
                    cpu->iq[IQ_index].fu_type_op = 2;
                }
                else if ((cpu->dispatch.opcode == OPCODE_AND) || (cpu->dispatch.opcode == OPCODE_OR) || (cpu->dispatch.opcode == OPCODE_XOR))
                {
                    cpu->iq[IQ_index].fu_type_op = 3;
                }
                else
                {
                    cpu->iq[IQ_index].fu_type_op = 0;
                }
                cpu->iq[IQ_index].data = cpu->dispatch;
                cpu->dispatch.has_insn = FALSE;
                if ((ENABLE_DEBUG_MESSAGES) || (cpu->command == DISPLAY) || (cpu->command == SINGLE_STEP) || (cpu->command == SIMULATE))
                {
                    print_stage_content("RD2", &cpu->dispatch);
                }
            }
            else
            {
                cpu->fetch_from_next_cycle = TRUE;
            }
        }
    }
}

static void
APEX_issue_queue(APEX_CPU *cpu)
{
    update_iq_entries(cpu);
    update_lsq_entries(cpu);
    if (!cpu->integer.has_insn)
    {
        int instruction = fetch_next_instruction(cpu, 0);
        if (instruction != 99)
        {
            cpu->integer = cpu->iq[instruction].data;
            cpu->iq[instruction].issued = 1;
            cpu->int_ind = 1;
            cpu->int_tag = cpu->iq[instruction].data.prd;
            // if ((ENABLE_DEBUG_MESSAGES) || (cpu->command == DISPLAY) || (cpu->command == SINGLE_STEP))
            // {
            //     print_stage_content("ISSUE_QUEUE", &cpu->iq[instruction].data);
            // }
        }
    }
    if (!cpu->logical.has_insn)
    {
        int instruction = fetch_next_instruction(cpu, 3);
        if (instruction != 99)
        {
            cpu->logical = cpu->iq[instruction].data;
            cpu->iq[instruction].issued = 1;
            cpu->logical_ind = 1;
            cpu->logical_tag = cpu->iq[instruction].data.prd;
            // if ((ENABLE_DEBUG_MESSAGES) || (cpu->command == DISPLAY) || (cpu->command == SINGLE_STEP))
            // {
            //     print_stage_content("ISSUE_QUEUE", &cpu->iq[instruction].data);
            // }
        }
    }
    if (!cpu->multiplier.has_insn)
    {
        int instruction = fetch_next_instruction(cpu, 2);
        if (instruction != 99)
        {
            cpu->multiplier = cpu->iq[instruction].data;
            cpu->iq[instruction].issued = 1;
            // if ((ENABLE_DEBUG_MESSAGES) || (cpu->command == DISPLAY) || (cpu->command == SINGLE_STEP))
            // {
            //     print_stage_content("ISSUE_QUEUE", &cpu->iq[instruction].data);
            // }
        }
    }
}
/*
 * Execute Stage of APEX Pipeline
 *
 * Note: You are free to edit this function according to your implementation
 */
static void
APEX_logical_fu(APEX_CPU *cpu)
{
    if (cpu->logical.has_insn)
    {
        if ((cpu->logical.opcode != OPCODE_STORE) && (cpu->logical.opcode != OPCODE_STR) && (cpu->logical.opcode != OPCODE_CMP) && (cpu->logical.opcode != OPCODE_NOP) && (cpu->logical.opcode != OPCODE_BZ) && (cpu->logical.opcode != OPCODE_BNZ))
        {
            cpu->state_regs[cpu->logical.prd] = 1;
        }
        /* Execute logic based on instruction type */
        switch (cpu->logical.opcode)
        {
        case OPCODE_AND:
        {
            cpu->logical.result_buffer = cpu->logical.rs1_value & cpu->logical.rs2_value;
            cpu->phy_regs[cpu->logical.prd].value = cpu->logical.result_buffer;
            /* Set the zero flag based on the result buffer */
            if (cpu->logical.result_buffer == 0)
            {
                cpu->zero_flag = TRUE;
            }
            else
            {
                cpu->zero_flag = FALSE;
            }
            cpu->phy_regs[cpu->logical.prd].CC_flag = cpu->zero_flag;
            break;
        }
        case OPCODE_OR:
        {
            cpu->logical.result_buffer = cpu->logical.rs1_value | cpu->logical.rs2_value;
            cpu->phy_regs[cpu->logical.prd].value = cpu->logical.result_buffer;
            /* Set the zero flag based on the result buffer */
            if (cpu->logical.result_buffer == 0)
            {
                cpu->zero_flag = TRUE;
            }
            else
            {
                cpu->zero_flag = FALSE;
            }
            cpu->phy_regs[cpu->logical.prd].CC_flag = cpu->zero_flag;
            break;
        }
        case OPCODE_XOR:
        {
            cpu->logical.result_buffer = cpu->logical.rs1_value ^ cpu->logical.rs2_value;
            cpu->phy_regs[cpu->logical.prd].value = cpu->logical.result_buffer;
            /* Set the zero flag based on the result buffer */
            if (cpu->logical.result_buffer == 0)
            {
                cpu->zero_flag = TRUE;
            }
            else
            {
                cpu->zero_flag = FALSE;
            }
            cpu->phy_regs[cpu->logical.prd].CC_flag = cpu->zero_flag;
            break;
        }
        }
        /* Forward value to next adjacent instruction*/
        if ((cpu->broadcast[0].fu_type_op == 3) && (cpu->broadcast[0].valid == 1))
        {
            cpu->fwd_bus[0].tag = cpu->logical.prd;
            cpu->fwd_bus[0].value = cpu->logical.result_buffer;
            cpu->fwd_bus[0].valid = 1;
        }
        if ((cpu->broadcast[0].fu_type_op == 3) && (cpu->broadcast[0].valid == 1))
        {
            cpu->fwd_bus[1].tag = cpu->logical.prd;
            cpu->fwd_bus[1].value = cpu->logical.result_buffer;
            cpu->fwd_bus[1].valid = 1;
        }
        if (((cpu->broadcast[0].fu_type_op == 3) && (cpu->broadcast[0].valid == 1)) || ((cpu->broadcast[1].fu_type_op == 3) && (cpu->broadcast[1].valid == 1)))
        {

            cpu->state_regs[cpu->logical.prd] = 0;
            free_iq(cpu, cpu->logical.iq_index);
            update_rob(cpu, cpu->logical);
            cpu->logical.has_insn = FALSE;
        }
        else
        {
            cpu->logical_tag = 1;
            cpu->logical_tag = cpu->logical.prd;
        }
        if ((ENABLE_DEBUG_MESSAGES) || (cpu->command == DISPLAY) || (cpu->command == SINGLE_STEP) || (cpu->command == SIMULATE))
        {
            print_stage_content("Logical FU", &cpu->logical);
        }
    }
}
static void
APEX_integer_fu(APEX_CPU *cpu)
{
    if (cpu->integer.has_insn)
    {
        if ((cpu->integer.opcode != OPCODE_STORE) && (cpu->integer.opcode != OPCODE_STR) && (cpu->integer.opcode != OPCODE_NOP) && (cpu->integer.opcode != OPCODE_BZ) && (cpu->integer.opcode != OPCODE_BNZ))
        {
            cpu->state_regs[cpu->integer.prd] = 1;
        }
        /* Execute logic based on instruction type */
        switch (cpu->integer.opcode)
        {
        case OPCODE_ADD:
        {
            cpu->integer.result_buffer = cpu->integer.rs1_value + cpu->integer.rs2_value;
            cpu->phy_regs[cpu->integer.prd].value = cpu->integer.result_buffer;
            /* Set the zero flag based on the result buffer */
            if (cpu->integer.result_buffer == 0)
            {
                cpu->zero_flag = TRUE;
            }
            else
            {
                cpu->zero_flag = FALSE;
            }
            cpu->phy_regs[cpu->integer.prd].CC_flag = cpu->zero_flag;
            break;
        }
        case OPCODE_ADDL:
        {
            cpu->integer.result_buffer = cpu->integer.rs1_value + cpu->integer.imm;
            cpu->phy_regs[cpu->integer.prd].value = cpu->integer.result_buffer;
            /* Set the zero flag based on the result buffer */
            if (cpu->integer.result_buffer == 0)
            {
                cpu->zero_flag = TRUE;
            }
            else
            {
                cpu->zero_flag = FALSE;
            }
            cpu->phy_regs[cpu->integer.prd].CC_flag = cpu->zero_flag;
            break;
        }
        case OPCODE_SUB:
        {
            cpu->integer.result_buffer = cpu->integer.rs1_value - cpu->integer.rs2_value;
            cpu->phy_regs[cpu->integer.prd].value = cpu->integer.result_buffer;
            /* Set the zero flag based on the result buffer */
            if (cpu->integer.result_buffer == 0)
            {
                cpu->zero_flag = TRUE;
            }
            else
            {
                cpu->zero_flag = FALSE;
            }
            cpu->phy_regs[cpu->integer.prd].CC_flag = cpu->zero_flag;
            break;
        }
        case OPCODE_SUBL:
        {
            cpu->integer.result_buffer = cpu->integer.rs1_value - cpu->integer.imm;
            cpu->phy_regs[cpu->integer.prd].value = cpu->integer.result_buffer;
            /* Set the zero flag based on the result buffer */
            if (cpu->integer.result_buffer == 0)
            {
                cpu->zero_flag = TRUE;
            }
            else
            {
                cpu->zero_flag = FALSE;
            }
            cpu->phy_regs[cpu->integer.prd].CC_flag = cpu->zero_flag;
            break;
        }
        case OPCODE_DIV:
        {
            cpu->integer.result_buffer = cpu->integer.rs1_value / cpu->integer.rs2_value;
            cpu->phy_regs[cpu->integer.prd].value = cpu->integer.result_buffer;
            /* Set the zero flag based on the result buffer */
            if (cpu->integer.result_buffer == 0)
            {
                cpu->zero_flag = TRUE;
            }
            else
            {
                cpu->zero_flag = FALSE;
            }
            cpu->phy_regs[cpu->integer.prd].CC_flag = cpu->zero_flag;
            break;
        }

        case OPCODE_CMP:
        {
            /*   cpu->integer.result_buffer = cpu->integer.rs1_value - cpu->integer.rs2_value;*/
            /* Set the zero flag based on the result buffer */
            if ((cpu->integer.rs1_value - cpu->integer.rs2_value) == 0)
            {
                cpu->zero_flag = TRUE;
            }
            else
            {
                cpu->zero_flag = FALSE;
            }
            cpu->phy_regs[cpu->integer.prd].CC_flag = cpu->zero_flag;
            break;
        }

        case OPCODE_BZ:
        {
            BTB btb_entry;
            if (cpu->integer.branch_predit == 1)
            {
                if (cpu->integer.imm < 0)
                {

                    if (cpu->phy_regs[cpu->integer.prs1].CC_flag == FALSE)
                    {
                        cpu->flushed = 1;
                        /* Calculate new PC, and send it to fetch unit */
                        cpu->pc = cpu->integer.pc + 4;

                        /* Since we are using reverse callbacks for pipeline stages,
                         * this will prevent the new instruction from being fetched in the current cycle*/
                        cpu->fetch_from_next_cycle = TRUE;
                        /*Initilizing with value with random value 55 so that reg R0 doesn't match with inititialized value*/

                        /* Flush previous stages */
                        cpu->decode.has_insn = FALSE;
                        cpu->dispatch.has_insn = FALSE;
                        init_iq(cpu, cpu->integer.iq_index);
                        cpu->ROB_Rear = cpu->integer.rob_index;
                        for (int i = 0; i < REG_FILE_SIZE; i++)
                        {
                            cpu->mapping[i] = cpu->bts[cpu->BTB_Front].mapping[i];
                        }
                        for (int i = cpu->LSQ_Front; i != cpu->LSQ_Rear; i = (i + 1) % LSQ_SIZE)
                        {
                            if (cpu->lsq[i].data.branch_identifier == cpu->bts[cpu->BTS_Front].pc)
                            {
                                cpu->LSQ_Front = (cpu->lsq[i].data.lsq_index + 1);
                            }
                        }
                        //    bts_entry.mapping = cpu->mapping;
                        cpu->free_list_Front = cpu->bts[cpu->BTB_Front].free_list_Front;
                        cpu->free_list_Rear = cpu->bts[cpu->BTB_Front].free_list_Rear;

                        /* Make sure fetch stage is enabled to start fetching from new PC */
                        cpu->fetch.has_insn = TRUE;
                    }
                }
                else
                {
                    if (cpu->phy_regs[cpu->integer.prs1].CC_flag == TRUE)
                    {
                        /* Calculate new PC, and send it to fetch unit */
                        //    cpu->pc = cpu->integer.pc + cpu->integer.imm;
                        cpu->flushed = 1;
                        cpu->pc = cpu->integer.pc + cpu->integer.imm;
                        /* Since we are using reverse callbacks for pipeline stages,
                         * this will prevent the new instruction from being fetched in the current cycle*/
                        cpu->fetch_from_next_cycle = TRUE;
                        /*Initilizing with value with random value 55 so that reg R0 doesn't match with inititialized value*/

                        /* Flush previous stages */
                        cpu->decode.has_insn = FALSE;
                        cpu->dispatch.has_insn = FALSE;
                        init_iq(cpu, cpu->integer.iq_index);
                        cpu->ROB_Rear = cpu->integer.rob_index;
                        for (int i = 0; i < REG_FILE_SIZE; i++)
                        {
                            cpu->mapping[i] = cpu->bts[cpu->BTB_Front].mapping[i];
                        }
                        for (int i = cpu->LSQ_Front; i != cpu->LSQ_Rear; i = (i + 1) % LSQ_SIZE)
                        {
                            if (cpu->lsq[i].data.branch_identifier == cpu->bts[cpu->BTS_Front].pc)
                            {
                                cpu->LSQ_Front = (cpu->lsq[i].data.lsq_index + 1);
                            }
                        }
                        //    bts_entry.mapping = cpu->mapping;
                        cpu->free_list_Front = cpu->bts[cpu->BTB_Front].free_list_Front;
                        cpu->free_list_Rear = cpu->bts[cpu->BTB_Front].free_list_Rear;

                        /* Make sure fetch stage is enableed to start fetching from new PC */
                        cpu->fetch.has_insn = TRUE;
                    }
                }
            }
            else
            {
                btb_entry.pc = cpu->integer.pc;
                btb_entry.prediction = 1;
                if (cpu->integer.imm < 0)
                {
                    btb_entry.target_address = cpu->integer.pc + cpu->integer.imm;
                }
                else
                {
                    btb_entry.target_address = cpu->integer.pc + 4;
                }
                insert_BTB_data(cpu, btb_entry);

                if (cpu->phy_regs[cpu->integer.prs1].CC_flag == TRUE)
                {
                    /* Calculate new PC, and send it to fetch unit */
                    cpu->pc = cpu->integer.pc + cpu->integer.imm;
                    /* Since we are using reverse callbacks for pipeline stages,
                     * this will prevent the new instruction from being fetched in the current cycle*/
                    cpu->fetch_from_next_cycle = TRUE;
                    /*Initilizing with value with random value 55 so that reg R0 doesn't match with inititialized value*/

                    /* Flush previous stages */
                    cpu->decode.has_insn = FALSE;
                    cpu->dispatch.has_insn = FALSE;
                    init_iq(cpu, cpu->integer.iq_index);
                    cpu->ROB_Rear = cpu->integer.rob_index;
                    for (int i = cpu->LSQ_Front; i != cpu->LSQ_Rear; i = (i + 1) % LSQ_SIZE)
                    {
                        if (cpu->lsq[i].data.branch_identifier == cpu->bts[cpu->BTS_Front].pc)
                        {
                            cpu->LSQ_Front = (cpu->lsq[i].data.lsq_index + 1);
                        }
                    }
                    for (int i = 0; i < REG_FILE_SIZE; i++)
                    {
                        cpu->mapping[i] = cpu->bts[cpu->BTB_Front].mapping[i];
                    }
                    //    bts_entry.mapping = cpu->mapping;
                    cpu->free_list_Front = cpu->bts[cpu->BTB_Front].free_list_Front;
                    cpu->free_list_Rear = cpu->bts[cpu->BTB_Front].free_list_Rear;

                    /* Make sure fetch stage is enabled to start fetching from new PC */
                    cpu->fetch.has_insn = TRUE;
                }
            }

            break;
        }

        case OPCODE_BNZ:
        {
            BTB btb_entry;
            if (cpu->integer.branch_predit == 1)
            {
                if (cpu->integer.imm < 0)
                {

                    if (cpu->phy_regs[cpu->integer.prs1].CC_flag == TRUE)
                    {
                        cpu->flushed = 1;
                        cpu->pc = cpu->integer.pc + 4;
                        /* Calculate new PC, and send it to fetch unit */
                        //      cpu->pc = cpu->integer.pc + cpu->integer.imm;

                        /* Since we are using reverse callbacks for pipeline stages,
                         * this will prevent the new instruction from being fetched in the current cycle*/
                        cpu->fetch_from_next_cycle = TRUE;
                        /*Initilizing with value with random value 55 so that reg R0 doesn't match with inititialized value*/

                        /* Flush previous stages */
                        cpu->decode.has_insn = FALSE;
                        cpu->dispatch.has_insn = FALSE;
                        init_iq(cpu, cpu->integer.iq_index);
                        cpu->ROB_Rear = cpu->integer.rob_index;
                        for (int i = cpu->LSQ_Front; i != cpu->LSQ_Rear; i = (i + 1) % LSQ_SIZE)
                        {
                            if (cpu->lsq[i].data.branch_identifier == cpu->bts[cpu->BTS_Front].pc)
                            {
                                cpu->LSQ_Front = (cpu->lsq[i].data.lsq_index + 1);
                            }
                        }
                        for (int i = 0; i < REG_FILE_SIZE; i++)
                        {
                            cpu->mapping[i] = cpu->bts[cpu->BTB_Front].mapping[i];
                        }
                        //    bts_entry.mapping = cpu->mapping;
                        cpu->free_list_Front = cpu->bts[cpu->BTB_Front].free_list_Front;
                        cpu->free_list_Rear = cpu->bts[cpu->BTB_Front].free_list_Rear;

                        /* Make sure fetch stage is enabled to start fetching from new PC */
                        cpu->fetch.has_insn = TRUE;
                    }
                }
                else
                {
                    if (cpu->phy_regs[cpu->integer.prs1].CC_flag == FALSE)
                    {
                        cpu->flushed = 1;
                        cpu->pc = cpu->integer.pc + cpu->integer.imm;
                        /* Calculate new PC, and send it to fetch unit */

                        /* Since we are using reverse callbacks for pipeline stages,
                         * this will prevent the new instruction from being fetched in the current cycle*/
                        cpu->fetch_from_next_cycle = TRUE;
                        /*Initilizing with value with random value 55 so that reg R0 doesn't match with inititialized value*/

                        /* Flush previous stages */
                        cpu->decode.has_insn = FALSE;
                        cpu->dispatch.has_insn = FALSE;
                        init_iq(cpu, cpu->integer.iq_index);
                        cpu->ROB_Rear = cpu->integer.rob_index;
                        for (int i = cpu->LSQ_Front; i != cpu->LSQ_Rear; i = (i + 1) % LSQ_SIZE)
                        {
                            if (cpu->lsq[i].data.branch_identifier == cpu->bts[cpu->BTS_Front].pc)
                            {
                                cpu->LSQ_Front = (cpu->lsq[i].data.lsq_index + 1);
                            }
                        }
                        for (int i = 0; i < REG_FILE_SIZE; i++)
                        {
                            cpu->mapping[i] = cpu->bts[cpu->BTB_Front].mapping[i];
                        }
                        //    bts_entry.mapping = cpu->mapping;
                        cpu->free_list_Front = cpu->bts[cpu->BTB_Front].free_list_Front;
                        cpu->free_list_Rear = cpu->bts[cpu->BTB_Front].free_list_Rear;

                        /* Make sure fetch stage is enabled to start fetching from new PC */
                        cpu->fetch.has_insn = TRUE;
                    }
                }
            }
            else
            {
                btb_entry.pc = cpu->integer.pc;
                btb_entry.prediction = 1;
                if (cpu->integer.imm < 0)
                {
                    btb_entry.target_address = cpu->integer.pc + cpu->integer.imm;
                }
                else
                {
                    btb_entry.target_address = cpu->integer.pc + 4;
                }
                insert_BTB_data(cpu, btb_entry);
                if (cpu->phy_regs[cpu->integer.prs1].CC_flag == FALSE)
                {
                    /* Calculate new PC, and send it to fetch unit */
                    cpu->pc = cpu->integer.pc + cpu->integer.imm;
                    // btb_entry.pc = cpu->integer.pc;
                    // btb_entry.prediction = 1;
                    // btb_entry.target_address = cpu->pc;
                    // insert_BTB_data(cpu, btb_entry);

                    /* Since we are using reverse callbacks for pipeline stages,
                     * this will prevent the new instruction from being fetched in the current cycle*/
                    cpu->fetch_from_next_cycle = TRUE;
                    /*Initilizing with value with random value 55 so that reg R0 doesn't match with inititialized value*/

                    /* Flush previous stages */
                    cpu->decode.has_insn = FALSE;
                    cpu->dispatch.has_insn = FALSE;
                    init_iq(cpu, cpu->integer.iq_index);
                    cpu->ROB_Rear = cpu->integer.rob_index;
                    for (int i = cpu->LSQ_Front; i != cpu->LSQ_Rear; i = (i + 1) % LSQ_SIZE)
                    {
                        if (cpu->lsq[i].data.branch_identifier == cpu->bts[cpu->BTS_Front].pc)
                        {
                            cpu->LSQ_Front = (cpu->lsq[i].data.lsq_index + 1);
                        }
                    }
                    for (int i = 0; i < REG_FILE_SIZE; i++)
                    {
                        cpu->mapping[i] = cpu->bts[cpu->BTB_Front].mapping[i];
                    }
                    //    bts_entry.mapping = cpu->mapping;
                    cpu->free_list_Front = cpu->bts[cpu->BTB_Front].free_list_Front;
                    cpu->free_list_Rear = cpu->bts[cpu->BTB_Front].free_list_Rear;

                    /* Make sure fetch stage is enabled to start fetching from new PC */
                    cpu->fetch.has_insn = TRUE;
                }
            }
            break;
        }
        case OPCODE_JMP:
        {
            cpu->pc = cpu->integer.imm + cpu->integer.rs1_value;
            cpu->fetch_from_next_cycle = TRUE;
            /* Flush previous stages */
            cpu->decode.has_insn = FALSE;
            cpu->dispatch.has_insn = FALSE;
            init_iq(cpu, cpu->integer.iq_index);
            cpu->ROB_Rear = cpu->integer.rob_index;

            /* Make sure fetch stage is enabled to start fetching from new PC */
            cpu->fetch.has_insn = TRUE;
        }

        case OPCODE_MOVC:
        {
            cpu->integer.result_buffer = cpu->integer.imm;
            cpu->phy_regs[cpu->integer.prd].value = cpu->integer.result_buffer;

            /* Set the zero flag based on the result buffer */
            if (cpu->integer.result_buffer == 0)
            {
                cpu->zero_flag = TRUE;
            }
            else
            {
                cpu->zero_flag = FALSE;
            }
            break;
        }
        }
        /* Forward value to next adjacent instruction*/
        if ((cpu->broadcast[0].fu_type_op == 0) && (cpu->broadcast[0].valid == 1))
        {
            if ((cpu->integer.opcode != OPCODE_STORE) && (cpu->integer.opcode != OPCODE_STR) && (cpu->integer.opcode != OPCODE_LOAD) && (cpu->integer.opcode != OPCODE_LDR) && (cpu->integer.opcode != OPCODE_NOP) && (cpu->integer.opcode != OPCODE_BZ) && (cpu->integer.opcode != OPCODE_BNZ) && (cpu->integer.opcode != OPCODE_JMP))
            {

                cpu->fwd_bus[0].tag = cpu->integer.prd;
                cpu->fwd_bus[0].value = cpu->integer.result_buffer;
                cpu->fwd_bus[0].valid = 1;
            }
            else
            {
                /*Resetting with value with random value 55 so that reg R0 doesn't match with inititialized value*/
                cpu->fwd_bus[0].valid = 0;
            }
        }
        if ((cpu->broadcast[1].fu_type_op == 0) && (cpu->broadcast[1].valid == 1))
        {
            if ((cpu->integer.opcode != OPCODE_STORE) && (cpu->integer.opcode != OPCODE_STR) && (cpu->integer.opcode != OPCODE_LOAD) && (cpu->integer.opcode != OPCODE_LDR) && (cpu->integer.opcode != OPCODE_NOP) && (cpu->integer.opcode != OPCODE_BZ) && (cpu->integer.opcode != OPCODE_BNZ) && (cpu->integer.opcode != OPCODE_JMP))
            {

                cpu->fwd_bus[1].tag = cpu->integer.prd;
                cpu->fwd_bus[1].value = cpu->integer.result_buffer;
                cpu->fwd_bus[1].valid = 1;
            }
            else
            {
                /*Resetting with value with random value 55 so that reg R0 doesn't match with inititialized value*/
                cpu->fwd_bus[1].valid = 0;
            }
        }
        if (((cpu->broadcast[0].fu_type_op == 0) && (cpu->broadcast[0].valid == 1)) || ((cpu->broadcast[1].fu_type_op == 0) && (cpu->broadcast[1].valid == 1)))
        {
            cpu->state_regs[cpu->integer.prd] = 0;
            if ((cpu->integer.opcode != OPCODE_STORE) && (cpu->integer.opcode != OPCODE_STR) && (cpu->integer.opcode != OPCODE_LOAD) && (cpu->integer.opcode != OPCODE_LDR))
            {
                free_iq(cpu, cpu->integer.iq_index);
                update_rob(cpu, cpu->integer);
            }
            else
            {
                update_data_cache_iq(cpu, cpu->integer);
                update_lsq(cpu, cpu->integer);
            }
            cpu->integer.has_insn = FALSE;
        }

        if ((ENABLE_DEBUG_MESSAGES) || (cpu->command == DISPLAY) || (cpu->command == SINGLE_STEP) || (cpu->command == SIMULATE))
        {
            print_stage_content("Integer FU", &cpu->integer);
        }
    }
}
static void
APEX_multiplier_fu(APEX_CPU *cpu)
{
    if (cpu->multiplier.has_insn)
    {

        if ((cpu->multiplier.opcode != OPCODE_STORE) && (cpu->multiplier.opcode != OPCODE_STR) && (cpu->multiplier.opcode != OPCODE_NOP) && (cpu->multiplier.opcode != OPCODE_BZ) && (cpu->multiplier.opcode != OPCODE_BNZ))
        {
            cpu->state_regs[cpu->multiplier.prd] = 1;
        }
        if (cpu->multiplier.cycle == 2)
        {
            cpu->mul_ind = 1;
            cpu->mul_tag = cpu->multiplier.prd;
        }
        if (cpu->multiplier.cycle == 3)
        {
            /* Execute logic based on instruction type */

            cpu->multiplier.result_buffer = cpu->multiplier.rs1_value * cpu->multiplier.rs2_value;
            cpu->phy_regs[cpu->multiplier.prd].value = cpu->multiplier.result_buffer;
            /* Set the zero flag based on the result buffer */
            if (cpu->multiplier.result_buffer == 0)
            {
                cpu->zero_flag = TRUE;
            }
            else
            {
                cpu->zero_flag = FALSE;
            }
            cpu->phy_regs[cpu->multiplier.prd].CC_flag = cpu->zero_flag;
            /* Copy data from execute latch to memory latch*/

            /* Forward value to next adjacent instruction*/
            //   printf("MJXX: check fwd valid:%d fu:%d tag%d", cpu->broadcast[0].valid, cpu->broadcast[0].fu_type_op, cpu->broadcast[0].tag);
            if ((cpu->broadcast[0].fu_type_op == 2) && (cpu->broadcast[0].valid == 1))
            {
                cpu->fwd_bus[0].tag = cpu->multiplier.prd;
                cpu->fwd_bus[0].value = cpu->multiplier.result_buffer;
                cpu->fwd_bus[0].valid = 1;
            }
            if ((cpu->broadcast[1].fu_type_op == 2) && (cpu->broadcast[1].valid == 1))
            {
                cpu->fwd_bus[1].tag = cpu->multiplier.prd;
                cpu->fwd_bus[1].value = cpu->multiplier.result_buffer;
                cpu->fwd_bus[1].valid = 1;
            }
            if (((cpu->broadcast[0].fu_type_op == 2) && (cpu->broadcast[0].valid == 1)) || ((cpu->broadcast[1].fu_type_op == 2) && (cpu->broadcast[1].valid == 1)))
            {

                cpu->state_regs[cpu->multiplier.prd] = 0;
                cpu->multiplier.has_insn = FALSE;
                free_iq(cpu, cpu->multiplier.iq_index);
                update_rob(cpu, cpu->multiplier);
            }
            else
            {
                cpu->mul_ind = 1;
                cpu->mul_tag = cpu->multiplier.prd;
                goto SKIP_MUL;
            }
        }
        else
        {
            cpu->multiplier.cycle++;
        }
    SKIP_MUL:;
        if ((ENABLE_DEBUG_MESSAGES) || (cpu->command == DISPLAY) || (cpu->command == SINGLE_STEP) || (cpu->command == SIMULATE))
        {
            print_stage_content("Mul FU", &cpu->multiplier);
        }
    }
}

static void
APEX_load_store_fu(APEX_CPU *cpu)
{
    if (!cpu->load_store.has_insn)
    {
        if (fetch_lsq_instruction(cpu))
        {
            cpu->load_store = cpu->lsq[cpu->LSQ_Front].data;
        }
    }
    if (cpu->load_store.has_insn)
    {
        if ((cpu->load_store.opcode == OPCODE_LOAD) || (cpu->load_store.opcode == OPCODE_LDR))
        {
            cpu->state_regs[cpu->load_store.prd] = 1;
        }
        /* Execute logic based on instruction type */
        switch (cpu->load_store.opcode)
        {
        case OPCODE_LOAD:
        {
            cpu->load_store.memory_address = cpu->load_store.rs1_value + cpu->load_store.imm;
            cpu->load_store.result_buffer = cpu->data_memory[cpu->load_store.memory_address];
            cpu->phy_regs[cpu->load_store.prd].value = cpu->load_store.result_buffer;
            break;
        }

        case OPCODE_STORE:
        {
            cpu->load_store.memory_address = cpu->load_store.rs2_value + cpu->load_store.imm;
            cpu->data_memory[cpu->load_store.memory_address] = cpu->load_store.rs1_value;
            break;
        }

        case OPCODE_LDR:
        {
            cpu->load_store.memory_address = cpu->load_store.rs1_value + cpu->load_store.rs2_value;
            cpu->load_store.result_buffer = cpu->data_memory[cpu->load_store.memory_address];
            cpu->phy_regs[cpu->load_store.prd].value = cpu->load_store.result_buffer;
            break;
        }

        case OPCODE_STR:
        {
            cpu->load_store.memory_address = cpu->load_store.rs1_value + cpu->load_store.rs2_value;
            cpu->data_memory[cpu->load_store.memory_address] = cpu->load_store.rs3_value;
            break;
        }
        }
        /* Forward value to next adjacent instruction*/

        /* Forward value to next adjacent instruction*/
        if (cpu->load_store.lsq_index == cpu->rob[cpu->ROB_Front].data.lsq_index)
        {
            if ((cpu->broadcast[0].fu_type_op == 1) && (cpu->broadcast[0].valid == 1))
            {
                if ((cpu->load_store.opcode == OPCODE_LOAD) || (cpu->load_store.opcode == OPCODE_LDR))
                {
                    cpu->fwd_bus[0].tag = cpu->load_store.prd;
                    cpu->fwd_bus[0].value = cpu->load_store.result_buffer;
                    cpu->fwd_bus[0].valid = 1;
                }
                else
                {
                    /*Resetting with value with random value 55 so that reg R0 doesn't match with inititialized value*/
                    cpu->fwd_bus[0].valid = 0;
                }
            }
            if ((cpu->broadcast[1].fu_type_op == 1) && (cpu->broadcast[1].valid == 1))
            {
                if ((cpu->load_store.opcode == OPCODE_LOAD) || (cpu->load_store.opcode == OPCODE_LDR))
                {
                    cpu->fwd_bus[1].tag = cpu->load_store.prd;
                    cpu->fwd_bus[1].value = cpu->load_store.result_buffer;
                    cpu->fwd_bus[1].valid = 1;
                }
                else
                {
                    /*Resetting with value with random value 55 so that reg R0 doesn't match with inititialized value*/
                    cpu->fwd_bus[1].valid = 0;
                }
            }

            if (((cpu->broadcast[0].fu_type_op == 1) && (cpu->broadcast[0].valid == 1)) || ((cpu->broadcast[1].fu_type_op == 1) && (cpu->broadcast[1].valid == 1)))
            {
                /* Copy data from execute latch to memory latch*/
                cpu->state_regs[cpu->load_store.prd] = 0;
                //   free_iq(cpu, cpu->load_store.iq_index);
                cpu->load_store.has_insn = FALSE;
                lsq_dequeue(cpu);
                update_rob(cpu, cpu->load_store);
            }
            else
            {
                cpu->load_ind = 1;
                cpu->load_tag = cpu->load_store.prd;
            }
        }
        else
        {
            cpu->load_ind = 0;
        }
        // }
        // else
        // {
        //     cpu->load_store.cycle++;
        // }
        if ((ENABLE_DEBUG_MESSAGES) || (cpu->command == DISPLAY) || (cpu->command == SINGLE_STEP) || (cpu->command == SIMULATE))
        {
            print_stage_content("D-Cache", &cpu->load_store);
        }
    }
    else
    {
        cpu->load_ind = 0;
    }

    //     cpu->state_regs[cpu->load_store.rd] = 1;
    // cpu->load_store.has_insn = FALSE;
}
static int
APEX_commit(APEX_CPU *cpu)
{
    int result = check_ready_rob_instruction(cpu);
    if (result == 1)
    {
        // for (int i = 0; i < REG_FILE_SIZE; i++)
        //     {
        //         cpu->regs[i] = cpu->phy_regs[cpu->mapping[i]];
        //     }
        if ((cpu->rob[cpu->ROB_Front].data.opcode != OPCODE_STORE) && (cpu->rob[cpu->ROB_Front].data.opcode != OPCODE_STR) && (cpu->rob[cpu->ROB_Front].data.opcode != OPCODE_CMP) && (cpu->rob[cpu->ROB_Front].data.opcode != OPCODE_NOP) && (cpu->rob[cpu->ROB_Front].data.opcode != OPCODE_BZ) && (cpu->rob[cpu->ROB_Front].data.opcode != OPCODE_BNZ) && (cpu->rob[cpu->ROB_Front].data.opcode != OPCODE_JMP))
        {
            // cpu->regs[cpu->rob[cpu->ROB_Front].data.rd] = cpu->phy_regs[cpu->rob[cpu->ROB_Front].data.prd].value;
            cpu->regs[cpu->rob[cpu->ROB_Front].data.rd] = cpu->rob[cpu->ROB_Front].data.result_buffer;
            free_list_enqueue(cpu, cpu->rob[cpu->ROB_Front].data.prd);
        }
        if ((ENABLE_DEBUG_MESSAGES) || (cpu->command == DISPLAY) || (cpu->command == SINGLE_STEP) || (cpu->command == SIMULATE))
        {
            print_stage_content("Commit", &cpu->rob[cpu->ROB_Front].data);
        }
        cpu->insn_completed++;

        rob_dequeue(cpu);
        bts_dequeue(cpu);
    }
    if (cpu->rob[cpu->ROB_Front].data.opcode == OPCODE_HALT)
    {
        cpu->insn_completed++;
        return TRUE;
    }
    return 0;
}

/*
 * This function creates and initializes APEX cpu.
 *
 * Note: You are free to edit this function according to your implementation
 */
APEX_CPU *
APEX_cpu_init(const char *filename)
{
    int i;
    APEX_CPU *cpu;

    if (!filename)
    {
        return NULL;
    }

    cpu = calloc(1, sizeof(APEX_CPU));

    if (!cpu)
    {
        return NULL;
    }

    /* Initialize PC, Registers and all pipeline stages */
    cpu->pc = 4000;
    memset(cpu->regs, 0, sizeof(int) * REG_FILE_SIZE);
    memset(cpu->phy_regs, 0, sizeof(int) * PHY_REG_FILE_SIZE);
    memset(cpu->data_memory, 0, sizeof(int) * DATA_MEMORY_SIZE);
    memset(cpu->instruction_queue, 0, sizeof(int) * QUEUE_SIZE);

    cpu->Front = -1;
    cpu->Rear = -1;
    cpu->single_step = ENABLE_MULTIPLE_STEP;
    cpu->free_list_Front = -1;
    cpu->free_list_Rear = -1;
    // for (int i = 8; i <= 14; i++)
    // {
    //     free_list_enqueue(cpu, i);
    // }
    for (int i = 0; i <= 14; i++)
    {
        free_list_enqueue(cpu, i);
    }
    show_free_list(cpu);
    // for (int i = 0; i < REG_FILE_SIZE; i++)
    // {
    //     cpu->mapping[i] = i;
    // }
    cpu->mapping[REG_FILE_SIZE] = 0;

    cpu->LSQ_Front = -1;
    cpu->LSQ_Rear = -1;

    cpu->ROB_Front = -1;
    cpu->ROB_Rear = -1;

    cpu->BTB_Front = -1;
    cpu->BTB_Rear = -1;
    cpu->BTS_Front = -1;
    cpu->BTS_Rear = -1;

    /* Parse input file and create code memory */
    cpu->code_memory = create_code_memory(filename, &cpu->code_memory_size);
    if (!cpu->code_memory)
    {
        free(cpu);
        return NULL;
    }

    if ((ENABLE_DEBUG_MESSAGES) || (cpu->command == INITIALIZE))
    {
        fprintf(stderr,
                "APEX_CPU: Initialized APEX CPU, loaded %d instructions\n",
                cpu->code_memory_size);
        fprintf(stderr, "APEX_CPU: PC initialized to %d\n", cpu->pc);
        fprintf(stderr, "APEX_CPU: Printing Code Memory\n");
        printf("%-9s %-9s %-9s %-9s %-9s %-9s\n", "opcode_str", "rd", "rs1", "rs2",
               "imm", "number");

        for (i = 0; i < cpu->code_memory_size; ++i)
        {
            printf("%-9s %-9d %-9d %-9d %-9d %-9d\n", cpu->code_memory[i].opcode_str,
                   cpu->code_memory[i].rd, cpu->code_memory[i].rs1,
                   cpu->code_memory[i].rs2, cpu->code_memory[i].imm, cpu->code_memory[i].number);
        }
    }

    /* To start fetch stage */
    cpu->fetch.has_insn = TRUE;
    return cpu;
}

int APEX_cpu_simulator(const char *command)
{
    if (strcmp(command, "initialize") == 0)
    {
        return INITIALIZE;
    }
    else if (strcmp(command, "simulate") == 0)
    {

        return SIMULATE;
    }
    else if (strcmp(command, "single_step") == 0)
    {

        return SINGLE_STEP;
    }
    else if (strcmp(command, "display") == 0)
    {

        return DISPLAY;
    }
    else if (strcmp(command, "show_mem") == 0)
    {

        return SHOWMEM;
    }
    // else if (strcmp(command, "nodataforwarding") == 0)
    // {

    //     return NO_DATA_FORWARD;
    // }
    else
    {
        return 0;
    }
}

/*
 * APEX CPU simulation loop
 *
 * Note: You are free to edit this function according to your implementation
 */
void APEX_cpu_run(APEX_CPU *cpu)
{
    char user_prompt_val;
    int user_prompt_cycle = 0;
    int temp_cycle = 0;

    while (TRUE)
    {
        if ((ENABLE_DEBUG_MESSAGES) || (cpu->command == DISPLAY) || (cpu->command == SINGLE_STEP) || (cpu->command == SIMULATE))
        {
            printf("--------------------------------------------\n");
            printf("Clock Cycle #: %d\n", cpu->clock);
            printf("--------------------------------------------\n");
        }

        if (APEX_commit(cpu))
        {
            /* Halt in writeback stage */
            if (cpu->command == SHOWMEM)
            {
                printf("MEM[%-2d]=%-2d \n", cpu->command_2, cpu->data_memory[cpu->command_2]);
                printf("APEX_CPU: Simulation Complete, cycles = %d instructions = %d\n", cpu->clock, cpu->insn_completed);
            }
            else
            {
                print_reg_file(cpu);
                //    print_zero_flag(cpu);
                print_memory_file(cpu);
                printf("APEX_CPU: Simulation Complete, cycles = %d instructions = %d\n", cpu->clock, cpu->insn_completed);
            }
            break;
        }

        /*    APEX_memory(cpu);*/
        APEX_load_store_fu(cpu);
        APEX_integer_fu(cpu);
        APEX_multiplier_fu(cpu);
        APEX_logical_fu(cpu);
        APEX_issue_queue(cpu);
        APEX_dispatch(cpu);
        APEX_decode(cpu);
        APEX_fetch(cpu);
        if (fetch_lsq_instruction(cpu))
        {
            cpu->load_ind = 1;
            cpu->load_tag = cpu->lsq[cpu->LSQ_Front].data.prd;
        }
        early_broadcast_tag(cpu);
        cpu->logical_ind = 0;
        cpu->int_ind = 0;
        cpu->mul_ind = 0;
        cpu->load_ind = 0;

        if (cpu->command == SINGLE_STEP)
        {
            print_reg_file(cpu);
            print_memory_file(cpu);
        }

        if (ENABLE_DEBUG_MESSAGES || (cpu->command == DISPLAY))
        {
            // print_reg_file(cpu);
            // print_phy_reg_file(cpu);
            // print_rename_table(cpu);
            //     print_zero_flag(cpu);
            // print_memory_file(cpu);
            // show_free_list(cpu);
            show_iq(cpu);
            lsq_show(cpu);
            rob_show(cpu);
            btb_show(cpu);
            print_forward_bus(cpu);
            //  bts_show(cpu);
        }
        if ((cpu->command_2 != 0) && (cpu->command != SHOWMEM))
        {
            if (cpu->clock == cpu->command_2)
            {

                print_reg_file(cpu);
                //  print_zero_flag(cpu);
                print_memory_file(cpu);
                printf("Enter additional cycle number to run or 0 to quit:\n");
                scanf("%d", &user_prompt_cycle);

                if (user_prompt_cycle == 0)
                {
                    print_reg_file(cpu);
                    //  print_zero_flag(cpu);
                    print_memory_file(cpu);
                    printf("APEX_CPU: Simulation Stopped, cycles = %d instructions = %d\n", cpu->clock, cpu->insn_completed);
                    break;
                }
                temp_cycle = cpu->command_2;
                temp_cycle = temp_cycle + user_prompt_cycle;
                cpu->command_2 = temp_cycle;
            }
        }
        if (cpu->single_step)
        {
            if (cpu->forward == DATA_FORWARD)
            {
                //     print_zero_flag(cpu);
            }
            printf("Press any key to advance CPU Clock or <exit> to quit:\n");
            scanf("%c", &user_prompt_val);

            if ((user_prompt_val == 'E') || (user_prompt_val == 'e'))
            {
                print_reg_file(cpu);
                //  print_zero_flag(cpu);
                print_memory_file(cpu);
                printf("APEX_CPU: Simulation Stopped, cycles = %d instructions = %d\n", cpu->clock, cpu->insn_completed);
                break;
            }
        }
        update_iq_counter(cpu);

        cpu->clock++;
    }
}

/*
 * This function deallocates APEX CPU.
 *
 * Note: You are free to edit this function according to your implementation
 */
void APEX_cpu_stop(APEX_CPU *cpu)
{
    free(cpu->code_memory);
    free(cpu);
}