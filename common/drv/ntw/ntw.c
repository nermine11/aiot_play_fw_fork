#include <string.h>
#include "ntw.h"

//=========================== define ==========================================

//=========================== variables =======================================

typedef struct {
    uint16_t        dummy;
} ntw_vars_t;

ntw_vars_t ntw_vars;

typedef struct {
    uint32_t        dummy;
} ntw_dbg_t;

ntw_dbg_t ntw_dbg;

//=========================== prototypes ======================================

//=========================== public ==========================================

void ntw_init(void) {

    // reset variables
    memset(&ntw_vars,0x00,sizeof(ntw_vars_t));
    memset(&ntw_dbg, 0x00,sizeof(ntw_dbg_t));
}

//=========================== private =========================================

//=========================== interrupt handlers ==============================
