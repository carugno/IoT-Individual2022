#ifndef MAIN_FUNCTIONS
#define MAIN_FUNCTIONS
#endif

static void *emcute_thread(void *arg);
static int address_setup(char *name, char *ip_address);
static int connect_broker(void);
static int publish(char *t, char *message);
static int sensor_init(void);
void *humidity_temp_logic(void *arg);
