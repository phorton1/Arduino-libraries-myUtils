//------------------------------------------
// button and expression pedal routines
//------------------------------------------

#include "myDebug.h"

int debug_level = 0;
    // defined even if MY_DEBUG is turned off

uint8_t myButtonPressed(uint8_t pin, uint8_t *state)
    // available if !MY_DEBUG
{
    if (digitalRead(pin))
    {
        if (*state)
        {
            *state = 0;
        }
    }
    else if (!*state)
    {
        *state = 1;
        return 1;
    }
    return 0;
}


#if USE_MY_DISPLAY

    int proc_level = 0;

    //-------------------------
    // hex display routines
    //-------------------------
    // use 4 buffers so they can be
    // called from my display macros
    
    #define NUM_HEX_BUFS  4
    char hex_buf[NUM_HEX_BUFS][10];
    int next_hex_buf = 0;
    
    char *hex2(uint8_t v)
    {
        int bnum = next_hex_buf++;
        sprintf(hex_buf[bnum],"0x%02x",v);
        if (next_hex_buf>=NUM_HEX_BUFS) next_hex_buf=0;
        return hex_buf[bnum];
    }
    
    char *hex4(uint16_t v)
    {
        int bnum = next_hex_buf++;
        sprintf(hex_buf[bnum],"0x%04x",v);
        if (next_hex_buf>=NUM_HEX_BUFS) next_hex_buf=0;
        return hex_buf[bnum];
    }
    
    char *hex8(uint32_t v)
    {
        int bnum = next_hex_buf++;
        sprintf(hex_buf[bnum],"0x%08lx",v);
        if (next_hex_buf>=NUM_HEX_BUFS) next_hex_buf=0;
        return hex_buf[bnum];
    }
    
    
    //-------------------------
    // rest of API
    //-------------------------
    
    void init_my_debug()
    {
        dbgSerial.begin(MY_BAUD_RATE);
        checkMem();
    }
    
    void clearPutty()
    {
        // dbgSerial.print("\033[1;1]f");
        // dbgSerial.print("\033[1J");
        dbgSerial.print("\033[2J");
        dbgSerial.print("\033[3J");
        
    }

    void indent()
    {
        if (proc_level < 0)
        {
            dbgSerial.println("WARNING: MISSING (unbalanced) proc_entry!!");
            proc_level = 0;
        }
        int i=proc_level;
        // if (i>12) i=12;
        while (i--) { dbgSerial.print("    "); };
    }

    
    #ifdef CORE_TEENSY
    
        // optimized version of display_bytes (speed vs memory)
        
        #define MAX_DISPLAY_BYTES    2048
        char disp_bytes_buf[MAX_DISPLAY_BYTES];

        char *indent_buf(char *obuf)
        {
            if (proc_level < 0)
            {
                dbgSerial.println("WARNING: MISSING (unbalanced) proc_entry!!");
                proc_level = 0;
            }
            int i = proc_level * 4;
            while (i--) { *obuf++ = ' '; }
            return obuf;
        }

        void display_bytes(int level, const char *label, uint8_t *buf, int len)
        {
            if (level > debug_level) return;
            display_bytes_ep(level,0,label,buf,len);
        }

                
        void display_bytes_ep(int level, uint8_t ep, const char *label, uint8_t *buf, int len)
        {
            if (level > debug_level) return;
            char *obuf = disp_bytes_buf;
            obuf = indent_buf(obuf);
            
            uint8_t space_len = 4;
            if (ep)
            {
                *obuf++ = '0' + ep;
                space_len = 5;
            }
            while (*label)
            {
                *obuf++ = *label++;
            }
            *obuf++ = ' ';
            
            if (!len)
            {
                strcpy(obuf," (0 bytes!!)");
                dbgSerial.println(disp_bytes_buf);
                return;
            }

            // dbgSerial.println("its not just the call to dbgSerial.println");
            // return;
            
            int bnum = 0;
            while (bnum < len && bnum < MAX_DISPLAY_BYTES-8)
            {
                if (bnum % 16 == 0)
                {
                    if (bnum)
                    {
                        *obuf++ = 10;   // "\r";
                        *obuf++ = 13;   // "\n";
                        obuf = indent_buf(obuf);
                        for (int i=0; i<space_len; i++)
                        {
                            *obuf++ = ' ';
                        }
                    }
                }
                
                #if 1
                    uint8_t byte = buf[bnum++];
                    uint8_t nibble = byte >> 4;
                    byte &= 0x0f;
                    
                    *obuf++ = nibble > 9 ? ('a' + nibble-10) : ('0' + nibble);
                    *obuf++ = byte > 9 ? ('a' + byte-10) : ('0' + byte);
                    *obuf++ = ' ';
                    
                #else
                    sprintf(obuf,"%02x ",buf[bnum++]);
                    obuf += 3;
                #endif
            }
            *obuf++ = 0;
            dbgSerial.println(disp_bytes_buf);
        }



    
    #else      // display_bytes() on arduino
    
        void display_bytes(int level, const char *label, uint8_t *buf, int len)
        {
            checkMem();    
            if (level > debug_level) return;
            if (!len)
            {
                indent();
                dbgSerial.print(label);
                dbgSerial.println(" (0 bytes!!)");
                return;
            }
            
            char tbuf[6];
            int bnum = 0;
            while (bnum < len)
            {
                if (bnum % 16 == 0)
                {
                    if (!bnum)
                    {
                        indent();
                        dbgSerial.print(label);
                        dbgSerial.print(" ");
                    }
                    else
                    {
                        dbgSerial.println();
                        indent();
                        dbgSerial.print("    ");
                    }
                }
                sprintf(tbuf,"%02x ",buf[bnum++]);
                dbgSerial.print(tbuf);
            }
            dbgSerial.println();
        }
        
    #endif      // arduino version of display_bytes
    
    
    void display_bytes_long(int level, uint16_t addr, uint8_t *buf, int len)
    {
        checkMem();    
        if (level > debug_level) return;
        if (!len)
        {
            indent();
            dbgSerial.println("0x000000 (0 bytes!!)");
            return;
        }
        
        char tbuf[6];
        char char_buf[17];
        memset(char_buf,0,17);
        
        int bnum = 0;
        while (bnum < len)
        {
            if (bnum % 16 == 0)
            {
                if (bnum)
                {
                    dbgSerial.print("    ");
                    dbgSerial.println(char_buf);
                    memset(char_buf,0,17);
                }
                indent();
                dbgSerial.print(hex4(addr + bnum));
                dbgSerial.print(": ");
            }

            uint8_t c = buf[bnum];
            sprintf(tbuf,"%02x ",c);
            dbgSerial.print(tbuf);
            char_buf[bnum % 16] = (c >= 32) && (c < 128) ? ((char) c) : '.';
            bnum++;
        }
        if (bnum)
        {
            dbgSerial.print("    ");
            dbgSerial.println(char_buf);
        }
    }
    
