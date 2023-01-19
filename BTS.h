//

static void insert_BTS_data(APEX_CPU *cpu, BTS data)
{
    if ((cpu->BTS_Front == 0 && cpu->BTS_Rear == BTS_SIZE - 1) || (cpu->BTS_Front == cpu->BTS_Rear + 1))
    {
        //   printf("Overflow \n");
    }
    else
    {
        if (cpu->BTS_Front == -1)
            cpu->BTS_Front = 0;
        cpu->BTS_Rear = (cpu->BTS_Rear + 1) % BTS_SIZE;
        cpu->bts[cpu->BTS_Rear] = data;
    }
}

static void bts_show(APEX_CPU *cpu)
{
    printf("----------\n%s\n----------\n", "BTS entry:");
    if (cpu->BTS_Front == -1)
    {
        //     printf("Empty Queue \n");
    }
    else
    {
        //    printf("Queue: \n");
        for (int i = cpu->BTS_Front; i != cpu->BTS_Rear; i = (i + 1) % BTS_SIZE)
            printf("%d ", cpu->bts[i].pc);
        printf("%d ", cpu->bts[cpu->BTS_Rear].pc);

        printf("\n");
    }
}

static void bts_dequeue(APEX_CPU *cpu)
{
    if (cpu->BTS_Front == -1)
    {
        //   printf("Underflow \n");
        return;
    }
    else
    {
        if (ENABLE_DEBUG_MESSAGES)
        {
            //       printf("Element deleted from the Queue: %d\n", cpu->bts[cpu->BTS_Front].pc);
        }
        if (cpu->BTS_Front == cpu->BTS_Rear)
        {
            cpu->BTS_Front = -1;
            cpu->BTS_Rear = -1;
        }
        else
        {
            cpu->BTS_Front = (cpu->BTS_Front + 1) % BTS_SIZE;
        }
    }
}