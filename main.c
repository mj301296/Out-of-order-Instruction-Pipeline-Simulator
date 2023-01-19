/*
 * main.c
 *
 * Author:
 * Copyright (c) 2020, Gaurav Kothari (gkothar1@binghamton.edu)
 * State University of New York at Binghamton
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "apex_cpu.h"

int main(int argc, char const *argv[])
{
    APEX_CPU *cpu;
    int command = 0;
    int command_2 = 0;
    int forward = 0;

    fprintf(stderr, "APEX CPU Pipeline Simulator v%0.1lf\n", VERSION);

    /* if (argc != 4)
    {
        fprintf(stderr, "APEX_Help: Usage %s <input_file>\n", argv[0]);
        exit(1);
    }*/

    // if (argc == 4)
    // {
    //     if (strcmp(argv[2], "fwd") == 0)
    //     {

    //         if (strcmp(argv[3], "y") == 0)
    //         {
    //             forward = 1;
    //             command = SINGLE_STEP;
    //         }
    //     }
    //     else
    //     {
    //         command = APEX_cpu_simulator(argv[2]);
    //         if (command == SHOWMEM)
    //         {
    //             command_2 = atoi(argv[3]);
    //         }
    //         else
    //         {
    //             command_2 = atoi(argv[3]) - 1;
    //         }
    //     }
    // }
    // if (argc == 5)
    // {
    //     if (strcmp(argv[3], "fwd") == 0)
    //     {
    //         if (strcmp(argv[4], "y") == 0)
    //         {
    //             forward = 1;
    //             command = APEX_cpu_simulator(argv[2]);
    //         }
    //         else
    //         {
    //             command = APEX_cpu_simulator(argv[2]);
    //         }
    //     }
    // }
    // if (argc == 6)
    // {
    //     if (strcmp(argv[4], "fwd") == 0)
    //     {
    //         if (strcmp(argv[5], "y") == 0)
    //         {
    //             forward = 1;
    //             command = APEX_cpu_simulator(argv[2]);
    //         }
    //         else
    //         {
    //             command = APEX_cpu_simulator(argv[2]);
    //         }
    //         if (command == SHOWMEM)
    //         {
    //             command_2 = atoi(argv[3]);
    //         }
    //         else
    //         {
    //             command_2 = atoi(argv[3]) - 1;
    //         }
    //     }
    // }

    // // if (!command)
    // // {
    // //     fprintf(stderr, "APEX_Error: Unable to find simulator command\n");
    // //     exit(1);
    // // }

    cpu = APEX_cpu_init(argv[1]);
    if (!cpu)
    {
        fprintf(stderr, "APEX_Error: Unable to initialize CPU\n");
        exit(1);
    }
    if (argc > 2 && argc < 5)
    {
        command = APEX_cpu_simulator(argv[2]);
        if (argc == 4)
        {
            command_2 = atoi(argv[3]) - 1;
        }
    }
    cpu->command = command;
    cpu->command_2 = command_2;
    cpu->forward = forward;
    if (argc == 2)
    {
        cpu->command = SINGLE_STEP;
        cpu->single_step = ENABLE_SINGLE_STEP;
    }

    //   if ((strcmp(argv[2], "initiate") == 0) || (strcmp(argv[2], "simulate") == 0) || (strcmp(argv[2], "display") == 0) || (strcmp(argv[2], "single_step") == 0) || (strcmp(argv[2], "showmem") == 0))
    if (cpu->command != INITIALIZE)
    {
        APEX_cpu_run(cpu);
        APEX_cpu_stop(cpu);
    }
    return 0;
}