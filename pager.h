#ifndef PAGER_H
#define PAGER_H

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include "common.h"

#define PAGE_SIZE 4096

Pager* pager_open(const char* filename);

void pager_flush(Pager* pager, uint32_t page_num);

void* get_page(Pager* pager, uint32_t page_num);

#endif /* PAGER_H */
