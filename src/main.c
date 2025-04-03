/*
** EPITECH PROJECT, 2025
** B-MUL-100:my_radar
** File description:
** main.c
** creator : ethan.saintot@epitech.eu
*/

#include "my_radar.h"

static int print_usage(void)
{
    printf("Air traffic simulation panel\n");
    printf("USAGE\n");
    printf("   ./my_radar [OPTIONS] path_to_script\n");
    printf("   path_to_script   The path to the script file\n");
    printf("OPTIONS\n");
    printf("   -h                print the usage and quit\n");
    printf("USER INTERACTIONS\n");
    printf("   'L' key           enable/disable hitboxes and areas\n");
    printf("   'S' key           enable/disable sprites\n");
    printf("   'Q' key           quit the simulation\n");
    return 0;
}

static int is_valid_file(const char *filepath)
{
    struct stat st;
    if (stat(filepath, &st) == -1)
        return 0;
    if (!S_ISREG(st.st_mode))
        return 0;
    return 1;
}

int main(int argc, char **argv, char **env)
{
    if (argc != 2) {
        fprintf(stderr, "./my_radar: bad arguments: %d given but 1 is required\n", 
                argc - 1);
        fprintf(stderr, "retry with -h\n");
        return 84;
    }
    
    if (strcmp(argv[1], "-h") == 0)
        return print_usage();
    
    if (!is_valid_file(argv[1])) {
        fprintf(stderr, "./my_radar: %s is not a valid file\n", argv[1]);
        return 84;
    }
    
    int i = 0;
    while (env[i] != NULL) {
        if (strncmp(env[i], "DISPLAY=", 8) == 0)
            break;
        i++;
    }
    
    if (env[i] == NULL) {
        fprintf(stderr, "./my_radar: no display available\n");
        return 84;
    }
    
    return run_simulation(argv[1]);
}