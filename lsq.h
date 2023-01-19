static int lsq_enqueue(APEX_CPU *cpu, CPU_Stage data)
{
    if ((cpu->LSQ_Front == 0 && cpu->LSQ_Rear == LSQ_SIZE - 1) || (cpu->LSQ_Front == cpu->LSQ_Rear + 1))
    {
        //      printf("Overflow \n");
        return 99;
    }
    else
    {
        if (cpu->LSQ_Front == -1)
            cpu->LSQ_Front = 0;
        cpu->LSQ_Rear = (cpu->LSQ_Rear + 1) % LSQ_SIZE;
        cpu->lsq[cpu->LSQ_Rear].LSQ_entry = 1;
        if (cpu->state_regs[data.prs1] == 1)
        {
            cpu->lsq[cpu->LSQ_Rear].scr1_valid = 0;
        }
        else
        {
            cpu->lsq[cpu->LSQ_Rear].scr1_valid = 1;
        }
        if (data.opcode != OPCODE_LOAD)
        {
            if (cpu->state_regs[data.prs2] == 1)
            {
                cpu->lsq[cpu->LSQ_Rear].scr2_valid = 0;
            }
            else
            {
                cpu->lsq[cpu->LSQ_Rear].scr2_valid = 1;
            }
        }
        else
        {
            cpu->lsq[cpu->LSQ_Rear].scr2_valid = 1;
        }
        if (data.opcode == OPCODE_STR)
        {
            if (cpu->state_regs[data.prs3] == 1)
            {
                cpu->lsq[cpu->LSQ_Rear].scr3_valid = 0;
            }
            else
            {
                cpu->lsq[cpu->LSQ_Rear].scr3_valid = 1;
            }
        }
        else
        {
            cpu->lsq[cpu->LSQ_Rear].scr3_valid = 1;
        }
        if ((data.opcode == OPCODE_LOAD) || (data.opcode == OPCODE_LDR))
        {
            cpu->lsq[cpu->LSQ_Rear].LOS = 1;
        }
        else
        {
            cpu->lsq[cpu->LSQ_Rear].LOS = 0;
        }
        //       cpu->lsq[cpu->LSQ_Rear].valid_bit_mem_address = 0;
        cpu->lsq[cpu->LSQ_Rear].data = data;
        //   cpu->lsq[cpu->LSQ_Rear].data.lsq_index = cpu->LSQ_Rear;

        cpu->lsq[cpu->LSQ_Rear].done_bit = 0;
        cpu->lsq[cpu->LSQ_Rear].ROB_index = 0;
        return cpu->LSQ_Rear;
    }
    return 99;
}

static void lsq_dequeue(APEX_CPU *cpu)
{
    if (cpu->LSQ_Front == -1)
    {
        //       printf("Underflow \n");
        return;
    }
    else
    {
        if (ENABLE_DEBUG_MESSAGES)
        {
            //        printf("Element deleted from the Queue: %d\n", cpu->lsq[cpu->LSQ_Front].data.number);
        }
        if (cpu->LSQ_Front == cpu->LSQ_Rear)
        {
            cpu->LSQ_Front = -1;
            cpu->LSQ_Rear = -1;
        }
        else
        {
            cpu->LSQ_Front = (cpu->LSQ_Front + 1) % LSQ_SIZE;
        }
    }
}

static void lsq_show(APEX_CPU *cpu)
{
    printf("----------\n%s\n----------\n", "LSQ entry:");
    if (cpu->LSQ_Front == -1)
    {
    }
    //   printf("Empty Queue \n");
    else
    {
        //  printf("Queue: \n");
        for (int i = cpu->LSQ_Front; i != cpu->LSQ_Rear; i = (i + 1) % LSQ_SIZE)
            printf("%d ", cpu->lsq[i].data.number);
        printf("%d ", cpu->lsq[cpu->LSQ_Rear].data.number);

        printf("\n");
    }
}

