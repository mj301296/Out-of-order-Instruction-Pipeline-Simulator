static void free_list_enqueue(APEX_CPU *cpu, int insert_item)
{
    if (cpu->free_list_Rear == QUEUE_SIZE - 1)
    {
    }
    //  printf("Overflow \n");
    else
    {
        if (cpu->free_list_Front == -1)
        {
            cpu->free_list_Front = 0;
        }
        cpu->free_list_Rear = cpu->free_list_Rear + 1;
        cpu->free_list[cpu->free_list_Rear] = insert_item;
    }
}

static void free_list_dequeue(APEX_CPU *cpu)
{
    if (cpu->free_list_Front == -1 || cpu->free_list_Front > cpu->free_list_Rear)
    {
        //  printf("Underflow \n");
        return;
    }
    else
    {
        // if (ENABLE_DEBUG_MESSAGES)
        // {
        //     printf("Element deleted from the Queue: %d\n", cpu->free_list[cpu->free_list_Front]);
        // }
        cpu->free_list_Front = cpu->free_list_Front + 1;
    }
}

static void show_free_list(APEX_CPU *cpu)
{

    if (cpu->free_list_Front == -1)
    {
    }
    //     printf("Empty Queue \n");
    else
    {
        //  printf("Queue: \n");
        for (int i = cpu->free_list_Front; i <= cpu->free_list_Rear; i++)
            printf("%d ", cpu->free_list[i]);
        printf("\n");
    }
}
