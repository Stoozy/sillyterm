#pragma once
#include "windows.h"


typedef enum vt_states {

  GROUND,
  OSC_STRING,
  ESCAPE,
  ESC_INTERMEDIATE,
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
  IGN,
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
  VT_STATE state;
} VT100;


VT_STATE vt_execute_handler();
VT_STATE vt_print_handler();
VT_STATE vt_ignore_handler();
VT_STATE vt_clear_handler();

VT_STATE vt_hook_handler();
VT_STATE vt_put_handler();
VT_STATE vt_unhook_handler();

VT_STATE vt_osc_start_handler();
VT_STATE vt_osc_put_handler();
VT_STATE vt_osc_end_handler();

VT_STATE vt_param_handler();
VT_STATE vt_collect_handler();
VT_STATE vt_esc_dispatch_handler();
VT_STATE vt_csi_dispatch_handler();

#ifdef __cplusplus
extern "C" {
#endif

VOID vt_handle_code(UINT32 code);

#ifdef __cplusplus
}
#endif

extern VT100 vt;
