
#include <stdio.h>
#include <string.h>
#include <math.h>

#include <pico/stdlib.h>

#include <FreeRTOS.h>
#include <queue.h>
#include <task.h>

#include "tkjhat/sdk.h"


#define DEFAULT_STACK_SIZE 1024
#define LIIKKEEN_RAJA 0.15 // valitaan arvo, joka pitää ylittää että voidaan määrittää liikkuuko sensori vai ei
#define SAMPLE_DELAY_MS 200


//TILAN PÄIVITYS

enum state { IDLE=1, STATE_MOVING};
enum state programState = IDLE;



static void update_task(float ax, float ay, float az){  // Alustaa ensimmäiset arvot, että voidaan tulkita onko liikettä tapahtunut (on jokin mihin verrata)
    float aikaisempi_ax = 0, aikaisempi_ay = 0, aikaisempi_az = 0;
    static bool first_read = true; // True, jolloin seuraava funktio alkaa pyörimään

    if (first_read){
        aikaisempi_ax = ax;
        aikaisempi_ay = ay;
        aikaisempi_az = az;
        first_read = false; // false, että ensimmäinen mittaus ja sen arvot kerätään vain kerran -> task tapahtuu vain kerran
        return;
    }

     // lasketaan seuraavaksi arvojen erotus, josta voidaan päätellä onko laite liikkeellä vai pysähtyneenä
    float erotus = fabs(aikaisempi_ax - ax) + fabs(aikaisempi_ay - ay) + fabs(aikaisempi_az - az);

    if (erotus > LIIKKEEN_RAJA) {
        programState = STATE_MOVING;
    } else {
        programState = IDLE;
    }

    aikaisempi_ax = ax;
    aikaisempi_ay = ay;
    aikaisempi_az = az;
}
//TILAN PÄIVITYS LOPPUU


//IMU-ARVOJEN LUKEMINEN -hyödynnettiin examples/hat_im_cdc_ex


void imu_task(void *pvParameters) {
    (void)pvParameters;
    
    float ax, ay, az, gx, gy, gz, t;
    // Setting up the sensor. 
    if (init_ICM42670() == 0) {
        printf("ICM-42670P initialized successfully!\n");
        if (ICM42670_start_with_default_values() != 0){
            printf("ICM-42670P could not initialize accelerometer or gyroscope");
        }
    } else {
        printf("Failed to initialize ICM-42670P.\n");
    }
    // Datan keräys 
    while (1)
    {
        if (ICM42670_read_sensor_data(&ax, &ay, &az, &gx, &gy, &gz, &t) == 0) {
            update_task(ax, ay, az);
        }
        vTaskDelay(pdMS_TO_TICKS(200));
    }

}

//IMU-ARVOT LUETTU



// PRINT-TASKI -tässä hyödynnetty examples/hello_serial_bidirectional_client

static void print_task(void *arg){
    (void)arg;

    while(1){
        switch(programState) {
            case IDLE:
                printf("- ");
                break;
            case STATE_MOVING:
                printf(". ");
                break;
            
        }
        fflush(stdout); //lähettää . ja - heti, ei jää odottamaan rivinvaihtoa
        vTaskDelay(pdMS_TO_TICKS(300));
    }
}
        
//PRINT-TASKI LOPPUU


//MAIN - hyödynnettiin src/template esimerkkiä

int main() {
    stdio_init_all();
    
    init_hat_sdk();
    sleep_ms(300); //Wait some time so initialization of USB and hat is done.

    TaskHandle_t IMUTask = NULL;
    TaskHandle_t PRINTTask =NULL;
    
    xTaskCreate(imu_task,"IMU TASK", DEFAULT_STACK_SIZE, NULL, 2, &IMUTask);
    xTaskCreate(print_task, "PRINT TASK", DEFAULT_STACK_SIZE, NULL, 2, &PRINTTask);

    vTaskStartScheduler();

    return 0;
    }



