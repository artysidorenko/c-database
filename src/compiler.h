
#ifndef COMPILER_H
#define COMPILER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "common.h"

typedef struct {
  char* buffer;
  size_t buffer_length;
  ssize_t input_length;
} InputBuffer;

typedef enum {
  PREPARE_SUCCESS,
  PREPARE_SYNTAX_ERROR,
  PREPARE_UNRECOGNIZED_STATEMENT,
  PREPARE_STRING_TOO_LONG,
  PREPARE_NEGATIVE_ID
} PrepareResult;

typedef enum { STATEMENT_INSERT, STATEMENT_SELECT } StatementType;

typedef enum {
  META_COMMAND_SUCCESS,
  META_COMMAND_UNRECOGNIZED_COMMAND
} MetaCommandResult;

typedef struct {
  StatementType type;
  Row row_to_insert;  // only used by insert statement
} Statement;

PrepareResult prepare_insert(InputBuffer* input_buffer, Statement* statement);

PrepareResult prepare_statement(InputBuffer* input_buffer, Statement* statement);

#endif /* COMPILER_H */
