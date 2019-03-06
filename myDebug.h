
#ifndef MYDEBUGH
#define MYDEBUGH

#include "Arduino.h"



//---------------------------------------------------
// Use of Arrows with display() and display_bytes.
//---------------------------------------------------
//
// For myUsbHost and it's myMax3421e (in host mode)
//
//      myUSBHost   -->   sending to myMa3421e
//                  <--   received from myMax3421e
//
//                  ..>   received from client (other) code
//                  <..   sent to client code (other) code
//             
//                  +>>   direct to MAX3421e
//                  <<+   direct from MAX3421e
//
//      myMax3421e  ->>    sent to hostedDevice
//                  <<-    received from hosted device
//
//                  .>>   received from client (other) code
//                  <<.   sent to client code (other) code
//
// For teensyPassThruDevice
//
//                  =>>    received from USB module (masterHost)
//                  <<=    sent to usb module (masterHost)
//
//                  <==    received from myUsbHost
//                  ==>    sent to myUsbHost
//
//                  ::>    received from client (other) code
//                  <::    sent to client (other) code
//
// For myMax3421e in peripheral mode
//
//                  >>>   received from masterHost
//                  <<<   sent to masterHost
//
//   myUsbDevice & myPassThruDevice
//
//                  -->   received from myMax3421e
//                  <--   sending to myMax3421e


   
#define USE_MY_DISPLAY  1
    // Turn on display() routines
#define USE_MEMORY_CHECK  1
    // Turn on memory checking (not on teensy)

        
#ifdef CORE_TEENSY    
    // use the hardware Serial1 port on TEENSY
    #define USE_SERIAL1  1
    #define MY_BAUD_RATE  460800    // 921600, 460800, 230400, 115200
    #undef USE_MEMORY_CHECK
#else
    #define USE_SERIAL1  0
    #define MY_BAUD_RATE  115200    // 921600 // 460800, 230400, 115200
    #undef USE_MEMORY_CHECK
#endif


#if USE_SERIAL1
    #define dbgSerial Serial1
#else
    #define dbgSerial Serial
#endif

    
extern int debug_level;
    // always defined so code that might touch it
    // does not have `to change if !USE_MY_DISPLAY
uint8_t myButtonPressed(uint8_t pin, uint8_t *state);
    // always available


//----------------------------------------
// display routines
//----------------------------------------

#if USE_MY_DISPLAY 

    #define display0(l,f)       {       \
        if (l <= debug_level)           \
        {                               \
            char dt_buf[120];           \
            strcpy_P(dt_buf,f);         \
            indent();                   \
            dbgSerial.println(dt_buf);  \
        }}
    
    #define display1(l,f,p1)   {        \
        if (l <= debug_level)           \
        {                               \
            char dt_buf[120];           \
            indent();                   \
            strcpy_P(dt_buf,f);         \
            dbgSerial.print(dt_buf);    \
            dbgSerial.print(" ");       \
            dbgSerial.println(p1);      \
        }}
    #define display2(l,f,p1,p2) {       \
        if (l <= debug_level)           \
        {                               \
            char dt_buf[120];           \
            strcpy_P(dt_buf,f);         \
            indent();                   \
            dbgSerial.print(dt_buf);    \
            dbgSerial.print(" ");       \
            dbgSerial.print(p1);        \
            dbgSerial.print(" ");       \
            dbgSerial.println(p2);      \
        }}
    #define display3(l,f,p1,p2,p3) {    \
        if (l <= debug_level)           \
        {                               \
            char dt_buf[120];           \
            strcpy_P(dt_buf,f);         \
            indent();                   \
            dbgSerial.print(dt_buf);    \
            dbgSerial.print(" ");       \
            dbgSerial.print(p1);        \
            dbgSerial.print(" ");       \
            dbgSerial.print(p2);        \
            dbgSerial.print(" ");       \
            dbgSerial.println(p3);      \
        }}
    

    void init_my_debug();
    extern int proc_level;

    #define proc_entry()            proc_level++
    #define proc_leave()            proc_level--

    char *hex2(uint8_t b);
    char *hex4(uint16_t u);
    char *hex8(uint32_t l);
    
    void indent();
    void clearPutty();
    void display_bytes(int level, const char *label, uint8_t *buf, int len);
    void display_bytes_ep(int level, uint8_t ep, const char *label, uint8_t *buf, int len);
    void display_bytes_long(int level, uint16_t addr, uint8_t *buf, int len);

    
#else   // !USE_MY_DISPLAY

    #define init_my_debug()
    #define clearPutty()
    #define display0(l,f)
    #define display1(l,f,p1)
    #define display2(l,f,p1,p2)
    #define display3(l,f,p1,p2,p3)
    #define hex2(b)
    #define hex4(u)
    #define hex8(ul)
    #define display_bytes(a,b,c,d)
    #define display_bytes_ep(a,e,b,c,d)
    #define display_bytes_long(a,b,c,d)
    #define proc_entry()       
    #define proc_leave()

#endif  // !USE_MY_DEBUG && !USE_MY_DISPLAY


//----------------------------------------
// memory checking
//----------------------------------------

#ifdef USE_MEMORY_CHECK
    extern "C" {
        
        #define MEMORY_LIMIT_WARNING 256
    
        void checkMem();
            // call this at the top of a routine and if there is less than
            // MEMORY_LIMIT_WARNING of bytes of free memory, it will print
            // a warning to the dbgSerial port.  Note that this is called
            // in all debug routines, so merely calling display() in your
            // routine *should* roughly check the memory situation
        
        void dbgMem();
            // call this and it will print out the amount of free memory
            // to the dbgSerial port    
    }
#else
    #define checkMem()
    #define dbgMem();
#endif


#endif  // !MYDEBUGH