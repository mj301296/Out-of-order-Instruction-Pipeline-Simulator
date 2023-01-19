// static void
// early_broadcast_tag(APEX_CPU *cpu)
// {

//     if (cpu->int_ind)
//     {
//         cpu->broadcast[0].fu_type_op = 0;
//         cpu->broadcast[0].tag = cpu->int_tag;
//         cpu->broadcast[0].valid = 1;
//     }
//     else if (cpu->logical_ind)
//     {
//         if (cpu->mul_ind)
//         {
//             cpu->broadcast[0].fu_type_op = 2;
//             cpu->broadcast[0].tag = cpu->mul_tag;
//             cpu->broadcast[0].valid = 1;
//         }
//         else
//         {
//             cpu->broadcast[0].fu_type_op = 3;
//             cpu->broadcast[0].tag = cpu->logical_tag;
//             cpu->broadcast[0].valid = 1;
//         }
//     }
//     else if (cpu->mul_ind)
//     {
//         cpu->broadcast[0].fu_type_op = 2;
//         cpu->broadcast[0].tag = cpu->mul_tag;
//         cpu->broadcast[0].valid = 1;
//     }
// }
#include <stdio.h>
#include <stdlib.h>
int cmpfunc(const void *a, const void *b)
{
    return (*(int *)a - *(int *)b);
}
static void
foward_tag(APEX_CPU *cpu, int index)
{
    if (cpu->int_ind)
    {
        cpu->broadcast[index].fu_type_op = 0;
        cpu->broadcast[index].tag = cpu->int_tag;
        cpu->broadcast[index].valid = 1;
        cpu->int_ind = 0;
    }
    else if (cpu->load_ind)
    {
        cpu->broadcast[index].fu_type_op = 1;
        cpu->broadcast[index].tag = cpu->load_tag;
        cpu->broadcast[index].valid = 1;
        cpu->load_ind = 0;
    }
    else if (cpu->mul_ind)
    {
        cpu->broadcast[index].fu_type_op = 2;
        cpu->broadcast[index].tag = cpu->mul_tag;
        cpu->broadcast[index].valid = 1;
        cpu->mul_ind = 0;
    }
    else if (cpu->logical_ind)
    {
        cpu->broadcast[index].fu_type_op = 3;
        cpu->broadcast[index].tag = cpu->logical_tag;
        cpu->broadcast[index].valid = 1;
        cpu->logical_ind = 0;
    }
}

static void
early_broadcast_tag(APEX_CPU *cpu)
{
    int Front = 0;
    int Rear = -1;
    int reshuffle[4];

    if (cpu->int_ind)
    {
        Rear++;
        reshuffle[Rear] = 0;
    }
    if (cpu->load_ind)
    {
        Rear++;
        reshuffle[Rear] = 1;
    }

    if (cpu->mul_ind)
    {
        Rear++;
        reshuffle[Rear] = 2;
    }

    if (cpu->logical_ind)
    {
        Rear++;
        reshuffle[Rear] = 3;
    }
    // printf("MJXX: int_ind;%d load_ind;%d mul_ind;%d log_ind;%d", cpu->int_ind, cpu->load_ind, cpu->mul_ind, cpu->logical_ind);
    if (Rear != 1 && Rear > 1)
    {
        qsort(reshuffle, Rear + 1, sizeof(int), cmpfunc);
        foward_tag(cpu, Front);
        foward_tag(cpu, Front + 1);
    }
    else if (Rear == 1)
    {
        foward_tag(cpu, Front);
        foward_tag(cpu, Front + 1);
    }
    else if (Rear == 0)
    {
        foward_tag(cpu, Front);
    }
}

static void
update_iq_tag(APEX_CPU *cpu)
{
    for (int i = 0; i < IQ_SIZE; i++)
    {
        if (cpu->broadcast[0].valid == 1)
        {
            if ((cpu->broadcast[0].tag == cpu->iq[i].data.rs1) && (cpu->iq[i].scr1_valid == 0))
            {
                cpu->iq[i].scr1_valid = 1;
            }
            if ((cpu->broadcast[0].tag == cpu->iq[i].data.rs2) && (cpu->iq[i].scr2_valid == 0))
            {
                cpu->iq[i].scr2_valid = 1;
            }
            if ((cpu->broadcast[0].tag == cpu->iq[i].data.rs3) && (cpu->iq[i].scr3_valid == 0))
            {
                cpu->iq[i].scr3_valid = 1;
            }
        }
        if (cpu->broadcast[1].valid == 1)
        {
            if ((cpu->broadcast[1].tag == cpu->iq[i].data.rs1) && (cpu->iq[i].scr1_valid == 0))
            {
                cpu->iq[i].scr1_valid = 1;
            }
            if ((cpu->broadcast[1].tag == cpu->iq[i].data.rs2) && (cpu->iq[i].scr2_valid == 0))
            {
                cpu->iq[i].scr2_valid = 1;
            }
            if ((cpu->broadcast[1].tag == cpu->iq[i].data.rs3) && (cpu->iq[i].scr3_valid == 0))
            {
                cpu->iq[i].scr3_valid = 1;
            }
        }
    }
}
