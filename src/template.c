
#include <stdio.h>
#include <string.h>

#include <pico/stdlib.h>

#include <FreeRTOS.h>
#include <queue.h>
#include <task.h>

#include "tkjhat/sdk.h"

// Default stack size for the tasks. It can be reduced to 1024 if task is not using lot of memory.
#define DEFAULT_STACK_SIZE 2048 
#define LIIKKEEN_RAJA 1? // valitaan arvo, joka pitää ylittää että voidaan määrittää liikkuuko sensori vai ei 

//TILAN PÄIVITYS

//Add here necessary states
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
        programState = IDEL;
    }

    aikaisempi_ax = ax;
    aikaisempi_ay = ay;
    aikaisempi_az = az;
}
//TILAN PÄIVITYS LOPPUU








    
    //for(;;){
       // tight_loop_contents(); // Modify with application code here.
      //  vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

int main() {
    stdio_init_all();
    // Uncomment this lines if you want to wait till the serial monitor is connected
    /*while (!stdio_usb_connected()){
        sleep_ms(10);
    }*/ 
    init_hat_sdk();
    sleep_ms(300); //Wait some time so initialization of USB and hat is done.

    TaskHandle_t myExampleTask = NULL;
    // Create the tasks with xTaskCreate
    BaseType_t result = xTaskCreate(example_task,       // (en) Task function
                "example",              // (en) Name of the task 
                DEFAULT_STACK_SIZE, // (en) Size of the stack for this task (in words). Generally 1024 or 2048
                NULL,               // (en) Arguments of the task 
                2,                  // (en) Priority of this task
                &myExampleTask);    // (en) A handle to control the execution of this task

    if(result != pdPASS) {
        printf("Example Task creation failed\n");
        return 0;
    }

    // Start the scheduler (never returns)
    vTaskStartScheduler();

    // Never reach this line.
    return 0;
}

