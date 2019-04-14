#ifndef __rpiSerialMonitor__
#define __rpiSerialMonitor__


class rpiSerialMonitor
{
public:
    
    rpiSerialMonitor(uint8_t start_command, uint8_t num_commands);
        // construct with a key (i.e. 'd') and a "number of commands"
        // task() will return one of the command keys, if appropriate
        // for you to act upon.  Otherwise handles:
        //
        //      ^B to reboot the the rpi
        //      passing all other serial data between the two
        
    ~rpiSerialMonitor();
    
    uint8_t task();
    void    rebootPi();
    
private:
    
    uint8_t m_start_command;
    uint8_t m_num_commands;
    uint8_t m_key_pressed;
    
    elapsedMillis  m_key_timer;
    
};



#endif // !__rpiSerialMonitor__