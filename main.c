#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>

#include "input_buffer.h"

#define size_of_attribute(Struct, Attribute) sizeof(((Struct*)0)->Attribute)

const u_int32_t ID_SIZE = size_of_attribute(Row, id);
const u_int32_t NAME_SIZE = size_of_attribute(Row, name);
const u_int32_t EMAIL_SIZE = size_of_attribute(Row, email);
const u_int32_t ID_OFFSET = 0;
const u_int32_t NAME_OFFSET = ID_OFFSET + ID_SIZE;
const u_int32_t EMAIL_OFFSET = NAME_OFFSET + NAME_SIZE;
const u_int32_t ROW_SIZE = ID_SIZE + NAME_SIZE + EMAIL_SIZE;

typedef enum {
    META_COMMAND_SUCCESS,
    META_UNRECOGNIZED_COMMAND
} MetaCommandResult;

typedef enum {
    PREPARE_SUCCESS,
    PREPARE_UNRECOGNIZED_STATEMENT,
    PREPARE_SYNTAX_ERROR
} PrepareResult;

typedef enum {
    STATEMENT_INSERT, 
    STATEMENT_SELECT
} StatementType;

const u_int32_t COLUMN_NAME_LENGTH = 32;
const u_int32_t COLUMN_EMAIL_LENGTH = 255;

typedef struct {
    u_int32_t id;
    char name[COLUMN_NAME_LENGTH];
    char email[COLUMN_EMAIL_LENGTH];
} Row;

typedef struct {
    StatementType type;
    Row row_insert_data;
} Statement;

// Serialize the row
void serialize_row(Row* source, void* destination) {
    memcpy(destination + ID_OFFSET, &(source->id), ID_SIZE);
    memcpy(destination + NAME_OFFSET, &(source->name), NAME_SIZE);
    memcpy(destination + EMAIL_OFFSET, &(source->email), EMAIL_SIZE);
}

void deserialize_row(void* source, Row* destination) {
    memcpy(&(destination->id), source + ID_OFFSET, ID_SIZE);
    memcpy(&(destination->name), source + NAME_OFFSET, NAME_SIZE);
    memcpy(&(destination->email), source + EMAIL_OFFSET, EMAIL_SIZE);
}

const u_int32_t PAGES_SIZE = 4096;
const u_int32_t TABLE_MAX_PAGES = 100;
const u_int32_t ROWS_PER_PAGE = PAGES_SIZE / TABLE_MAX_PAGES;
const u_int32_t TABLE_MAX_ROWS = ROWS_PER_PAGE * TABLE_MAX_PAGES;

typedef struct {
    void* pages[TABLE_MAX_PAGES];
    u_int32_t rows_count;
} Table;

MetaCommandResult do_meta_command(InputBuffer* input_buffer) {
    if(strcmp(input_buffer->buffer, ".exit") == 0) {
        exit(EXIT_SUCCESS);
    }
    return META_UNRECOGNIZED_COMMAND;
}

PrepareResult prepare_statement(InputBuffer* input_buffer, Statement* statement) {
    if(strncmp(input_buffer->buffer, "insert", 6) == 0) {
        statement->type = STATEMENT_INSERT;
        int assigned_columns_count = sscanf(
            input_buffer->buffer,
            "insert %d %s %s",
            &(statement->row_insert_data.id),
            &(statement->row_insert_data.name),
            &(statement->row_insert_data.email)
        );
        if(assigned_columns_count < 3) {
            return PREPARE_SYNTAX_ERROR;
        }
        return PREPARE_SUCCESS;
    }

    if(strncmp(input_buffer->buffer, "select", 6) == 0) {
        statement->type = STATEMENT_SELECT;
        return PREPARE_SUCCESS;
    }

    return PREPARE_UNRECOGNIZED_STATEMENT;
}

void execute_statement(Statement* statement) {
    printf("Executing..\n");
    switch(statement->type) {
        case (STATEMENT_INSERT):
            printf("Inserting \n");
            break;
        case (STATEMENT_SELECT):
            printf("Selecting \n");
            break;
    }
}

inline void print_prompt() {
    printf("db > ");
}

void read_input(InputBuffer* input_buffer) {
    ssize_t bytes_read = getline(&(input_buffer->buffer), &(input_buffer->buffer_length), stdin);
    if(bytes_read <= 0) {
        printf("Error reading input");
        exit(EXIT_FAILURE);
    }

    input_buffer->input_length = bytes_read - 1;
    input_buffer->buffer[bytes_read - 1] = 0;
}

int main(int argc, char* argv) {
    InputBuffer* input_buffer = new_input_buffer();
    while(true) {
        print_prompt();
        read_input(input_buffer);

        if(input_buffer->input_length == 0) {
            continue;
        }

        if(input_buffer->buffer[0] == '.') {
            switch(do_meta_command(input_buffer)) {
                case (META_COMMAND_SUCCESS):
                    continue;
                case (META_UNRECOGNIZED_COMMAND):
                    printf("Unrecognized command: '%s'. Enter .exit to exit.\n", input_buffer->buffer);
                    continue;
            }
        }

        Statement statement;
        switch(prepare_statement(input_buffer, &statement)) {
            case (PREPARE_SUCCESS):
                break;
            case (PREPARE_UNRECOGNIZED_STATEMENT):
                printf("Unrecognized keyword at start of '%s'.\n", input_buffer->buffer);
                continue;
            case (PREPARE_SYNTAX_ERROR):
                printf("Syntax error");
                continue;
        }

        execute_statement(&statement);
        printf("Executed\n");
    }
    return 0;
}