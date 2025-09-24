#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include "hcsr04.h"

int stop_flag = 0;
hcsr04_callback distance_cb = NULL;

void my_distance_cb(hcsr04_all_data *data) {
    for(int i=0;i<4;i++){
        printf("Sensor %d: %d cm\n", i, data->ultrasonic[i].distance);
    }
}

int main() {
    distance_cb = my_distance_cb;

    if(hcsr04_open_all() != 0){
        return -1;
    }

    pthread_t tid;
    pthread_create(&tid, NULL, hcsr04_thread_func, NULL);

    sleep(5);  // Åª 5 ¬í

    stop_flag = 1;
    pthread_join(tid, NULL);

    hcsr04_close_all();
    return 0;
}
