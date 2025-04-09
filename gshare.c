#include "gshare.h"
#include <stdlib.h>
#include <stdio.h>

// 辅助函数：检查是否为2的幂
static inline int is_power_of_2( unsigned n )
{
    return ( n != 0 ) && ( ( n & ( n - 1 ) ) == 0 );
}

// 辅助函数：生成指定位数的掩码
static inline unsigned mask( unsigned bits ) { return ( 1U << bits ) - 1; }

// 更新全局历史
static void update_global_history( GshareBP * bp, bool taken )
{
    bp->globalHistory =
        ( ( bp->globalHistory << 1 ) | ( taken ? 1 : 0 ) ) & bp->historyMask;
}

// 获取PHT索引
static unsigned get_index( GshareBP * bp, uint64_t branch_addr )
{
    return ( ( branch_addr >> bp->instShiftAmt ) ^ bp->globalHistory ) &
           ( bp->globalPredictorSize - 1 );
}

// 创建Gshare预测器
GshareBP * gshare_bp_create( unsigned globalPredictorSize,
                             unsigned globalHistoryBits, unsigned instShiftAmt )
{
    if ( ! is_power_of_2( globalPredictorSize ) )
        {
            fprintf( stderr, "Gshare PHT size must be a power of 2!\n" );
            exit( 1 );
        }

    GshareBP * bp = ( GshareBP * ) malloc( sizeof( GshareBP ) );
    if ( ! bp )
        {
            fprintf( stderr, "Failed to allocate GshareBP!\n" );
            exit( 1 );
        }

    bp->globalPredictorSize = globalPredictorSize;
    bp->globalHistoryBits = globalHistoryBits;
    bp->globalHistory = 0;
    bp->historyMask = mask( globalHistoryBits );
    bp->instShiftAmt = instShiftAmt;

    // 动态分配PHT并初始化为弱取（weakly taken）
    bp->pht = ( uint8_t * ) malloc( globalPredictorSize * sizeof( uint8_t ) );
    if ( ! bp->pht )
        {
            fprintf( stderr, "Failed to allocate PHT!\n" );
            free( bp );
            exit( 1 );
        }
    for ( unsigned i = 0; i < globalPredictorSize; i++ )
        {
            bp->pht[i] = 2;    // 初始化为2（弱取）
        }

    return bp;
}

// 查找分支预测
bool gshare_bp_lookup( GshareBP * bp, uint64_t branch_addr, void ** bp_history )
{
    unsigned idx = get_index( bp, branch_addr );
    uint8_t counter = bp->pht[idx];
    bool pred = ( counter >= 2 );    // >=2 表示预测为taken
    *bp_history = malloc( sizeof( unsigned ) );
    if ( *bp_history )
        {
            *( unsigned * ) *bp_history = idx;    // 保存索引以供更新
        }
    return pred;
}

// 更新分支预测
void gshare_bp_update( GshareBP * bp, uint64_t branch_addr, bool taken,
                       void * bp_history, bool squashed )
{
    if ( bp_history )
        {
            unsigned idx = *( unsigned * ) bp_history;
            uint8_t * counter = &bp->pht[idx];
            if ( taken )
                {
                    *counter =
                        ( *counter < 3 ) ? *counter + 1 : 3;    // 饱和计数器增
                }
            else
                {
                    *counter =
                        ( *counter > 0 ) ? *counter - 1 : 0;    // 饱和计数器减
                }
            update_global_history( bp, taken );
            free( bp_history );
        }
}

// 撤销预测
void gshare_bp_squash( GshareBP * bp, void * bp_history )
{
    if ( bp_history )
        {
            free( bp_history );
        }
}

// 销毁Gshare预测器
void gshare_bp_destroy( GshareBP * bp )
{
    if ( bp )
        {
            free( bp->pht );
            free( bp );
        }
}