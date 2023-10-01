#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_FILENAME_LENGTH 128
#define MAX_MEM_BUF_SIZE 10000

typedef struct {
    char debug_mode;
    char file_name[MAX_FILENAME_LENGTH];
    int unit_size;
    unsigned char mem_buf[MAX_MEM_BUF_SIZE];
    size_t mem_count;
    int display_mode;

} state;

void toggle_debug_mode(state* s) {
    s->debug_mode = !s->debug_mode;
    if (s->debug_mode) {
        fprintf(stderr, "Debug flag now on\n");
    } else {
        fprintf(stderr, "Debug flag now off\n");
    }
}

void set_file_name(state* s) {
    printf("Enter file name: ");
    fgets(s->file_name, MAX_FILENAME_LENGTH, stdin);
    s->file_name[strcspn(s->file_name, "\n")] = '\0'; // Remove trailing newline
    if (s->debug_mode) {
        fprintf(stderr, "Debug: file name set to '%s'\n", s->file_name);
    }
}

void set_unit_size(state* s) {
    int size;
    printf("Enter unit size (1, 2, or 4): ");
    if (scanf("%d", &size) != 1) {
        printf("Invalid input. Please enter a number.\n");
        while (getchar() != '\n'); // Clear input buffer
        return;
    }
    getchar(); // Consume newline character
    if (size == 1 || size == 2 || size == 4) {
        s->unit_size = size;
        if (s->debug_mode) {
            fprintf(stderr, "Debug: set size to %d\n", s->unit_size);
        }
    } else {
        printf("Invalid unit size. Please enter 1, 2, or 4.\n");
    }
}

void load_into_memory(state* s) {
    if (strcmp(s->file_name, "") == 0) {
        printf("File name not set. Please set the file name first.\n");
        return;
    }

    FILE* file = fopen(s->file_name, "rb");
    if (!file) {
        printf("Failed to open file: %s\n", s->file_name);
        return;
    }

    char source_address[10];
    int decimal_length;

    printf("Enter target_location (hexadecimal): ");
    fgets(source_address, sizeof(source_address), stdin);
    source_address[strcspn(source_address, "\n")] = '\0'; // Remove trailing newline

    printf("Enter length (decimal): ");
    if (scanf("%d", &decimal_length) != 1) {
        printf("Invalid input. Please enter a number.\n");
        while (getchar() != '\n'); // Clear input buffer
        fclose(file);
        return;
    }
    getchar(); // Consume newline character

    if (s->debug_mode) {
        fprintf(stderr, "Debug: file_name='%s', target_location='%s', length=%d\n",
                s->file_name, source_address, decimal_length);
    }

    unsigned long target_location;
    sscanf(source_address, "%lx", &target_location);

    fseek(file, target_location, SEEK_SET);
    size_t bytes_read = fread(s->mem_buf, s->unit_size, decimal_length, file);
    s->mem_count = bytes_read * s->unit_size;

    fclose(file);
    printf("Loaded %zu bytes into memory.\n", s->mem_count);
}

void toggle_display_mode(state* s) {
    s->display_mode = !s->display_mode;
    if (s->display_mode) {
        printf("Display flag now on, hexadecimal representation\n");
    } else {
        printf("Display flag now off, decimal representation\n");
    }
}

void memory_display(state* s) {
    unsigned long target_location;
    int decimal_units;

    printf("Enter target_location in hexadecimal: ");
    if (scanf("%lx", &target_location) != 1) {
        printf("Invalid input. Please enter a hexadecimal number.\n");
        while (getchar() != '\n'); // Clear input buffer
        return;
    }
    getchar(); // Consume newline character

    printf("Enter number of units (decimal): ");
    if (scanf("%d", &decimal_units) != 1) {
        printf("Invalid input. Please enter a number.\n");
        while (getchar() != '\n'); // Clear input buffer
        return;
    }
    getchar(); // Consume newline character

    if (target_location == 0) {
        target_location = (unsigned long)s->mem_buf;
    }

    if (s->display_mode) {
        unsigned char* addr = (unsigned char*)target_location;
        for (int i = 0; i < decimal_units; i++) {
            for (int j = s->unit_size -1 ; j >= 0; j--) {
                printf("%02hhx", addr[j]);
            }
            printf("\n");
            addr += s->unit_size;
        }
    } else {
        unsigned int* addr = (unsigned int*)target_location;
        for (int i = 0; i < decimal_units; i++) {
            for (int j =  0 ; j < s->unit_size; j++) {
                printf("%d", addr[j]);
            }
            printf("\n");
            addr += s->unit_size;
        }
    }
}

