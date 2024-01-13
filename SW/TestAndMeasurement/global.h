#ifndef GLOBAL_H_
#define GLOBAL_H_

// Below deactivates some functionality, such that the firmware can be used for speedtests using a dummy device.
// The dummy device consists out of 3K pullup resistors on all GPIB lines
// and has a logic implemented between NDAC and DAV to mimic handshaking
//#define SPEEDTEST_DUMMY_DEVICE

#endif /* GLOBAL_H_ */
