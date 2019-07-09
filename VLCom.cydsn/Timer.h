// タイマー部

#ifndef	_TIMER_H_
#define	_TIMER_H_

#include<stdbool.h>    
    
/******************************
 API関数
 ******************************/

// 初期化する。
// msec: 周期タイマの周期[msec]
void Timer_init(int msec);


// タイムアウト計測をスタートする。
// msec: 時間[msec]
void Timer_start(int msec);

// タイムアウトを待つ。
void Timer_wait(void);

// タイムアウトしたか？
bool Timer_isUp(void);

// 周期タイマのチェック
// 戻り値: 周期フラグが立ったか？ (そしてフラグはクリアされる)
bool Timer_hasCycled(void);

// 周期タイマのチェック
// 戻り値: 指定時間経過したか？
bool Timer_isOver(int msec);


#endif
