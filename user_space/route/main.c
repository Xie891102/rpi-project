// 測試 路線分析
#include <stdio.h>
#include "route.h"

int main(){
	int raw[] = {1, 2, 3, 1, 4};	// 直行 右轉 左轉 直行 停車
	int len = sizeof(raw)/sizeof(raw[0]);

	// 建立路線
	Route *r = create_route(raw, len);

	printf("前進路線: \n");
	while(has_next(r)){
		Action a = next_step(r);
		printf("%s  ", action_to_string(a));
	}
	printf("\n");

	// 重設路線
	reset_route(r);

	// 建立回程路線
	Route *rev = reverse_route(r);
	printf("回程路線: \n");
	while(has_next(rev)){
		Action a = next_step(rev);
		printf("%s  ", action_to_string(a));
	}
	printf("\n");

	// 釋放記憶體
	free_route(r);
	free_route(rev);
	
	return 0;
}


















