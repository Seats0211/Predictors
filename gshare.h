#ifndef __CPU_PRED_GSHARE_H__
#define __CPU_PRED_GSHARE_H__

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    unsigned globalPredictorSize;  // PHT的大小
    unsigned globalHistoryBits;    // 全局历史寄存器的位数
    unsigned globalHistory;        // 全局历史寄存器
    uint8_t *pht;                  // 模式历史表 (PHT)，使用2-bit饱和计数器
    unsigned historyMask;          // 用于截取历史的掩码
    unsigned instShiftAmt;         // 指令地址右移位数（假设为常量或外部传入）
} GshareBP;

// 函数声明
GshareBP *gshare_bp_create(unsigned globalPredictorSize, unsigned globalHistoryBits, unsigned instShiftAmt);
bool gshare_bp_lookup(GshareBP *bp, uint64_t branch_addr, void **bp_history);
void gshare_bp_update(GshareBP *bp, uint64_t branch_addr, bool taken, void *bp_history, bool squashed);
void gshare_bp_squash(GshareBP *bp, void *bp_history);
void gshare_bp_destroy(GshareBP *bp);

#endif // __CPU_PRED_GSHARE_H__