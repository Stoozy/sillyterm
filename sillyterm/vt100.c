#include "vt100.h"
#include "windows.h"
#include <string.h>

VT100 vt;

BOOL inRange(UINT32 val, UINT32 lower, UINT32 upper){
    return val >= lower && val <= upper;
}

VT_STATE vt_execute_handler(){
  // TODO
  OutputDebugStringA("vt_execute_handler()\n");
  if(vt.code == '\b'){

    if(terminalState.cx-1 < 0){
      terminalState.cy--;
      terminalState.cx = terminalState.cols-1;
    }else terminalState.cx--;

    terminalState.screen[terminalState.cy][terminalState.cx].character = ' ';
    return;
  }
}

VT_STATE vt_print_handler(){
  // TODO
  OutputDebugStringA("vt_print_handler()\n");

  if(terminalState.cx >= terminalState.cols){
    terminalState.cx = 0;
    terminalState.cy++;
  }

  if(terminalState.cx < terminalState.cols && terminalState.cy < terminalState.lines)
    terminalState.screen[terminalState.cy][terminalState.cx++].character = (wchar_t)vt.code;

}

VT_STATE vt_ignore_handler(){
  // TODO
  OutputDebugStringA("vt_ignore_handler()\n");
}

VT_STATE vt_clear_handler(){
  // TODO
  OutputDebugStringA("vt_clear_handler()\n");
  for(int i=0; i<vt.num_params; i++){
    ZeroMemory(vt.param_list[i].buf, sizeof(wchar_t) * vt.param_list[i].cap);
    vt.param_list[i].len = 0;
  }

  vt.num_params = 0;
}

VT_STATE vt_hook_handler(){
  // TODO
  OutputDebugStringA("vt_hook_handler()\n");
}

VT_STATE vt_put_handler(){
  // TODO
  OutputDebugStringA("vt_put_handler()\n");
}

VT_STATE vt_unhook_handler(){
  // TODO
  OutputDebugStringA("vt_unhook_handler()\n");
}

VT_STATE vt_osc_start_handler(){
  // TODO
  OutputDebugStringA("vt_osc_start_handler()\n");
}

VT_STATE vt_osc_put_handler(){
  // TODO
  OutputDebugStringA("vt_osc_put_handler()\n");
}

VT_STATE vt_osc_end_handler(){
  // TODO
  OutputDebugStringA("vt_osc_end_handler()\n");
}

VT_STATE vt_param_handler(){
  // TODO
  OutputDebugStringA("vt_param_handler()\n");

  VT_PARAM * param = &vt.param_list[vt.num_params];

  // move on to next param in case of ";"
  if(vt.code == 0x3b){
    vt.num_params++;
    return NIL;
  }

  if(vt.num_params == 0)
    vt.num_params = 1;

  if(param->len ==  param->cap){
    param->buf = HeapReAlloc(GetProcessHeap(),
			    0,
			    param->buf,
			    2 * param->cap * sizeof(wchar_t));
    param->cap *= 2;
  }

  if(param->buf){
    param->buf[param->len++] = vt.code;
    param->buf[param->len+1] = 0;
  }
  else{
    OutputDebugStringA("Param buffer is null :c\n");
    exit(-1);
  }


  wchar_t msg[2] = {vt.code, 0};
  OutputDebugStringA(msg);
  OutputDebugStringA("\n");
}

VT_STATE vt_collect_handler(){
  // TODO
  OutputDebugStringA("vt_collect_handler()\n");
}

VT_STATE vt_esc_dispatch_handler(){
  // TODO
  OutputDebugStringA("vt_esc_dispatch_handler()\n");
}

VT_STATE vt_csi_dispatch_handler(){

  vt.num_params++;
  // TODO
  OutputDebugStringA("vt_csi_dispatch_handler()\n");
  OutputDebugStringA("PARAM LIST: \n");
  for(int i=0; i<vt.num_params; i++){
    OutputDebugStringA(vt.param_list[i].buf);
    OutputDebugStringA("\n");
  }

  wchar_t msg[2] = {vt.code, 0};
  OutputDebugStringA("Final Character is : ");
  OutputDebugStringA(msg);
  OutputDebugStringA("\n");

  switch(vt.code){
  case 'J': {
    int param = atoi(vt.param_list[0].buf);
    // 0 erases from the current cursor position (inclusive) to the end of the line/display

    // 1 erases from the beginning of the line/display up to
    // and including the current cursor position

    // 2 erases the entire line/display


    // TODO
    switch(param){
    case  0: break;
    case  1: break;
    case  2:{
      int line = terminalState.cy;
      for(int i=0; i < terminalState.cols; i++){
	TerminalCharacter ** screen = terminalState.screen;
	screen[terminalState.cy][i].character = ' ';
      }
      break;
    }
    }
    break;
  }
  case 'H' : {
    int y = atoi(vt.param_list[0].buf);
    int x = atoi(vt.param_list[1].buf);

    terminalState.cx = x;
    terminalState.cy = y;

    break;
  }
  }
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

static VOID vt_print_state(VT_STATE state){
  switch(state){
  case NIL: OutputDebugStringA("NIL\n"); break;
  case GROUND: OutputDebugStringA("GROUND\n"); break;
  case SOS_PM_APC_STRING: OutputDebugStringA("SOS_PM_APC_STRING\n"); break;
  case OSC_STRING: OutputDebugStringA("OSC_STRING\n"); break;
  case ESCAPE: OutputDebugStringA("ESCAPE\n"); break;
  case ESCAPE_INTERMEDIATE: OutputDebugStringA("ESCAPE_INTERMEDIATE\n"); break;
  case CSI_PARAM: OutputDebugStringA("CSI_PARAM\n"); break;
  case CSI_IGNORE: OutputDebugStringA("CSI_IGNORE\n"); break;
  case CSI_ENTRY: OutputDebugStringA("CSI_ENTRY\n"); break;
  case CSI_INTERMEDIATE: OutputDebugStringA("CSI_INTERMEDIATE\n"); break;
  case DCS_ENTRY: OutputDebugStringA("DCS_ENTRY\n"); break;
  case DCS_INTERMEDIATE: OutputDebugStringA("DCS_INTERMEDIATE\n"); break;
  case DCS_IGNORE: OutputDebugStringA("DCS_IGNORE\n"); break;
  case DCS_PARAM: OutputDebugStringA("DCS_PARAM\n"); break;
  case DCS_PASSTHROUGH: OutputDebugStringA("DCS_PASSTHROUGH\n"); break;
  }
}

VOID vt_handle_code(UINT32 code){
  vt.code = code;
  VT_STATE oldState = vt.state;

  vt_print_state(oldState);

  VT_STATE potentialState = handle_from_anywhere(code);
  if(potentialState != NIL) {
   vt.state = potentialState; return;
  }


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
      vt.state = DCS_PASSTHROUGH;
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
    OutputDebugStringA("vt_handle_code(): Unknown code!\n");
    break;
  }

}

VOID vt_init(){
  ZeroMemory(&vt,sizeof(VT100));

  // initialize param list
  for(UINT32 i=0; i<MAX_PARAMS; i++){
    vt.param_list[i].cap = 64;
    vt.param_list[i].len = 0;
    vt.param_list[i].buf = (wchar_t*)HeapAlloc(GetProcessHeap(),
					       HEAP_ZERO_MEMORY,
					       64 * sizeof(wchar_t));
  }

}
