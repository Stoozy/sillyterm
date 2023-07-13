#pragma once
#include "windows.h"


typedef enum vt_states {
  NIL,
  GROUND,
  SOS_PM_APC_STRING,
  OSC_STRING,
  ESCAPE,
  ESCAPE_INTERMEDIATE,
  CSI_PARAM,
  CSI_IGNORE,
  CSI_ENTRY,
  CSI_INTERMEDIATE,
  DCS_ENTRY,
  DCS_INTERMEDIATE,
  DCS_IGNORE,
  DCS_PARAM,
  DCS_PASSTHROUGH,

} VT_STATE;


typedef enum vt_events {

  EXECUTE,
  PRINT,
  _IGNORE,
  CLEAR,
  COLLECT,
  PARAM,
  ESC_DISPATCH,
  CSI_DISPATCH,
  HOOK,
  UNHOOK,
  PUT,
  OSC_START,
  OSC_PUT,
  OSC_END,

} VT_EVENT;


typedef struct vt100 {
  wchar_t code;
  VT_STATE state;
} VT100;


VT_STATE vt_execute_handler(void);
VT_STATE vt_print_handler(void);
VT_STATE vt_ignore_handler(void);
VT_STATE vt_clear_handler(void);

VT_STATE vt_hook_handler(void);
VT_STATE vt_put_handler(void);
VT_STATE vt_unhook_handler(void);

VT_STATE vt_osc_start_handler(void);
VT_STATE vt_osc_put_handler(void);
VT_STATE vt_osc_end_handler(void);

VT_STATE vt_param_handler(void);
VT_STATE vt_collect_handler(void);
VT_STATE vt_esc_dispatch_handler(void);
VT_STATE vt_csi_dispatch_handler(void);

#ifdef __cplusplus
extern "C" {
#endif

VOID vt_handle_code(UINT32 code);

#ifdef __cplusplus
}
#endif

extern VT100 vt;
