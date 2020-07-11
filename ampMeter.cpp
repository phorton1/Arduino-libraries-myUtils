// ampMeter.cpp
//---------------------------------------------------------------------------
// I use 1/8 of the Arduino RAM just for this ...
//
// almost impossible to get something that matches the readings on my bench power supply
// almost impossible (for me) to do this scientifically using published ACS712 scaling factors
// so I have a kludge that works for me.
//
//      I am using 1 of the 2 available circuits on a H9110 h bridge to control the motor.
//           This chip has a documented MAXIMUM current rating of 600 ma.
//
// I connect the ACS712 output directly to an analogIn pin on the arduino.
// I add one of the blue plasic u1J63 (0.1 micro farad at 63V == 100 nano farad)
//    capacitors from the arduino analogIn pin output directly to ground.
//    Even though most RC low pass filter diagrams show a resistor from the AC712
//    output TO the capacitor/analogIn connection, I do not have one of those.
//
// I analogRead the ACS712 4 times a millisecond for 5 milliseconds and use the peak
//    value I find in that interval as the "sample".
//
// Every 5 milliseconds, I add that sample to a circular buffer. The average
//    of this circular buffer represents the "instantantaneous" current. Since I am
//    only interested in one direction of current flow, I first bias the sample down
//    to 0..511.  The crucial kludge is that I have determined, empirically, that
//    upscaling these values by 32x  (METER_SCALE_UP_FACTOR) results in an "instantanteous"
//    (average of first circular buffer) current that sort of makes sense to me.
//    This cicrcular buffer holds 40 (SAMPLE_BUFFER_SIZE) values, and so represents
//    200 milliseconds of time.  It is NOT instantantaneous.
//
// I NEVER ACTUALLY CHECK THE PEAK CURRENT TO THE CHIP!!
//
// To protect the controller chip, every time the first buffer wraps, I call the
//    milliAmps() function and compare the "instantantaneous" (average) to a constant
//    and set an overflow flag if it is exceeeded.
//
// To further protect the chip, I add THAT value to a second circular buffer
//    that holds 4 seconds worth of 'instantanteous' averages (AVERAGE_BUFFER_SIZE),
//    which is roughly the amount of time it takes the chip to heat up or cool down.
//    Once per second (every AVERAGE_BUFFER_CHECK additions to the 2nd buffer), I
//    compare the "averageMilliAmps()" (average over the 2nd buffer) to a 2nd constant
//    and set the overflow flag if it is exceeeded.
//
//--------------------------------------------------------------------------

#include <myDebug.h>
#include "ampMeter.h"

#define dbg_amp  1


#define AMP_METER_PIN   A6
    // analog input pin from ACS712 current sensor


#define SHOW_SERIAL2   1
    // define to show stuff in Arduino plotter on 2nd serial port

#if SHOW_SERIAL2
    #include <SoftwareSerial.h>
    SoftwareSerial Serial2(2, 3); // RX, TX
    bool serial2_started = false;
#endif


ampMeter theAmpMeter(AMP_METER_PIN);


ampMeter::ampMeter(int pin, int max_peak, int max_average)
{
    pinMode(m_pin,INPUT);

    m_pin = pin;
    m_max_peak = max_peak;
    m_max_average = max_average;
    m_bOverload = 0;

    clear();
}


void ampMeter::clear()
{
    m_sample            = 0;
    m_read_counter      = 0;
    m_sample_counter    = 0;
    m_average_counter   = 0;
    m_last_read_time    = 0;

    memset(m_sample_buffer,0,SAMPLE_BUFFER_SIZE * sizeof(int));
    memset(m_average_buffer,0,AVERAGE_BUFFER_SIZE * sizeof(int));
}



void ampMeter::task()
{

    #if SHOW_SERIAL2
        if (!serial2_started)
        {
            serial2_started = true;
            Serial2.begin(115200);
            display(0,"ampMeter started Serial2",0);
            Serial2.println("ampMeter started Serial2");
        }
    #endif


    uint32_t now = micros();
    if (now >= m_last_read_time + MICROS_PER_METER_READ)
    {
        m_last_read_time = now;
        int value = analogRead(m_pin);
        if (value > m_sample)           // take the highest one
            m_sample = value;

        // add to "instantaneous" (first) buffer

        m_read_counter++;
        if (m_read_counter >= METER_READS_PER_SAMPLE)
        {
            m_read_counter = 0;
            m_sample -= 512;
            if (m_sample < 0) m_sample = 0;
            m_sample *= METER_SCALE_UP_FACTOR;
            m_sample_buffer[m_sample_counter++] = m_sample;
            m_sample = 0;

            if (m_sample_counter >= SAMPLE_BUFFER_SIZE)
            {
                m_sample_counter = 0;
                int value = milliAmps();
                if (!m_bOverload && value > DEFAULT_OVERLOAD)
                {
                    m_bOverload = true;
                    my_error("OVERLOAD at %d ma",value);
                }

                m_average_buffer[m_average_counter++] = value;
                if ((m_average_counter % AVERAGE_BUFFER_CHECK) == 0)
                {
                    int average = averageMilliAmps();
                    display(dbg_amp,"ma(%d) average(%d)",value,average);
                    if (!m_bOverload && average > DEFAULT_AVERAGE_OVERLOAD)
                    {
                        m_bOverload = true;
                        my_error("AVERAGE OVERLOAD at %d ma(%d)",average,value);
                    }
                    if (m_average_counter >= AVERAGE_BUFFER_SIZE)
                        m_average_counter = 0;
                }
            }

            #if SHOW_SERIAL2
                Serial2.print("0,250,");
                Serial2.print(milliAmps());
                Serial2.print(",");
                Serial2.println(averageMilliAmps());
            #endif

        }
    }
}


int ampMeter::milliAmps()
{
    uint32_t total = 0;
    for (int i=0; i<SAMPLE_BUFFER_SIZE; i++)
        total += m_sample_buffer[i];
    total /= SAMPLE_BUFFER_SIZE;
    return total;
}


int ampMeter::averageMilliAmps()
{
    uint32_t total = 0;
    for (int i=0; i<AVERAGE_BUFFER_SIZE; i++)
        total += m_average_buffer[i];
    total /= AVERAGE_BUFFER_SIZE;
    return total;
}
