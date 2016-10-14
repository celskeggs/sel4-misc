#ifndef APP_INIT_SMALL_H
#define APP_INIT_SMALL_H

seL4_CPtr small_table_alloc(int type);
void small_table_free(seL4_CPtr ptr);

#endif //APP_INIT_SMALL_H
