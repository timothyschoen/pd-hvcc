
#include "HvHeavy.h"

#if defined(_LANGUAGE_C_PLUS_PLUS) || defined(__cplusplus)
extern "C" {
#endif

typedef HeavyContextInterface* (*t_create)(double);

void init_interface();
// Interface from pd to worker
void open_window(void* external);

void close_window();

void dequeue_messages();

// Interface from worker to pd
void hvcc_save_state(void* x, const char* content);

void load_state(void* obj, const char* content);

void hvcc_load(void* x, t_create createFunc);

#if defined(_LANGUAGE_C_PLUS_PLUS) || defined(__cplusplus)
}
#endif
