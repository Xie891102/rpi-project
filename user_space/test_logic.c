// �����޿�h
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>

// �ۭq���Y��
#include "hcsr04.h"  		// �W�n�i(�׻٥\��)
#include "buzzer.h"		// ���ﾹ
#include "tcrt5000.h"		// ���~�u(�`��)
#include "motor_ctrl.h"		// ���F���

#include "route.h"			// ���u�ѪR
#include "mqtt_config.h"		// �L�u�q�T(�իפ���Rpi)
#include "uart.h"			// ���u�q�T open/release(�}/���ɮ� <-> pico)
#include "uart_thread.h"		// ���u�q�T read/write(�D����Ū/�g <-> pico)

// �ʧ@���
#define  STRAIGHT	0
#define  LEFT 	1	
#define  RIGHT	2
#define  STOP	3

// ���F�t�׳]�w
#define SPEED_INIT   40    		// �򥻳t�צʤ���
#define SPEED1 	 3     		// �L�վ�
#define SPEED2 	 6    		// �����վ�
#define SPEED3 	10      		// �j�վ�

bool node_active = false;  		// �I��`�I�ɬ��~�u��w�X��
int stop_flag = 0;         	// ���� thread ����
tcrt5000_callback logic_cb = NULL;	// callback ���� (thread �|�I�s)

int nodes[] = {STRAIGHT};
int total_nodes = sizeof(nodes)/sizeof(nodes[0]);
int current_index = 0;


//  --------------------------------------- Ū���`�I --------------------------------------------------
void handle_node(int next_action) {
    
	printf("[NODE] �`�I�B�z�}�l\n");

	// 1.�������¦�t��
	set_left_motor(SPEED_INIT, 1);
	set_right_motor(SPEED_INIT, 1);
	sleep(1);


	// 2.�ھڤU�@�Ӱʧ@�B�z 
    	switch (next_action) {
        		// ������O
		case STRAIGHT:
            			move_forward();
			printf("[NODE] �`�I����\n");
            			break;
        
		// ������O
		case LEFT:
            			turn_left();	// ���F����
			sleep(1);		// �w��
			move_forward();	// ���s�᪽��
			printf("[NODE] �`�I����\n");
	            		break;
        
		// �k����O
		case RIGHT:
            			turn_right();	// ���F�k��
			sleep(1);		// �w��
			move_forward();	// ���s�᪽��
			printf("[NODE] �`�I�k��\n");
            			break;
        
		// ����
		case STOP:
            			stop_all_motors();
			printf("[NODE] �`�I����\n");
            			break;
			
		// ��L
        		default:
            			move_forward();
			printf("[NODE] �����ʧ@\n");
            			break;
    	}
}


// ----------------------------------------- �`��tcrt5000  -------------------------------------------------
void logic(int code) {
	switch(code) {
		case 0:	// 000 => �X�y	
			stop_all_motors();
			printf("[LOGIC] \n");
			break;
	
		case 2:	// 010 => ���`��p	
			move_forward();
			printf("[LOGIC] ���`��p�A�~��e�i\n");
			break;

		case 1:	// 001 => �ӥ���
			set_left_motor(SPEED_INIT + SPEED2, 1);
			printf("[LOGIC] �ӥ����A�j�O���k�ץ�\n");
			break;

		case 3:	// 011 => �L����
			set_left_motor(SPEED_INIT + SPEED1, 1);
			printf("[LOGIC] �L�����A �����L�[�t\n");
			break;
		
        		case 4: 	// 100 => �ӥk��
		            	set_right_motor(SPEED_INIT + SPEED2, 1);  
			printf("[LOGIC] �ӥk���A�j�O�����ץ�\n");
            			break;
	
       		 case 6: 	// 110 => �L�k��
                		set_right_motor(SPEED_INIT + SPEED1, 1);
            			printf("[LOGIC] �L�k���A �k���L�[�t\n");
            			break;
		
        		case 5: 	// 101 => �ؼЦ�m (���I)
            			printf("[LOGIC] ��F���I�A�ǳư���(3���)\n");
			stop_all_motors();   	// ����
			sleep(3);            		// ����3��A�קK�D��
			break;
		
        		case 7: 	// 111 => �`�I
			if(!node_active && current_index < total_nodes){
				node_active = true;       		// ��w�`�I
				handle_node(nodes[current_index]);   // �ե�Ū���`�I�禡
				current_index++;
				printf("[LOGIC] �I��`�I\n");
				node_active = false;      		// ���s��������
			}		
			break;   
			
		default:	// ��L: �P�_���X��
            			printf("[LOGIC] �����s�X: %d\n", code);
            			break;
	}
}



// ------------------- �D�{�� -------------------
int main() {
    	printf("[MAIN] ���ն}�l\n");

	// �}�Ҭ��~�u�˸m
    	if (tcrt5000_open() < 0) {
        	printf("�L�k�}�Ҭ��~�u�˸m�A�������\n");
        	return 1;
    	}

    	pthread_t tcrt_thread;

    	// �]�w callback
    	logic_cb = logic;

    	// �إ߬��~�u�`�� thread
    	pthread_create(&tcrt_thread, NULL, tcrt5000_thread_func, NULL);

    	// ���� thread ���� (�Φb���դ��i�H�� sleep ����)
    	// �o�̰��] thread �ۤv�|�ھ� stop_flag ����
    	while(current_index < total_nodes) {
        		usleep(50000);  // 50ms�A�קK busy loop
    	}

    	// ���� thread
    	stop_flag = 1;
    	pthread_join(tcrt_thread, NULL);

    	printf("[MAIN] ���ո��u����\n");
    	return 0;
}

