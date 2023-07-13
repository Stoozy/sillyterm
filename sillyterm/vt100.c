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


static VT_STATE handle_from_anywhere(UINT32 code){
  // TO GROUND
  if(code == 0x18 || code == 0x1a || inRange(code, 0x80, 0x8f)
     || inRange(code, 0x91, 0x97) || code == 0x9a || code == 0x9c)
    return GROUND;

  if(code == 0x98 || code == 0x9e || code == 0x9f) return SOS_PM_APC_STRING;
  if(code == 0x1b){
    vt_clear_handler();
    return ESCAPE;
  }

  if(code == 0x90) {
    vt_clear_handler();
    return DCS_ENTRY;
  }

  if(code == 0x9b){
    vt_clear_handler();
    return CSI_ENTRY;
  }

  if(code == 0x9d){
    vt_osc_start_handler();
    return OSC_STRING;
  }

  return NIL;
}

VOID vt_handle_code(UINT32 code){
  VT_STATE oldState = vt.state;

  VT_STATE potentialState = handle_from_anywhere(code);
  if(potentialState != NIL) vt.state = potentialState; return;


  switch(oldState){
  case GROUND:{

    // event / execute
    if(inRange(code, 0x0, 0x17) || code == 0x19 || inRange(code, 0x1c, 0x1f))
      vt_execute_handler();

    // event / print
    if(inRange(code, 0x20, 0x7f)) vt_print_handler();

    break;
  }
  case OSC_STRING:{
    if(code == 0x9c) {
      vt_osc_end_handler();
      vt.state = GROUND;
    }

    if(inRange(code, 0x20, 0x7f)) vt_osc_put_handler();

    break;
  }
  case ESCAPE:{

    // entry / clear handled by handle_from_anywhere()

    if(inRange(code, 0x0, 0x17) || code == 0x19 || inRange(code, 0x1c, 0x1f))
      vt_execute_handler();

    if(code == 0x58 || code == 0x5e || code == 0x5f)
      vt.state = SOS_PM_APC_STRING;

    if(code == 0x50) vt.state = DCS_ENTRY;
    if(code == 0x5b) vt.state = CSI_ENTRY;
    if(code == 0x5d) vt.state = OSC_STRING;

    if(inRange(code, 0x20, 0x2f)){
      vt_collect_handler();
      vt.state = ESCAPE_INTERMEDIATE;
    }

    // ground
    if(inRange(code, 0x30, 0x4f) || inRange(code, 0x51, 0x57)
       || code == 0x59 || code == 0x5a || code == 0x5c
       || inRange(code, 0x60, 0x7e))
      vt.state = GROUND;

    // if(code == 0x7f) do nothing
    break;
  }
  case ESCAPE_INTERMEDIATE:{


    // execute
    if(inRange(code, 0x0, 0x17) || code == 0x19 || inRange(code, 0x1c, 0x1f))
      vt_execute_handler();

    // collect
    if(inRange(code, 0x20, 0x2f))
      vt_collect_handler();

    // esc_dispatch
    if(inRange(code, 0x30, 0x7e)){
      vt_esc_dispatch_handler();
      vt.state = GROUND;
    }

    // if(code == 0x7f) ignore

    break;
  }
  case CSI_PARAM: {
    // execute
    if(inRange(code, 0x0, 0x17) || code == 0x19 || inRange(code, 0x1c, 0x1f))
      vt_execute_handler();

    // param
    if(inRange(code, 0x30, 0x39) || code == 0x3b)
      vt_param_handler();

    // state changes
    if(inRange(code, 0x40, 0x7e)) {
      vt_csi_dispatch_handler();
      vt.state = GROUND;
    }

    if(inRange(code, 0x20, 0x2f)){
      vt_collect_handler();
      vt.state = CSI_INTERMEDIATE;
    }

    if(code == 0x3a || inRange(code, 0x3c, 0x3f))
      vt.state = CSI_IGNORE;

    // if (code ==0x7f) ignore

    break;
  }
  case CSI_IGNORE: {
    // execute
    if(inRange(code, 0x0, 0x17) || code == 0x19 || inRange(code, 0x1c, 0x1f))
      vt_execute_handler();

    // state changes
    if(inRange(code, 0x40, 0x7e))
      vt.state = GROUND;

    break;
  }
  case CSI_ENTRY: {
    // execute
    if(inRange(code, 0x0, 0x17) || code == 0x19 || inRange(code, 0x1c, 0x1f))
      vt_execute_handler();

    // if(code == 0x7f) do nothing

    // state changes
    if(inRange(code, 0x30, 0x39) ||  inRange(code, 0x3b, 0x3f)){

      if(inRange(code, 0x3c, 0x3f)) vt_collect_handler();
      else vt_param_handler();

      vt.state = CSI_PARAM;
    }

    if(code == 0x3a) vt.state = CSI_IGNORE;

    if(inRange(code, 0x20, 0x2f)){
      vt_collect_handler();
      vt.state = CSI_INTERMEDIATE;
    }

    if(inRange(code, 0x40, 0x7e)){
      vt_csi_dispatch_handler();
      vt.state = GROUND;
    }

    break;
  }
  case CSI_INTERMEDIATE: {
    // execute
    if(inRange(code, 0x0, 0x17) || code == 0x19 || inRange(code, 0x1c, 0x1f))
      vt_execute_handler();

    if(inRange(code, 0x20, 0x2f)){
      vt_collect_handler();
    }

    // if(code == 0x7f) do nothing

    // state changes
    if(inRange(code, 0x30, 0x3f)) vt.state = CSI_IGNORE;

    if(inRange(code, 0x40, 0x7e)) {
      vt_csi_dispatch_handler();
      vt.state = GROUND;
    }

    break;
  }
  case DCS_ENTRY: {
    // execute
    if(inRange(code, 0x0, 0x17) || code == 0x19 || inRange(code, 0x1c, 0x1f))
      vt_execute_handler();

    // if(code == 0x7f) do nothing

    // state changes
    if(inRange(code, 0x20, 0x2f)){
      vt_collect_handler();
      vt.state = DCS_INTERMEDIATE;
    }

    if(inRange(code, 0x3b, 0x3f) || inRange(code, 0x30, 0x39)){

      if(inRange(code, 0x3c, 0x3f)) vt_collect_handler();
      else vt_param_handler();

      vt.state = DCS_PARAM;
    }

    if(code == 0x3a) vt.state = DCS_IGNORE;
    if(inRange(code, 0x40, 0x7e)){
      vt_hook_handler(); // entry
      vt.state = DCS_PASSTHROUGH;
    }

    break;
  }
  case DCS_INTERMEDIATE: {

    // do nothing
    // if(inRange(code, 0x0, 0x17) || code == 0x19 || inRange(code, 0x1c, 0x1f))
    // if(code == 0x7f)

    if(inRange(code, 0x20, 0x2f))
      vt_collect_handler();

    // state changes
    if(inRange(code, 0x30, 0x3f)) vt.state = DCS_IGNORE;
    if(inRange(code, 0x40, 0x7e)){
      vt_hook_handler(); // entry
      vt.state = DCS_PASTHROUGH;
    }


    break;
  }
  case DCS_IGNORE: {

    // do nothing
    // if(inRange(code, 0x0, 0x17) || code == 0x19 || inRange(code, 0x1c, 0x1f))

    if(code == 0x9c) vt.state = GROUND;
    break;
  }
  case DCS_PARAM: {
    // do nothing
    // if(inRange(code, 0x0, 0x17) || code == 0x19 || inRange(code, 0x1c, 0x1f))
    // if(code == 0x7f)

    if(inRange(code, 0x30, 0x39) || code == 0x3b)
      vt_param_handler();

    // state changes
    if(code == 0x3a || inRange(code, 0x3c, 0x3f)) vt.state = DCS_IGNORE;

    if(inRange(code, 0x20, 0x2f)){
      vt_collect_handler();
      vt.state = DCS_INTERMEDIATE;
    }

    if(inRange(code, 0x40, 0x7e)) {
      vt_hook_handler(); // entry
      vt.state = DCS_PASSTHROUGH;
    }

    break;
  }
  case DCS_PASSTHROUGH: {
    // execute
    if(inRange(code, 0x0, 0x17) || code == 0x19 || inRange(code, 0x1c, 0x1f)
       || inRange(code, 0x20, 0x7e))
      vt_put_handler();

    // if(code == 0x7f) do nothing

    if(code == 0x9c){
      vt_unhook_handler(); // exit
      vt.state = GROUND;
    }
    break;
  }
  default:
    break;
  }

}
