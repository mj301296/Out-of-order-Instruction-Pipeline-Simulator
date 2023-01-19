static int check_BTB_entry(APEX_CPU *cpu, int pc)
{
    for (int i = cpu->BTB_Front; i != cpu->BTB_Rear; i = (i + 1) % BTB_SIZE)
    {
        if (pc == cpu->btb[i].pc)
        {
            cpu->fetch.branch_predit = 1;
            return cpu->btb[i].target_address;
        }
    }

    if (pc == cpu->btb[cpu->BTB_Rear].pc)
    {
        cpu->fetch.branch_predit = 1;
        return cpu->btb[cpu->BTB_Rear].target_address;
    }

    return pc + 4;
}

static void insert_BTB_data(APEX_CPU *cpu, BTB data)
{
    if ((cpu->BTB_Front == 0 && cpu->BTB_Rear == BTB_SIZE - 1) || (cpu->BTB_Front == cpu->BTB_Rear + 1))
    {
        cpu->BTB_Front = (cpu->BTB_Front + 1) % BTB_SIZE;
        cpu->BTB_Rear = (cpu->BTB_Rear + 1) % BTB_SIZE;
        cpu->btb[cpu->BTB_Rear] = data;
    }
    else
    {
        if (cpu->BTB_Front == -1)
            cpu->BTB_Front = 0;
        cpu->BTB_Rear = (cpu->BTB_Rear + 1) % BTB_SIZE;
        cpu->btb[cpu->BTB_Rear] = data;
    }
}

static void btb_show(APEX_CPU *cpu)
{
    printf("----------\n%s\n----------\n", "BTB entry:");
    if (cpu->BTB_Front == -1)
    {
        //      printf("Empty Queue \n");
    }
    else
    {
        //   printf("Queue: \n");
        for (int i = cpu->BTB_Front; i != cpu->BTB_Rear; i = (i + 1) % BTB_SIZE)
            printf("%d ", cpu->btb[i].pc);
        printf("%d ", cpu->btb[cpu->BTB_Rear].pc);

        printf("\n");
    }
}