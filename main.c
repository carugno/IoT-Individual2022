//Libraries
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "main.h"
#include "xtimer.h"
#include "thread.h"
#include "msg.h"
#include "net/emcute.h"
#include "net/ipv6/addr.h"
#include "dht_params.h"
#include "dht.h"
#include "fmt.h"
#include "periph/gpio.h"
//Defines
#ifndef GOAL_TEMP
#define GOAL_TEMP 30
#endif
#ifndef GOAL_HUM
#define GOAL_HUM 40
#endif
#ifndef EMCUTE_ID
#define EMCUTE_ID ("nucleo-board")
#endif
#define EMCUTE_PRIO (THREAD_PRIORITY_MAIN - 1)
#define NUMOFSUBS (16U)
#define TOPIC_MAXLEN (64U)
#define SERVER_ADDR ("2000:2::1")
#define SERVER_PORT 1885
#define IPV6_PREFIX_LEN (64U)
#define DEFAULT_INTERFACE ("4")
#define DEVICE_IP_ADDR ("2000:2::2")
#define MQTT_TOPIC_OUT "out"
#define MQTT_QoS (EMCUTE_QOS_0)
#define DHT_PARAMS_PIN GPIO_PIN(PORT_A,10)

//Utilities
static char stack[THREAD_STACKSIZE_DEFAULT];
static char temperature_stack[THREAD_STACKSIZE_DEFAULT];
static int16_t goal_temp = GOAL_TEMP;
static int16_t goal_hum = GOAL_HUM;
static dht_t dev;
gpio_t yellow,red,green;
gpio_t pin_relay;

//Function for the emcute thread
static void *emcute_thread(void *arg)
{
    (void)arg;
    emcute_run(CONFIG_EMCUTE_DEFAULT_PORT, EMCUTE_ID);
    return NULL;
}

//Function that setup the address on the board
static int address_setup(char *name, char *ip_address){

    netif_t *iface = netif_get_by_name(name);
    ipv6_addr_t ip_addr;
    uint16_t flag = GNRC_NETIF_IPV6_ADDRS_FLAGS_STATE_VALID | (IPV6_PREFIX_LEN << 8U);
    if(netif_set_opt(iface, NETOPT_IPV6_ADDR, flag, &ip_addr, sizeof(ip_addr)) < 0){
            puts("Error with ipv6 address");
            return 1;
        }
    printf("Success! Added %s with prefix %d to interface %s\n", ip_address, IPV6_PREFIX_LEN, name);
    return 0;
}

//Function to connect to the Broker
static int connect_broker(void){
    sock_udp_ep_t gw = { .family = AF_INET6, .port = SERVER_PORT };
    char *topic = NULL;
    char *message = NULL;
    size_t len = 0;   
    //Parsing IPv6 Address from String
    if (ipv6_addr_from_str((ipv6_addr_t *)&gw.addr.ipv6, SERVER_ADDR) == NULL) {
        printf("Error parsing IPv6 address\n");
        return 1;
    }
    //Connecting to broker
    if (emcute_con(&gw, true, topic, message, len, 0) != EMCUTE_OK) {
        printf("Error connecting to [%s]:%i\n", SERVER_ADDR, (int)gw.port);
        return 1;
    }
    printf("Connected to gateway at [%s]:%i\n", SERVER_ADDR, (int)gw.port);
    return 0;
}

//Publish on the broker
static int publish(char *t, char *message){
    emcute_topic_t topic;
    topic.name = t;
    if(emcute_reg(&topic) != EMCUTE_OK){ 
        printf("cannot find topic:%s", topic.name);
        return 1;
    }
    if(emcute_pub(&topic, message, strlen(message), MQTT_QoS) != EMCUTE_OK){ 
        printf("cannot publish data\n");
        return 1;
    }
    return 0;

}


//Sensors and Actuators Initializer
static int sensor_init(void){
    pin_relay = GPIO_PIN(PORT_B, 5);
    if (gpio_init(pin_relay, GPIO_OUT)) {
        printf("Error to initialize GPIO_PIN(%d %d)\n", PORT_B, 5);
        return -1;
    }
    gpio_set(pin_relay);
    puts("Pump Ready"); 

    if (dht_init(&dev, &dht_params) == DHT_OK){
        printf("DHT22 sensor connected\n");
    }
    else{
        printf("Failed to connect to DHT sensor\n");
        return -1;
    }

    yellow = GPIO_PIN(PORT_B, 3);
    if (gpio_init(yellow, GPIO_OUT) == -1) {
        printf("Error to initialize GPIO_PIN(%d %d)\n", PORT_B, 3);
        return -1;
    }

    green = GPIO_PIN(PORT_A, 6);
    if (gpio_init(green, GPIO_OUT) == -1) {
        printf("Error to initialize GPIO_PIN(%d %d)\n", PORT_A, 6);
        return -1;
    }

    red = GPIO_PIN(PORT_A, 9);
    if (gpio_init(red, GPIO_OUT) == -1) {
        printf("Error to initialize GPIO_PIN(%d %d)\n", PORT_A, 9);
        return -1;
    }
    puts("Traffic Light Initialized");      

    return 0;
    
}


void *humidity_temp_logic(void *arg){
    (void)arg;
    while(1){
        int16_t temp, hum;
        dht_read(&dev, &temp, &hum); //reading values from sensor
        printf("Message to send: T%d, H%d\n",temp, hum);
        if(temp > goal_temp || hum > goal_hum){
            printf("Turn on water pump\n");
            gpio_set(yellow);
            gpio_clear(pin_relay);
            xtimer_sleep(2);
            gpio_clear(yellow);
            printf("Turn-off water pump\n");
            gpio_set(pin_relay);
            gpio_set(red);
            xtimer_sleep(2);
            gpio_clear(red);
        }
        else if(temp <= goal_temp || hum <= goal_hum)
            printf("Temperature and Humidity OK\n");
        gpio_set(green); 
        char message[9];    
        sprintf(message,"t%dh%d", temp, hum);
        publish(MQTT_TOPIC_OUT, message);
        xtimer_sleep(15);
        gpio_clear(green);
    }
    return NULL;
}

int main(void){
    printf("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
    puts("Starting Up\n");
    //Starting Emcute Thread
    thread_create(stack, sizeof(stack), EMCUTE_PRIO, 0,
                  emcute_thread, NULL, "emcute");
    printf("Emcute Thread Started\n");
    // Adding GUA to the interface 
    if(address_setup(DEFAULT_INTERFACE, DEVICE_IP_ADDR)){
        printf("Impossible to set up the interface\n");
        return 1;
    }
    //Connecting to broker
    if(connect_broker()){
        printf("Impossible to Connect Correctly with the Broker\n");
        return 1;
    }
    //Initializing Sensors
    if(sensor_init()){
        printf("Failed to Initialize Sensors\n");
        return 1;
    }
    //Starting Thread that handles temperature and humidity measurement
    thread_create(temperature_stack, 
                sizeof(temperature_stack),
                THREAD_PRIORITY_MAIN - 1, 0,
                humidity_temp_logic, NULL, "humidity_temp_logic");
    puts("DHT Thread Started");
    return 0;
}
