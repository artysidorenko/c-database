#ifndef CURSOR_H
#define CURSOR_H

#include "common.h"

void* cursor_value(Cursor* cursor);

void cursor_advance(Cursor* cursor);

void serialize_row(Row* source, void* destination);

void deserialize_row(void* source, Row* destination);

#endif /* CURSOR_H */
