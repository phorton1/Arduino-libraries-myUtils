#ifndef __rpiSerialMonitor__
#define __rpiSerialMonitor__

#ifdef CORE_TEENSY

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

    void init();

    uint8_t task();
    void    rebootPi();

    bool rpiReady() { return rpi_running & rpi_ready; }

private:

    uint8_t rpi_running;
    uint8_t rpi_ready;

    uint8_t m_start_command;
    uint8_t m_num_commands;
    uint8_t m_key_pressed;

    elapsedMillis  m_key_timer;

};

#endif  // CORE_TEENSY

#endif // !__rpiSerialMonitor__