void save_into_file(state* s) {
    int source_address, target_location, decimal_length;
    unsigned char *address;
    FILE *fd;

    if (strcmp(s->file_name, "") == 0) {
        fprintf(stderr, "File name is empty. Please set the file name first.\n");
        return;
    }

    printf("Enter source address in hexadecimal: ");
    if (scanf("%x", &source_address) != 1) {
        printf("Invalid input. Please enter a hexadecimal number.\n");
        return;
    }
    getchar(); // Consume newline character

    printf("Enter target location in hexadecimal: ");
    if (scanf("%x", &target_location) != 1) {
        printf("Invalid input. Please enter a hexadecimal number.\n");
        return;
    }
    getchar(); // Consume newline character

    printf("Enter length in decimal: ");
    if (scanf("%d", &decimal_length) != 1) {
        printf("Invalid input. Please enter a number.\n");
        return;
    }
    getchar(); // Consume newline character

    if (s->debug_mode) {
         fprintf(stderr, "Saving into file: file_name: %s, source_address: %x, target_location: %x, length: %d\n",s->file_name, source_address,target_location, decimal_length);
    }
    printf("target_location: %p\n", (void*)target_location);
    printf("Length: %d\n", decimal_length);
    printf("Unit size: %d\n", s->unit_size);

    if ((fd = fopen(s->file_name, "r+")) == NULL){
        perror("fopen");
        return;
    }

    address = (source_address == 0) ? s->mem_buf : (unsigned char* ) source_address;

    if (fseek(fd, target_location, SEEK_SET) == -1)
        perror("lseek");
    else if (fwrite(address, s->unit_size, decimal_length, fd))
        perror("fwrite");
    else
    {
        printf("Saved %d units into memory\n", decimal_length);
    }
    fclose(fd);
    
}

void memory_modify(state* s) {
    printf("Please enter <location> <val>: ");
    unsigned long location;
    unsigned long val;

    if (scanf("%lx %lx", &location, &val) != 2) {
        printf("Invalid input. Please enter valid hexadecimal values.\n");
        while (getchar() != '\n'); // Clear input buffer
        return;
    }
    if (s->debug_mode) {
        printf("Location: 0x%lx, Val: 0x%lx\n", location, val);
    }

    // Calculate the index within the memory buffer based on the location
    unsigned long index = location / s->unit_size;

    // Convert the value to a byte array to copy into the memory buffer
    unsigned char* byte_array = (unsigned char*)&val;

    // Copy the bytes of the value into the memory buffer at the specified index
    for (int i = 0; i < s->unit_size; i++) {
        s->mem_buf[index * s->unit_size + i] = byte_array[i];
    }

    printf("Memory modified successfully.\n");
}

void quit(state* s) {
    if (s->debug_mode) {
        fprintf(stderr, "Quitting\n");
    }
    exit(0);
}

void print_menu() {
    printf("Choose action:\n"
           "0-Toggle Debug Mode\n"
           "1-Set File Name\n"
           "2-Set Unit Size\n"
           "3-Load Into Memory\n"
           "4-Toggle Display Mode\n"
           "5-Memory Display\n"
           "6-Save Into File\n"
           "7-Memory Modify\n"
           "8-Quit\n");
}

int get_choice() {
    int choice;
    printf("Enter your choice: ");
    if (scanf("%d", &choice) != 1) {
        printf("Invalid input. Please enter a number.\n");
        while (getchar() != '\n'); // Clear input buffer
        return -1;
    }
    getchar(); // Consume newline character
    return choice;
}

int main() {
    state program_state;
    program_state.debug_mode = 0;
    strcpy(program_state.file_name, "");
    program_state.unit_size = 1; // Default unit size
    program_state.mem_count = 0;
    program_state.display_mode = 0; // Initialize display mode to decimal representation

    while (1) {
        if (program_state.debug_mode) {
            fprintf(stderr, "unit_size: %d, file_name: %s, mem_count: %zu\n",
                    program_state.unit_size, program_state.file_name, program_state.mem_count);
        }

        print_menu();

        int choice = get_choice();

        switch (choice) {
            case 0:
                toggle_debug_mode(&program_state);
                break;
            case 1:
                set_file_name(&program_state);
                break;
            case 2:
                set_unit_size(&program_state);
                break;
            case 3:
                load_into_memory(&program_state);
                break;
            case 4:
                toggle_display_mode(&program_state);
                break;
            case 5:
                memory_display(&program_state);
                break;
            case 6:
                save_into_file(&program_state);
                break;
            case 7:
                memory_modify(&program_state);
                break;
            case 8:
                quit(&program_state);
                break;
            default:
                printf("Invalid choice. Please enter a number between 0 and 8.\n");
        }

        printf("\n");
    }

    return 0;
}