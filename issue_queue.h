static int
check_free_IQ_index(APEX_CPU *cpu)
{
    for (int i = 0; i < IQ_SIZE; i++)
    {
        if (cpu->iq[i].alloc == 0)
        {
            return i;
        }
    }
    return 99;
}
static void
update_iq_counter(APEX_CPU *cpu)
{
    for (int i = 0; i < IQ_SIZE; i++)
    {
        if (cpu->iq[i].alloc == 1)
        {
            cpu->iq[i].dispatch_counter++;
        }
    }
}

// static int
// fetch_next_instruction(APEX_CPU *cpu, int fu_type)
// {
//     int highest_counter = -1;
//     int intruction_index = 99;
//     for (int i = 0; i < IQ_SIZE; i++)
//     {
//         //   printf("next_instruction number:%d a:%d fu:%d sc1:%d sc2:%d sc3:%d\n", cpu->iq[i].data.number, cpu->iq[i].alloc, cpu->iq[i].fu_type_op, cpu->iq[i].scr1_valid, cpu->iq[i].scr2_valid, cpu->iq[i].scr3_valid);
//         if ((cpu->iq[i].alloc == 1) && (cpu->iq[i].scr1_valid == 1) && (cpu->iq[i].scr2_valid == 1) && (cpu->iq[i].scr3_valid == 1) && (cpu->iq[i].issued == 0) && (cpu->iq[i].fu_type_op == fu_type))
//         {
//             //      printf("dispatch_counter:%d\n", cpu->iq[i].dispatch_counter);
//             if (cpu->iq[i].dispatch_counter > highest_counter)
//             {
//                 highest_counter = cpu->iq[i].dispatch_counter;
//                 intruction_index = i;
//             }
//         }
//     }
//     return intruction_index;
// }

// static int
// fetch_next_instruction(APEX_CPU *cpu)
// {
//     int highest_counter = -1;
//     int intruction_index = 99;
//     for (int i = 0; i < IQ_SIZE; i++)
//     {
//         //   printf("next_instruction number:%d a:%d fu:%d sc1:%d sc2:%d sc3:%d\n", cpu->iq[i].data.number, cpu->iq[i].alloc, cpu->iq[i].fu_type_op, cpu->iq[i].scr1_valid, cpu->iq[i].scr2_valid, cpu->iq[i].scr3_valid);
//         if ((cpu->iq[i].alloc == 1) && (cpu->iq[i].scr1_valid == 1) && (cpu->iq[i].scr2_valid == 1) && (cpu->iq[i].scr3_valid == 1) && (cpu->iq[i].issued == 0))
//         {
//             //      printf("dispatch_counter:%d\n", cpu->iq[i].dispatch_counter);
//             if (cpu->iq[i].dispatch_counter > highest_counter)
//             {
//                 highest_counter = cpu->iq[i].dispatch_counter;
//                 intruction_index = i;
//             }
//         }
//     }
//     return intruction_index;
// }

static int
fetch_next_instruction(APEX_CPU *cpu, int fu_type)
{
    int highest_counter = -1;
    int intruction_index = 99;
    for (int i = 0; i < IQ_SIZE; i++)
    {
        //   printf("next_instruction number:%d a:%d fu:%d sc1:%d sc2:%d sc3:%d\n", cpu->iq[i].data.number, cpu->iq[i].alloc, cpu->iq[i].fu_type_op, cpu->iq[i].scr1_valid, cpu->iq[i].scr2_valid, cpu->iq[i].scr3_valid);
        if ((cpu->iq[i].alloc == 1) && (cpu->iq[i].scr1_valid == 1) && (cpu->iq[i].scr2_valid == 1) && (cpu->iq[i].scr3_valid == 1) && (cpu->iq[i].issued == 0) && (cpu->iq[i].fu_type_op == fu_type))
        {
            //      printf("dispatch_counter:%d\n", cpu->iq[i].dispatch_counter);
            if (cpu->iq[i].dispatch_counter > highest_counter)
            {
                highest_counter = cpu->iq[i].dispatch_counter;
                intruction_index = i;
            }
        }
    }
    return intruction_index;
}
static void
show_iq(APEX_CPU *cpu)
{
    printf("----------\n%s\n----------\n", "IQ entry:");
    for (int i = 0; i < IQ_SIZE; i++)
    {
        if (cpu->iq[i].alloc == 1)
        {
            printf("%d ", cpu->iq[i].data.number);
        }
    }
    printf("\n");
}

static void
free_iq(APEX_CPU *cpu, int index)
{
    cpu->iq[index].alloc = 0; /*free =0 allocated =1*/
    cpu->iq[index].dispatch_counter = 0;
    cpu->iq[index].fu_type_op = 0;
    /*scr 1 fields*/
    cpu->iq[index].scr1_valid = 0;
    /*scr 2 fields*/
    cpu->iq[index].scr2_valid = 0;
    cpu->iq[index].scr3_valid = 0;
    cpu->iq[index].issued = 0;
    //  int iq_index;
    // cpu->iq[index].data = 0;
}

