#ifndef WRITEBACK_H
#define WRITEBACK_H
#include "mipssim.h"

/* CDB Queue of rob slots and values */
/* CDB should have unlimited bandwidth
 * but the number of items that can be
 * at maximum be in it is limited by the
 * size of rob. So we keep same size */
 #define MAX_CDB_ENTRIES MAX_ROB_ENTRIES

 struct cdb_entry {
    unsigned int rob_slot; //Rob slot wanting to broadcast
    int value; //Value to broadcast
 };

 struct cdb_pipe {
    struct cdb_entry cdb_item[MAX_CDB_ENTRIES];
    unsigned int num_entries; //Keep a caount of cdb entries to broadcast
 };

 extern struct cdb_pipe cdb;

#endif

