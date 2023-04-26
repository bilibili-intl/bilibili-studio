#ifndef _BEF_EFFECT_AI_AUTHMSG_H_
#define _BEF_EFFECT_AI_AUTHMSG_H_


#include "bef_effect_ai_public_define.h"

BEF_SDK_API bef_effect_result_t bef_effect_ai_get_auth_msg( char** buf, int* len);                                                              


BEF_SDK_API bef_effect_result_t bef_effect_ai_free_msg_buf(char* buf);     

#endif // _BEF_EFFECT_AI_AUTHMSG_H_