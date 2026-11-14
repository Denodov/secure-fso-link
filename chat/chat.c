#include "chat.h"
#include "../firmware/drivers/uart.h"
#include "framing.h"
#include "../firmware/drivers/time.h"

#define PAYLOAD_MAX 100u // max chat bytes per frame
#define IDLE_MS 50u // send partial line after this much silence

/* --------------- USB -> Laser --------------- */
static uint8_t tx_payload[PAYLOAD_MAX]; // text collected so far
static size_t tx_len = 0;
static uint32_t last_rx_ms = 0; // when the last byte arrived

static void flush_to_laser(void) {
    uint8_t wire[PAYLOAD_MAX + 4]; // COBS adds 1-2 bytes plus the 0x00
    size_t wire_len = 0;

    if(tx_len == 0) return; // array is empty
    // wrap text in COBS frame
    if(frame_encode(tx_payload, tx_len, wire, sizeof(wire), &wire_len) == 0) {
        for(size_t i = 0; i < wire_len; i++) {
            // push each byte of wire[] out USART1 (laser)
            uart1_write_byte(wire[i]);
        }
    }
    tx_len = 0; // start a new message
}

static void pump_usb(void) {
    uint8_t b;
    // takes bytes out of the USB receive buffer one at a time until empty
    while(uart_usb_read(&b) == 0) {
        if(b == '\r') b = '\n'; // terminals send \r when enter is pressed
                                // change \r to new line
        tx_payload[tx_len++] = b; // append the byte 
        last_rx_ms = millis();    // record the time
        // if new line, or array reached 100 bytes, call flush to laser
        if(b == '\n' || tx_len == PAYLOAD_MAX) {
            flush_to_laser();
        }
    }
// if you stopped typing mid line, send what you have
    if(tx_len > 0 && (millis() - last_rx_ms) >= IDLE_MS) {
        flush_to_laser();
    }
}

/* --------------- laser -> USB --------------- */
static uint8_t rx_frame[256]; // bytes of the frame being collected
static size_t rx_len = 0;
static int rx_overflow = 0;
volatile uint32_t bad_frames = 0; // watch in debugger
   
// print decoded text to terminal through USART2 one byte at a time
static void deliver(const uint8_t *d, size_t n) {
    for(size_t i = 0; i < n; i++) {
        if(d[i] == '\n') uart2_write_byte('\r'); // terminal wants CR+LF
        uart2_write_byte(d[i]);
    }
}

static void pump_fso(void) {
    uint8_t b;
    // take bytes out of the laser receive buffer one at a time
    while(uart_fso_read(&b) == 0) {
        if(b != 0x00) { // still inside frame, keep collecting
            if(rx_len < sizeof(rx_frame)) rx_frame[rx_len++] = b;
            // if frame is full, set overflow so frame is thrown away later
            else rx_overflow = 1;
            continue;
        }
    // got 0x00, frame complete
    if(rx_overflow) {
        bad_frames++;
    } else if(rx_len > 0) { // if frame has any bytes
        uint8_t out[PAYLOAD_MAX + 4];
        size_t out_len = 0;
        // on success, pass to deliver()
        // on failure, add one to bad_frames and drop it 
        // a 0x00 with nothing before it is ignored
        if(frame_decode(rx_frame, rx_len, out, sizeof(out), &out_len) == 0)
            deliver(out, out_len);
        else
            bad_frames++;
    }
    // get ready for next frame
    rx_len = 0;
    rx_overflow = 0;

    }
}
void chat_poll(void) {
    pump_usb();
    pump_fso();
}
