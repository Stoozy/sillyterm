#pragma once
#include "term.h"
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

typedef enum vt_graphics_rendition {
  // see: https://www.xfree86.org/current/ctlseqs.html
  NORMAL,
  BOLD,
  UNDERLINE,
  ITALIC,
  DIM,
  BLINK,
  NEGATIVE,
  HIDDEN,
  STRIKETHROUGH,

  FG_COLOR_BLACK = 30,
  FG_COLOR_RED = 31,
  FG_COLOR_GREEN = 32,
  FG_COLOR_YELLOW = 33,
  FG_COLOR_BLUE = 34,
  FG_COLOR_MAGENTA  = 35,
  FG_COLOR_CYAN = 36,
  FG_COLOR_WHITE = 37,
  FG_COLOR_DEFAULT = 39,

  BG_COLOR_BLACK = 40,
  BG_COLOR_RED = 41,
  BG_COLOR_GREEN = 42,
  BG_COLOR_YELLOW = 43,
  BG_COLOR_BLUE = 44,
  BG_COLOR_MAGENTA = 45,
  BG_COLOR_CYAN = 46,
  BG_COLOR_WHITE = 47,
  BG_COLOR_DEFAULT = 49
} VT_GR;



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



#define MAX_INTERMEDIATE_CHARS 2
#define MAX_PARAMS 16

typedef struct vt100 {
  wchar_t intermediate_chars[MAX_INTERMEDIATE_CHARS];
  wchar_t code;

  char * param_str;
  UINT32 param_cap;
  UINT32 param_len;

  struct rgb fgColor;
  struct rgb bgColor;

  VT_STATE state;
  VT_GR graphics;

  UINT32 num_params;
} VT100;


void vt_execute_handler(void);
void vt_print_handler(void);
void vt_ignore_handler(void);
void vt_clear_handler(void);

void vt_hook_handler(void);
void vt_put_handler(void);
void vt_unhook_handler(void);

void vt_osc_start_handler(void);
void vt_osc_put_handler(void);
void vt_osc_end_handler(void);

void vt_param_handler(void);
void vt_collect_handler(void);
void vt_esc_dispatch_handler(void);
void vt_csi_dispatch_handler(void);

#ifdef __cplusplus
extern "C" {
#endif

void vt_handle_code(UINT32 code);
void vt_init(void);


#ifdef __cplusplus
}
#endif

extern VT100 vt;