static void init_iq(APEX_CPU *cpu, int counter)
{
    for (int i = 0; i < IQ_SIZE; i++)
    {
        if ((cpu->iq[i].alloc == 1) && (cpu->iq[i].dispatch_counter < counter))
        {
            cpu->state_regs[cpu->iq[i].data.prd] = 0;
            free_iq(cpu, i);
        }
    }
}
static void
update_data_cache_iq(APEX_CPU *cpu, CPU_Stage data)
{
    cpu->iq[data.iq_index].fu_type_op = 1;
    // cpu->iq[data.iq_index].issued = 1;
    cpu->iq[data.iq_index].data = data;
}
static void
update_iq_entries(APEX_CPU *cpu)
{
    // if (cpu->fwd_bus[0].valid == 1)
    // {
    for (int i = 0; i < IQ_SIZE; i++)
    {
        if (cpu->iq[i].alloc == 1)
        {
            // printf("forward data 0:%d t:%d val:%d \n", cpu->fwd_bus[0].valid, cpu->fwd_bus[0].tag, cpu->fwd_bus[0].value);
            // printf("forward data 1:%d t:%d val:%d \n", cpu->fwd_bus[1].valid, cpu->fwd_bus[1].tag, cpu->fwd_bus[1].value);
            // printf("forward_instruction number:%d a:%d fu:%d sc1:%d sc2:%d sc3:%d\n", cpu->iq[i].data.number, cpu->iq[i].alloc, cpu->iq[i].fu_type_op, cpu->iq[i].scr1_valid, cpu->iq[i].scr2_valid, cpu->iq[i].scr3_valid);
            if ((cpu->fwd_bus[1].tag == cpu->iq[i].data.prs1) && (cpu->iq[i].scr1_valid == 0))
            //   if ((cpu->fwd_bus[1].tag == cpu->iq[i].data.prs1))
            {
                //     printf(" update iq rs1 %d ", cpu->iq[i].data.number);
                // cpu->iq[i].scr1_valid = 1;
                cpu->iq[i].data.rs1_value = cpu->fwd_bus[1].value;
            }
            if ((cpu->fwd_bus[1].tag == cpu->iq[i].data.prs2) && (cpu->iq[i].scr2_valid == 0))
            //  if ((cpu->fwd_bus[1].tag == cpu->iq[i].data.prs2))
            {
                //     printf("update iq rs2 %d ", cpu->iq[i].data.number);
                // cpu->iq[i].scr2_valid = 1;
                cpu->iq[i].data.rs2_value = cpu->fwd_bus[1].value;
            }

            if ((cpu->fwd_bus[1].tag == cpu->iq[i].data.prs3) && (cpu->iq[i].scr3_valid == 0))
            // if ((cpu->fwd_bus[1].tag == cpu->iq[i].data.prs3))
            {
                //    printf(" update iq rs1 %d ", cpu->iq[i].data.number);
                //   cpu->iq[i].scr3_valid = 1;
                cpu->iq[i].data.rs3_value = cpu->fwd_bus[1].value;
            }

            if ((cpu->fwd_bus[0].tag == cpu->iq[i].data.prs1) && (cpu->iq[i].scr1_valid == 0))
            //   if ((cpu->fwd_bus[0].tag == cpu->iq[i].data.prs1))
            {
                //     printf(" update iq rs1 %d ", cpu->iq[i].data.number);
                //     cpu->iq[i].scr1_valid = 1;
                cpu->iq[i].data.rs1_value = cpu->fwd_bus[0].value;
            }
            if ((cpu->fwd_bus[0].tag == cpu->iq[i].data.prs2) && (cpu->iq[i].scr2_valid == 0))
            //  if ((cpu->fwd_bus[0].tag == cpu->iq[i].data.prs2))
            {
                //     printf("update iq rs2 %d ", cpu->iq[i].data.number);
                //     cpu->iq[i].scr2_valid = 1;
                cpu->iq[i].data.rs2_value = cpu->fwd_bus[0].value;
            }
            if ((cpu->fwd_bus[0].tag == cpu->iq[i].data.prs3) && (cpu->iq[i].scr3_valid == 0))
            //  if ((cpu->fwd_bus[0].tag == cpu->iq[i].data.prs3))
            {
                //    printf(" update iq rs1 %d ", cpu->iq[i].data.number);
                //     cpu->iq[i].scr3_valid = 1;
                cpu->iq[i].data.rs3_value = cpu->fwd_bus[0].value;
            }

            if ((cpu->fwd_bus[0].tag == cpu->iq[i].data.prs1) || (cpu->fwd_bus[1].tag == cpu->iq[i].data.prs1))
            {
                cpu->iq[i].scr1_valid = 1;
            }
            if ((cpu->fwd_bus[0].tag == cpu->iq[i].data.prs2) || (cpu->fwd_bus[1].tag == cpu->iq[i].data.prs2))
            {
                cpu->iq[i].scr2_valid = 1;
            }
            if ((cpu->fwd_bus[0].tag == cpu->iq[i].data.prs3) || (cpu->fwd_bus[1].tag == cpu->iq[i].data.prs3))
            {
                cpu->iq[i].scr3_valid = 1;
            }
        }
    }
}
// }