static void update_lsq(APEX_CPU *cpu, CPU_Stage data)
{
    if (cpu->LSQ_Front == -1)
    {
        //   printf("Empty Queue \n");
    }
    else
    {
        //    printf("Queue: \n");
        for (int i = cpu->LSQ_Front; i != cpu->LSQ_Rear; i = (i + 1) % LSQ_SIZE)
        {
            if (i == data.lsq_index)
            {
                //  cpu->lsq[data.lsq_index].valid_bit_mem_address = 1;
                cpu->lsq[data.lsq_index].data = data;
            }
        }
        if (cpu->LSQ_Rear == data.lsq_index)
        {
            //    cpu->lsq[data.lsq_index].valid_bit_mem_address = 1;
            cpu->lsq[data.lsq_index].data = data;
        }
    }
}
static int fetch_lsq_instruction(APEX_CPU *cpu)
{
    if ((cpu->lsq[cpu->LSQ_Front].LSQ_entry == 1) && (cpu->lsq[cpu->LSQ_Front].scr1_valid == 1) && (cpu->lsq[cpu->LSQ_Front].scr2_valid == 1) && (cpu->lsq[cpu->LSQ_Front].scr3_valid == 1))
    {
        return 1;
    }
    return 0;
}

static void
update_lsq_entries(APEX_CPU *cpu)
{
    // if (cpu->fwd_bus[0].valid == 1)
    // {
    for (int i = cpu->LSQ_Front; i != cpu->LSQ_Rear; i = (i + 1) % LSQ_SIZE)
    {
        if ((cpu->fwd_bus[1].tag == cpu->lsq[i].data.prs1) && (cpu->lsq[i].scr1_valid == 0))
        {
            //     printf(" update iq rs1 %d ", cpu->lsq[i].data.number);
            // cpu->lsq[i].scr1_valid = 1;
            cpu->lsq[i].data.rs1_value = cpu->fwd_bus[1].value;
        }
        if ((cpu->fwd_bus[1].tag == cpu->lsq[i].data.prs2) && (cpu->lsq[i].scr2_valid == 0))
        {
            //     printf("update iq rs2 %d ", cpu->lsq[i].data.number);
            // cpu->lsq[i].scr2_valid = 1;
            cpu->lsq[i].data.rs2_value = cpu->fwd_bus[1].value;
        }

        if ((cpu->fwd_bus[1].tag == cpu->lsq[i].data.prs3) && (cpu->lsq[i].scr3_valid == 0))
        {
            //    printf(" update iq rs1 %d ", cpu->lsq[i].data.number);
            //   cpu->lsq[i].scr3_valid = 1;
            cpu->lsq[i].data.rs3_value = cpu->fwd_bus[1].value;
        }

        if ((cpu->fwd_bus[0].tag == cpu->lsq[i].data.prs1) && (cpu->lsq[i].scr1_valid == 0))
        {
            //     printf(" update iq rs1 %d ", cpu->lsq[i].data.number);
            //     cpu->lsq[i].scr1_valid = 1;
            cpu->lsq[i].data.rs1_value = cpu->fwd_bus[0].value;
        }
        if ((cpu->fwd_bus[0].tag == cpu->lsq[i].data.prs2) && (cpu->lsq[i].scr2_valid == 0))
        {
            //     printf("update iq rs2 %d ", cpu->lsq[i].data.number);
            //     cpu->lsq[i].scr2_valid = 1;
            cpu->lsq[i].data.rs2_value = cpu->fwd_bus[0].value;
        }

        if ((cpu->fwd_bus[0].tag == cpu->lsq[i].data.prs3) && (cpu->lsq[i].scr3_valid == 0))
        {
            //    printf(" update iq rs1 %d ", cpu->lsq[i].data.number);
            //     cpu->lsq[i].scr3_valid = 1;
            cpu->lsq[i].data.rs3_value = cpu->fwd_bus[0].value;
        }

        if ((cpu->fwd_bus[0].tag == cpu->lsq[i].data.prs1) || (cpu->fwd_bus[1].tag == cpu->lsq[i].data.prs1))
        {
            cpu->lsq[i].scr1_valid = 1;
        }
        if ((cpu->fwd_bus[0].tag == cpu->lsq[i].data.prs2) || (cpu->fwd_bus[1].tag == cpu->lsq[i].data.prs2))
        {
            cpu->lsq[i].scr2_valid = 1;
        }
        if ((cpu->fwd_bus[0].tag == cpu->lsq[i].data.prs3) || (cpu->fwd_bus[1].tag == cpu->lsq[i].data.prs3))
        {
            cpu->lsq[i].scr3_valid = 1;
        }
    }
    if ((cpu->fwd_bus[1].tag == cpu->lsq[cpu->LSQ_Rear].data.prs1) && (cpu->lsq[cpu->LSQ_Rear].scr1_valid == 0))
    {
        //     printf(" update iq rs1 %d ", cpu->lsq[cpu->LSQ_Rear].data.number);
        // cpu->lsq[cpu->LSQ_Rear].scr1_valid = 1;
        cpu->lsq[cpu->LSQ_Rear].data.rs1_value = cpu->fwd_bus[1].value;
    }
    if ((cpu->fwd_bus[1].tag == cpu->lsq[cpu->LSQ_Rear].data.prs2) && (cpu->lsq[cpu->LSQ_Rear].scr2_valid == 0))
    {
        //     printf("update iq rs2 %d ", cpu->lsq[cpu->LSQ_Rear].data.number);
        // cpu->lsq[cpu->LSQ_Rear].scr2_valid = 1;
        cpu->lsq[cpu->LSQ_Rear].data.rs2_value = cpu->fwd_bus[1].value;
    }

    if ((cpu->fwd_bus[1].tag == cpu->lsq[cpu->LSQ_Rear].data.prs3) && (cpu->lsq[cpu->LSQ_Rear].scr3_valid == 0))
    {
        //    printf(" update iq rs1 %d ", cpu->lsq[cpu->LSQ_Rear].data.number);
        //   cpu->lsq[cpu->LSQ_Rear].scr3_valid = 1;
        cpu->lsq[cpu->LSQ_Rear].data.rs3_value = cpu->fwd_bus[1].value;
    }

    if ((cpu->fwd_bus[0].tag == cpu->lsq[cpu->LSQ_Rear].data.prs1) && (cpu->lsq[cpu->LSQ_Rear].scr1_valid == 0))
    {
        //     printf(" update iq rs1 %d ", cpu->lsq[cpu->LSQ_Rear].data.number);
        //     cpu->lsq[cpu->LSQ_Rear].scr1_valid = 1;
        cpu->lsq[cpu->LSQ_Rear].data.rs1_value = cpu->fwd_bus[0].value;
    }
    if ((cpu->fwd_bus[0].tag == cpu->lsq[cpu->LSQ_Rear].data.prs2) && (cpu->lsq[cpu->LSQ_Rear].scr2_valid == 0))
    {
        //     printf("update iq rs2 %d ", cpu->lsq[cpu->LSQ_Rear].data.number);
        //     cpu->lsq[cpu->LSQ_Rear].scr2_valid = 1;
        cpu->lsq[cpu->LSQ_Rear].data.rs2_value = cpu->fwd_bus[0].value;
    }

    if ((cpu->fwd_bus[0].tag == cpu->lsq[cpu->LSQ_Rear].data.prs3) && (cpu->lsq[cpu->LSQ_Rear].scr3_valid == 0))
    {
        //    printf(" update iq rs1 %d ", cpu->lsq[cpu->LSQ_Rear].data.number);
        //     cpu->lsq[cpu->LSQ_Rear].scr3_valid = 1;
        cpu->lsq[cpu->LSQ_Rear].data.rs3_value = cpu->fwd_bus[0].value;
    }

    if ((cpu->fwd_bus[0].tag == cpu->lsq[cpu->LSQ_Rear].data.prs1) || (cpu->fwd_bus[1].tag == cpu->lsq[cpu->LSQ_Rear].data.prs1))
    {
        cpu->lsq[cpu->LSQ_Rear].scr1_valid = 1;
    }
    if ((cpu->fwd_bus[0].tag == cpu->lsq[cpu->LSQ_Rear].data.prs2) || (cpu->fwd_bus[1].tag == cpu->lsq[cpu->LSQ_Rear].data.prs2))
    {
        cpu->lsq[cpu->LSQ_Rear].scr2_valid = 1;
    }
    if ((cpu->fwd_bus[0].tag == cpu->lsq[cpu->LSQ_Rear].data.prs3) || (cpu->fwd_bus[1].tag == cpu->lsq[cpu->LSQ_Rear].data.prs3))
    {
        cpu->lsq[cpu->LSQ_Rear].scr3_valid = 1;
    }

    // }
}
