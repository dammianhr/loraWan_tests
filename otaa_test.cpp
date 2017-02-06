/********************************************************\
lorawan otaa test.
01/02/2017
damian nahmiyas.
\********************************************************/

#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>
 
#include <lmic.h>
#include <hal/hal.h>

/*
      "eui" : "692a41b807e9f840",
      "key" : "5ccc419a5f85614a7856d8c9ccef5193",
      "leasetime" : 86400,
      "name" : "LORA_AXON",
      "passphrase" : "LORA_AXON",
      "public" : true
*/

static const u1_t PROGMEM DEVEUI[8]={ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
void os_getDevEui (u1_t* buf) { getDevEuiFromMac(buf); }

static const u1_t PROGMEM APPEUI[8]={ 0x40, 0xf8, 0xe9, 0x07, 0xb8, 0x41, 0x2a, 0x69 };
void os_getArtEui (u1_t* buf) { memcpy_P(buf, APPEUI, 8);}

static const u1_t PROGMEM APPKEY[16] = { 0x5c, 0xcc, 0x41, 0x9a, 0x5f, 0x85, 0x61, 0x4a, 0x78, 0x56, 0xd8, 0xc9, 0xcc, 0xef, 0x51, 0x93 };
void os_getDevKey (u1_t* buf) { memcpy_P(buf, APPKEY, 16);}

static uint8_t mydata[] = "rpiLMIC";
static osjob_t sendjob;
// Schedule TX every this many seconds (might become longer due to duty)
// cycle limitations).
const unsigned TX_INTERVAL = 120;

//Flag for Ctrl-C
volatile sig_atomic_t force_exit = 0;


#define RF_LED_PIN RPI_V2_GPIO_P1_22
#define RF_CS_PIN  RPI_V2_GPIO_P1_18
#define RF_IRQ_PIN RPI_V2_GPIO_P1_16
#define RF_RST_PIN RPI_V2_GPIO_P1_15

// Pin mapping
const lmic_pinmap lmic_pins = {
    .nss  = RF_CS_PIN,
    .rxtx = LMIC_UNUSED_PIN,
    .rst  = RF_RST_PIN,
    .dio  = {LMIC_UNUSED_PIN, LMIC_UNUSED_PIN, LMIC_UNUSED_PIN},
};

#ifndef RF_LED_PIN
#define RF_LED_PIN NOT_A_PIN  
#endif

void do_send(osjob_t* j) {
    char strTime[16];
    getSystemTime(strTime , sizeof(strTime));
    printf("%s: ", strTime);

    // Check if there is not a current TX/RX job running
    if (LMIC.opmode & OP_TXRXPEND) {
        printf("OP_TXRXPEND, not sending\n");
    } else {
        digitalWrite(RF_LED_PIN, HIGH);
        // Prepare upstream data transmission at the next possible time.
        LMIC_setTxData2(1, mydata, sizeof(mydata)-1, 0);
        printf("Packet queued\n");
    }
    // Next TX is scheduled after TX_COMPLETE event.
}

void onEvent (ev_t ev) {
    char strTime[16];
    getSystemTime(strTime , sizeof(strTime));
    printf("%s: ", strTime);
 
    switch(ev) {
        case EV_SCAN_TIMEOUT:
            printf("EV_SCAN_TIMEOUT\n");
        break;
        case EV_BEACON_FOUND:
            printf("EV_BEACON_FOUND\n");
        break;
        case EV_BEACON_MISSED:
            printf("EV_BEACON_MISSED\n");
        break;
        case EV_BEACON_TRACKED:
            printf("EV_BEACON_TRACKED\n");
        break;
        case EV_JOINING:
            printf("EV_JOINING\n");
        break;
        case EV_JOINED:
            printf("EV_JOINED\n");
            digitalWrite(RF_LED_PIN, LOW);
            // Disable link check validation (automatically enabled
            // during join, but not supported by TTN at this time).
            LMIC_setLinkCheckMode(0);
        break;
        case EV_RFU1:
            printf("EV_RFU1\n");
        break;
        case EV_JOIN_FAILED:
            printf("EV_JOIN_FAILED\n");
        break;
        case EV_REJOIN_FAILED:
            printf("EV_REJOIN_FAILED\n");
        break;
        case EV_TXCOMPLETE:
            printf("EV_TXCOMPLETE (includes waiting for RX windows)\n");
            if (LMIC.txrxFlags & TXRX_ACK)
              printf("%s Received ack\n", strTime);
            if (LMIC.dataLen) {
              printf("%s Received %d bytes of payload\n", strTime, LMIC.dataLen);
            }
            digitalWrite(RF_LED_PIN, LOW);
            // Schedule next transmission
            os_setTimedCallback(&sendjob, os_getTime()+sec2osticks(TX_INTERVAL), do_send);
        break;
        case EV_LOST_TSYNC:
            printf("EV_LOST_TSYNC\n");
        break;
        case EV_RESET:
            printf("EV_RESET\n");
        break;
        case EV_RXCOMPLETE:
            // data received in ping slot
            printf("EV_RXCOMPLETE\n");
        break;
        case EV_LINK_DEAD:
            printf("EV_LINK_DEAD\n");
        break;
        case EV_LINK_ALIVE:
            printf("EV_LINK_ALIVE\n");
        break;
        default:
            printf("Unknown event\n");
        break;
    }
}

void sig_handler(int sig)
{
  printf("\nBreak received, exiting!\n");
  force_exit=true;
}

int main(void) 
{
    // caught CTRL-C to do clean-up
    signal(SIGINT, sig_handler);
    
    printf("%s Starting\n", __BASEFILE__);
    
      // Init GPIO bcm
    if (!bcm2835_init()) {
        fprintf( stderr, "bcm2835_init() Failed\n\n" );
        return 1;
    }

	// Show board config
    printConfig(RF_LED_PIN);
    printKeys();

    // Light off on board LED
    pinMode(RF_LED_PIN, OUTPUT);
    digitalWrite(RF_LED_PIN, HIGH);

    os_init();
    LMIC_reset();

    do_send(&sendjob);

    while(!force_exit) {
      os_runloop_once();

      usleep(1000);
    }

    digitalWrite(RF_LED_PIN, LOW);
    
    // module CS line High
    digitalWrite(lmic_pins.nss, HIGH);
    printf( "\n%s, done my job!\n", __BASEFILE__ );
    bcm2835_close();
    return 0;
}

DevEUI : B827EBC9337D0400
AppEUI: 692a41b807e9f840
Appkey: 5ccc419a5f85614a7856d8c9ccef5193