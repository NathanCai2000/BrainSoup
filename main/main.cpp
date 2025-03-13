#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "sdkconfig.h"
#include "math.h"

#include "Accel_Handler.h"
#include "LED_Handler.h"
#include "RC_Handler.h"
#include "MC_Handler.h"

//CRSF range:           [174, 1811]                     <- -100 to 100
//DShot normal range:   [0], [48, 2047]                 <- 0, 1 to 100
//DShot 3D range:       [1023, 48], [0], [1025, 2047]   <- -100 to -1, 0, 1 to 100%
#define mapRange(a1,a2,b1,b2,s) (b1 + (s-a1)*(b2-b1)/(a2-a1))

#define ACCEL_OFFSET    3.5 //Accel to Center of Rotation (cm)
#define G_MIN           10  //Minimum gs to do melty stuff

void setup();
void tankdrive(crsf_channels_t rc_out);
void spin_once(crsf_channels_t rc_out);
int DShot_Bidirectional(int rc_out, int deadzone_width);


static int LED_OFFSET = 0;          //Adjust for LED drift
static float MOTOR_SCALING = 1;     //Adjusts the "cone" size of motor activation
static int MAX_THROTTLE = 15;       //Maximum motor speed precentage (Leave +/-3% relative to this precentage)

Motor M1;
LED ledG;
LED ledR;

extern "C" void app_main(void) {

    setup();

    printf("Staring -------------------------\n");

    for(;;) {

        //Failsafe
        //If connection to Transmitter is lost, stop power to motors and blink LED/s
        printf("RC Healthy? %s \n", RC_Health()? "Y" : "N");
        while (!RC_Health()) {
            //Motor Power OFF
            M1.set_power(0);

            //Fast Blink LEDs
            ledG.LED_BLINK(2, 100);
            ledR.LED_ON();

        }

        //Get Receiver data
        crsf_channels_t rc_out = get_RC();

        int throttle = mapRange(174, 1811, 0, 100, rc_out.ch3);

        //If throttle is >20% use melty controls, else Tank Drive
        if (throttle > 20) {
            //Melty drive
            printf("MELTING!!!!!!!!!!\n");
            spin_once(rc_out);

        }
        else {
            //Tank Drive
            tankdrive(rc_out);
        }

        //int precent = mapRange(174, 1811, 0, 100, rc_out.ch1);
        //int dshot = mapRange(174, 1811, 0, 1000, rc_out.ch3);
    }
    
}

void setup() {

    ledG.init_LED(3, 0);
    ledR.init_LED(2, 1);
    init_RC(7, 6);
    init_accel();

    calibrate(200);
    ledG.LED_ON();

    M1.init(GPIO_NUM_5, "A");

    ledR.LED_BLINK(3, 500);

}

void tankdrive(crsf_channels_t rc_out) {

    ledG.LED_ON();
    ledR.LED_ON();
    int m1 = DShot_Bidirectional(rc_out.ch1, 100);

    //printf("Throttle: %d\n", m1);
    M1.set_power(m1);

}

/**
 * This system uses tank controls instead of strafing:
 * Forward-Back traslation is as normal Meltybrain style.
 * Turning is done by intentionally inducing a bit of drift into the heading.
 * This is done by adding a very small time offset at the start of calculations,
 * such that each rotation will compound a small delay for the next reading,
 * effectively stacking drift on the previous heading.
 */
void spin_once(crsf_channels_t rc_out) {

    //Cycle time trackers
    uint16_t start_time = esp_timer_get_time() / 1000; //Get time at start of cycle    (In us)
    uint16_t time_past = 0;                   //How much time has passed      (In ms)

    //Get speed and direction
    uint8_t throttle = mapRange(665, 1811, 1, 15, rc_out.ch3);
    int stick1 = rc_out.ch1;
    uint8_t forbac = 0;

    //Get forward/backwards speeds based on stick position
    if (stick1 < 960) {
        forbac = mapRange(174, 960, -3, 0, stick1);
    }
    else if (stick1 > 1025) {
        forbac = mapRange(1025, 1811, 0, 3, stick1);
    }

    //Map speeds to DShot input numbers
    int MotorR = mapRange(0, 100, 1025, 2047, throttle);
    int MotorA = mapRange(0, 100, 1025, 2047, throttle + forbac);
    int MotorB = mapRange(0, 100, 1025, 2047, throttle - forbac);

    //Set Presets
    if (rc_out.ch4 < 800) {
        LED_OFFSET--;
    }
    else if (rc_out.ch4 > 1248) {
        LED_OFFSET++;
    }
    //Set adjusted % of motor operation
    MOTOR_SCALING = mapRange(174, 1811, -.75, .75, rc_out.ch6);

    //Get turning amount:
    float t_tmp = rc_out.ch2;
    float turn = 0;
    if (t_tmp < 940) {
        turn = mapRange(174, 940, .9, .99, t_tmp);
    }
    else if (t_tmp > 1045) {
        turn = mapRange(1045, 1811, 1.01, 1.1, t_tmp);
    }
    else {
        turn = 1;
    }
    
    //Get Acceleration Data;
    float x, y, z;
    get_g(x, y, z);

    if (x < G_MIN) {
        M1.set_power(MotorR);
        ledR.LED_BLINK(6, 50);
        ledG.LED_BLINK(2, 500);
    }
    else {

        //Calculate RPM from g
        // RPM = Sqrt( g / (Radius[cm] * 1.118 * 0.00001))
        // Period (ms) = 1/rpm * 60 * 1000
        float r_Period = 1 / sqrt(x /(ACCEL_OFFSET * 0.00001118)) * 6000;

        //Temp testing
        // float r_Period = 1600;

        //LED presets:
        float LED_size = mapRange(174, 1811, 10, 25, rc_out.ch3)/ 100.f;
        float LED_start = (1- LED_size)/2 + LED_OFFSET;
        float LED_end = LED_start + LED_size;
        LED_start *= r_Period;
        LED_end *= r_Period;
        r_Period *= turn;

        float M1A_start = (.125 - .10 * MOTOR_SCALING) * r_Period;
        float M1A_end   = (.375 + .10 * MOTOR_SCALING) * r_Period;

        float M1B_start = (.625 - .10 * MOTOR_SCALING) * r_Period;
        float M1B_end   = (.875 + .10 * MOTOR_SCALING) * r_Period;

        while (time_past < r_Period) {

            //Motor phase A control
            if (time_past > M1A_start && time_past < M1A_end) {
                M1.set_power(MotorA);
            }
            //Motor phase B control
            else if (time_past > M1B_start && time_past < M1B_end) {
                M1.set_power(MotorB);
            }
            else {
                M1.set_power(MotorR);
            }
            //LED control
            if (time_past > LED_start && time_past < LED_end) {
                ledG.control(100);
                ledR.control(100);
            }
            else {
                ledG.control(0);
                ledR.control(0);
            }
            
            //Update how much time has passed
            time_past = esp_timer_get_time()/1000 - start_time;   //Do not forget to /1000 to get ms not us
            
        }
    }
}

int DShot_Bidirectional(int rc_out, int deadzone_width) {
    int dz_lower = (174 + 1811 - deadzone_width)/2;
    int dz_upper = (174 + 1811 + deadzone_width)/2;

    if (rc_out < dz_lower) {
        return mapRange(174, dz_lower, 1023, 48, rc_out);
    }
    else if (rc_out > dz_upper) {
        return mapRange(dz_upper, 1811, 1024, 2047, rc_out);
    }
    else {
        return 0;
    }
}