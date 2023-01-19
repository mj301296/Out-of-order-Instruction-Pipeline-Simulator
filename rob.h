static int rob_enqueue(APEX_CPU *cpu, CPU_Stage data)
{
    if ((cpu->ROB_Front == 0 && cpu->ROB_Rear == ROB_SIZE - 1) || (cpu->ROB_Front == cpu->ROB_Rear + 1))

    {
        //  printf("Overflow \n");
        return 99;
    }
    else
    {
        if (cpu->ROB_Front == -1)
            cpu->ROB_Front = 0;
        cpu->ROB_Rear = (cpu->ROB_Rear + 1) % ROB_SIZE;
        cpu->rob[cpu->ROB_Rear].ROB_entry = 1;
        cpu->rob[cpu->ROB_Rear].data = data;
        //  cpu->rob[cpu->ROB_Rear].data.rob_index = cpu->ROB_Rear;
        cpu->rob[cpu->ROB_Rear].commit_ready = 0;
        return cpu->ROB_Rear;
    }
    return 99;
}

static void rob_dequeue(APEX_CPU *cpu)
{
    if (cpu->ROB_Front == -1)
    {
        //   printf("Underflow \n");
        return;
    }
    else
    {
        if (ENABLE_DEBUG_MESSAGES)
        {
            //      printf("Element deleted from the Queue: %d\n", cpu->rob[cpu->ROB_Front].data.number);
        }
        if (cpu->ROB_Front == cpu->ROB_Rear)
        {
            cpu->ROB_Front = -1;
            cpu->ROB_Rear = -1;
        }
        else
        {
            cpu->ROB_Front = (cpu->ROB_Front + 1) % ROB_SIZE;
        }
    }
}

static void rob_show(APEX_CPU *cpu)
{
    printf("----------\n%s\n----------\n", "ROB entry:");
    if (cpu->ROB_Front == -1)
    {
    }
    //  printf("Empty Queue \n");
    else
    {
        //  printf("Queue: \n");
        for (int i = cpu->ROB_Front; i != cpu->ROB_Rear; i = (i + 1) % ROB_SIZE)
            printf("%d ", cpu->rob[i].data.number);
        printf("%d ", cpu->rob[cpu->ROB_Rear].data.number);
        printf("\n");
    }
}

static void update_rob(APEX_CPU *cpu, CPU_Stage data)
{
    if (cpu->ROB_Front == -1)
    {
    }
    //   printf("Empty Queue \n");
    else
    {
        //    printf("Queue: \n");
        for (int i = cpu->ROB_Front; i != cpu->ROB_Rear; i = (i + 1) % ROB_SIZE)
        {
            if (i == data.rob_index)
            {
                cpu->rob[data.rob_index].commit_ready = 1;
                cpu->rob[data.rob_index].data = data;
            }
        }
        if (cpu->ROB_Rear == data.rob_index)
        {
            cpu->rob[data.rob_index].commit_ready = 1;
            cpu->rob[data.rob_index].data = data;
        }
    }
}

static int
check_ready_rob_instruction(APEX_CPU *cpu)
{
    if (cpu->ROB_Front == -1)
    {
        //       printf("Empty Queue \n");
        return 0;
    }
    else
    {
        if (cpu->rob[cpu->ROB_Front].commit_ready == 1)
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }
    return 0;
}

static void squash_rob(APEX_CPU *cpu, int index)
{
    if (cpu->ROB_Front == -1)
    {
    }
    //       printf("Empty Queue \n");
    else
    {
        //      printf("Queue: \n");
        for (int i = index + 1; i != cpu->ROB_Rear; i = (i + 1) % ROB_SIZE)
        {

            cpu->rob[index].data.opcode = OPCODE_NOP;
        }

        cpu->rob[cpu->ROB_Rear].data.opcode = OPCODE_NOP;
    }
}
