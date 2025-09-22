#include "hcsr04.h"
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>


// ---------------- �����ܼ� ----------------
// �� main.c �w�q�A���� thread ����
extern int stop_flag;

// callback ���СA�� main.c �]�w
extern hcsr04_callback distance_cb;


// ---------------- �W�n�iŪ������� ----------------
void* hcsr04_thread_func(void* arg) {
    hcsr04_all_data data;

    while(!stop_flag) {
        // 1. Ū���|���Z��
        if(hcsr04_read_all(&data)==0){
            // 2. �I�s callback�A�N��ƶǵ��޿�h
            if(distance_cb!=NULL){
                distance_cb(&data); // �^�ǥ|���Z��
            }
        } else {
            fprintf(stderr,"hcsr04 read_all failed\n");
        }

        // 3. ����Ū���W�v�A�o�̨C 100ms Ū�@��
        usleep(100000);
    }

    return NULL;
}
