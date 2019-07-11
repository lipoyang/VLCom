// タイマ部
#include "project.h"
#include "Timer.h"

/******************************
 内部変数
 ******************************/
// 周期タイマフラグ
static volatile bool hasCycled;

// タイムアウト計測用 (カウントダウンであることに注意)
static uint16 T1;   // 開始時間
static uint16 T2;   // 終了時間

/******************************
 内部関数
 ******************************/

// タイマー割り込み
CY_ISR(ISR_TIMER_CYCLE)
{
    hasCycled = true;
    
    Timer_Cycle_ReadStatusRegister();
}

/******************************
 API関数
 ******************************/

// 初期化する。
// msec: 周期タイマの周期[msec]
void Timer_init(int msec)
{
    // フリーランタイマをスタート
    Timer_Free_Start();
    Timer_Free_WriteCounter(0);
    
    if(msec != 0){
        // 割り込みの設定
        isr_Timer_Cycle_StartEx(ISR_TIMER_CYCLE);
        
        // 周期タイマをスタート
        hasCycled = false;
        uint16 cnt = (uint16)(msec * 10);
        Timer_Cycle_Start();
        Timer_Cycle_WritePeriod(cnt);
    }
}

// タイムアウト計測をスタートする。
// msec: 時間[msec]
void Timer_start(int msec)
{
    CyGlobalIntDisable;
    uint16 cnt = (uint16)(msec * 10);
    T1 = Timer_Free_ReadCounter();
    T2 = T1 - cnt; // カウントダウンであることに注意
    CyGlobalIntEnable;
}

// タイムアウトを待つ。
void Timer_wait(void)
{
    while(!Timer_isUp()){
        ;
    }
}

// タイムアウトしたか？
bool Timer_isUp(void)
{
    // カウントダウンであることに注意
    uint16 t = Timer_Free_ReadCounter();
    bool timeup = (
        ((t <= T2) && (T2 < T1)) ||
        ((T2 < T1) && (T1 < t)) ||
        ((T1 < t) && (t <= T2))
    );
    //if(timeup){
    //    DEBUG_UART_PRINT("(T1=%d T2=%d t=%d) ", T1, T2, t);
    //}
    return timeup;
}

// 周期タイマのチェック
// 戻り値: 周期フラグが立ったか？ (そしてフラグはクリアされる)
bool Timer_hasCycled(void)
{
    CyGlobalIntDisable;
    bool ret = hasCycled;
    if(hasCycled) hasCycled = false;
    CyGlobalIntEnable;
    
    return ret;
}

// 周期タイマのチェック
// 戻り値: 指定時間経過したか？
bool Timer_isOver(int msec)
{
    uint16 limit = (uint16)(msec * 10);
    
    uint16 cnt = Timer_Cycle_ReadPeriod() - Timer_Cycle_ReadCounter();
    
    return (cnt >= limit) ? true : false;
}
