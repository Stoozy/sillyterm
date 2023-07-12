#include "vt100.h"
#include "windows.h"


VT100 vt;

BOOL inRange(UINT32 val, UINT32 lower, UINT32 upper){
    return val >= lower && val <= upper;
}

VT_STATE vt_execute_handler(){
    // TODO 
}

VT_STATE vt_print_handler(){
    // TODO 
}

VT_STATE vt_ignore_handler(){
    // TODO 
}

VT_STATE vt_clear_handler(){
    // TODO 
}

VT_STATE vt_hook_handler(){
    // TODO 
}

VT_STATE vt_put_handler(){
    // TODO 
}

VT_STATE vt_unhook_handler(){
    // TODO 
}

VT_STATE vt_osc_start_handler(){
    // TODO 
}

VT_STATE vt_osc_put_handler(){
    // TODO 
}

VT_STATE vt_osc_end_handler(){
    // TODO 
}

VT_STATE vt_param_handler(){
    // TODO 
}

VT_STATE vt_collect_handler(){
    // TODO 
}

VT_STATE vt_esc_dispatch_handler(){
    // TODO 
}

VT_STATE vt_csi_dispatch_handler(){
    // TODO 
}


static BOOL handle_from_anywhere(UINT32 code){
  // TO GROUND
  if(code == 0x18 || code == 0x1a || inRange(code, 0x80, 0x8f)
     || inRange(code, 0x91, 0x97) || code == 0x9a || code == 0x9c) {
    vt.state = GROUND;
    return TRUE;
  }

  if(code == 0x1b){
    vt.state = ESCAPE;
    return TRUE;
  }


  return FALSE;
}

VOID vt_handle_code(UINT32 code){
  VT_STATE oldState = vt.state;


    
  if(handle_from_anywhere(code))
    return;

  switch(oldState){
  case GROUND:{

    if(inRange(code, 0x0, 0x17) || code == 0x19
       || inRange(code, 0x1c, 0x1f))
      vt_execute_handler();

    if(inRange(code, 0x20, 0x7f))
      vt_print_handler();

    break;
  }
  case OSC_STRING:
    break;
  case ESCAPE:
    break;
  case ESC_INTERMEDIATE:
    break;
  case CSI_PARAM: 
    break;
  case CSI_IGNORE: 
    break;
  case CSI_ENTRY: 
    break;
  case CSI_INTERMEDIATE: 
    break;
  case DCS_ENTRY: 
    break;
  case DCS_INTERMEDIATE: 
    break;
  case DCS_IGNORE: 
    break;
  case DCS_PARAM: 
    break;
  case DCS_PASSTHROUGH: 
    break;
  default:
    break;
  }

}
