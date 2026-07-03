#pragma once

#include "../../globals.h"

#ifdef FUNCTESTING

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned char TbBool; //redefine rather than include extraneus header info

TbBool ftest_bug_imp_tp_attack_door__claim_init();
TbBool ftest_bug_imp_tp_attack_door__prisoner_init();
TbBool ftest_bug_imp_tp_attack_door__deadbody_init();


#ifdef __cplusplus
}
#endif

#endif // FUNCTESTING
