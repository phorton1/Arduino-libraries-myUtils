#ifndef __ampMeter_h__
#define __ampMeter_h__

// there are fairly agressive at 5V and still seem to save the chip

#define DEFAULT_OVERLOAD            400
#define DEFAULT_AVERAGE_OVERLOAD    200

// These defines were hard fought to determine, and are totally kludgy
// See CPP file for more info


#define MICROS_PER_METER_READ      250    // how often to sample the ACS712 (4 times per ms)
#define METER_READS_PER_SAMPLE     20     // how many analogReads do we look at per "sample" (5 ms)
#define SAMPLE_BUFFER_SIZE         40     // how many samples kept in circular buffer for "instantaneous" (200 ms)
#define AVERAGE_BUFFER_SIZE        20     // how many "instantaneous" values kept in circular buffer for "average" (4 seconds)

#define AVERAGE_BUFFER_CHECK       5
    // check the "average" for overload every 5 additions to the
    // second circular buffer (once per second)

#define METER_SCALE_UP_FACTOR      32
    // how much to scale up the 0-biased "samples" added the the first circular buffer
    // to get a reasonable value for my "instantaneous" (average of 1st circular buffer) reading.





class ampMeter
{
public:

    ampMeter(int pin, int max_peak=DEFAULT_OVERLOAD, int max_average=DEFAULT_AVERAGE_OVERLOAD);

    void setOverload(int value)         { m_max_peak = value; }
    void setAveragOverload(int value)   { m_max_average = value; }
        // override the constants used for overload at your own risk

    void task();
    void clear();
    bool overload()         { return m_bOverload; }
    void clearOverload()    { m_bOverload = 0; }

    int milliAmps();
    int averageMilliAmps();


private:

    int m_pin;
    int m_max_peak;
    int m_max_average;
    bool m_bOverload;

    int m_sample;           // the "sample" currently being determined (max reading over 5 ms)
    int m_read_counter;     // count to METER_READS_PER_SAMPLE then use m_sample
    int m_sample_counter;   // indec into "sample buffer"
    int m_average_counter;  // index into "average_buffer"

    uint32_t m_last_read_time;

    int m_sample_buffer[SAMPLE_BUFFER_SIZE];
    int m_average_buffer[AVERAGE_BUFFER_SIZE];

};


extern ampMeter theAmpMeter;


#endif // !__ampMeter_h__