#endif  // USE_MY_DISPLAY


    
//------------------------------------------------------
// memory debugging
//------------------------------------------------------

#if USE_MEMORY_CHECK
    extern "C" {
    
        // code copied from Arduino Playground

        extern unsigned int __heap_start;
        extern void *__brkval;
        
        
        // The free list structure as maintained by the
        // avr-libc memory allocation routines.
        
        struct __freelist {
            size_t sz;
            struct __freelist *nx;
        };
        
        // The head of the free list structure
        
        extern struct __freelist *__flp;
        
        
        // Calculates the size of the free list 
        int freeListSize()
        {
            struct __freelist* current;
            int total = 0;
            for (current = __flp; current; current = current->nx)
            {
                total += 2;
                    // Add two bytes for the memory block's header  
                total += (int) current->sz;
            }
            return total;
        }
        
        int freeMemory()
        {
            int free_memory;
            if ((int)__brkval == 0)
            {
                free_memory = ((int)&free_memory) - ((int)&__heap_start);
            }
            else
            {
                free_memory = ((int)&free_memory) - ((int)__brkval);
                free_memory += freeListSize();
            }
            return free_memory;
        }
                
        // my code
        
        void checkMem()
        {
            int free_mem = freeMemory();
            if (free_mem < MEMORY_LIMIT_WARNING)
            {
                dbgSerial.print("MEMORY WARNING!! free=");
                dbgSerial.println(free_mem);
            }
        }
        
        void dbgMem()
        {
            dbgSerial.print("DEBUG MEMORY = ");
            dbgSerial.println(freeMemory());
        }
        
    }   // extern "C"
    
#endif // USE_MEMORY_CHECK
