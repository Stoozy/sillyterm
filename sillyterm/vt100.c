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

    if(ts.cx-1 < 0){
      ts.cy--;
      ts.cx = ts.cols-2;
    }else ts.cx--;

    ts.lines[ts.cy].cells[ts.cx].character = ' ';
    ts.lines[ts.cy].dirty = TRUE;
    return;
  }
}

VT_STATE vt_print_handler(){
  // TODO
  // OutputDebugStringA("vt_print_handler()\n");

  if(ts.cx >= ts.cols){
    ts.cx = 0;
    if(ts.cy < ts.rows)
	ts.cy++;
    else{
      OutputDebugStringA("Ran out of space. Need to scroll!");
      exit(-1);
    }
  }


  if(ts.cx < ts.cols && ts.cy < ts.rows){
    ts.lines[ts.cy].cells[ts.cx++].character = (wchar_t)vt.code;
    ts.lines[ts.cy].dirty = TRUE;
  }
}

VT_STATE vt_ignore_handler(){
  // TODO
  // OutputDebugStringA("vt_ignore_handler()\n");
}

VT_STATE vt_clear_handler(){
  // TODO
  OutputDebugStringA("vt_clear_handler()\n");
  ZeroMemory(vt.param_str, vt.param_cap);
  vt.param_len = 0;
}

VT_STATE vt_hook_handler(){
  // TODO
  // OutputDebugStringA("vt_hook_handler()\n");
}

VT_STATE vt_put_handler(){
  // TODO
  // OutputDebugStringA("vt_put_handler()\n");
}

VT_STATE vt_unhook_handler(){
  // TODO
  // OutputDebugStringA("vt_unhook_handler()\n");
}

VT_STATE vt_osc_start_handler(){
  // TODO
  // OutputDebugStringA("vt_osc_start_handler()\n");
}

VT_STATE vt_osc_put_handler(){
  // TODO
  // OutputDebugStringA("vt_osc_put_handler()\n");
}

VT_STATE vt_osc_end_handler(){
  // TODO
  // OutputDebugStringA("vt_osc_end_handler()\n");
}

VT_STATE vt_param_handler(){
  // TODO
  OutputDebugStringA("vt_param_handler()\n");

  if(vt.param_len ==  vt.param_cap){
    vt.param_str = HeapReAlloc(GetProcessHeap(),
			    0,
			    vt.param_str,
			    2 * vt.param_cap);
    vt.param_cap *= 2;
  }

  vt.param_str[vt.param_len++] = vt.code;
  vt.param_str[vt.param_len+1] = '\0';
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

  // TODO
  OutputDebugStringA("vt_csi_dispatch_handler()\n");
  OutputDebugStringA("PARAM STRING: ");
  OutputDebugStringA(vt.param_str);
  OutputDebugStringA("\n");

  wchar_t msg[2] = {vt.code, 0};
  OutputDebugStringA("Final Character is : ");
  OutputDebugStringA(msg);
  OutputDebugStringA("\n");

  UINT32 params[MAX_PARAMS];
  char * token = strtok(vt.param_str, ";");
  // loop through the string to extract all other tokens

  int param_i = 0;

  OutputDebugStringA("PARAM LIST: \n");
  while( token != NULL ) {
    params[param_i++] = atoi(token);

    OutputDebugStringA(token);
    OutputDebugStringA("\n");
    token = strtok(NULL, " ");
  }

  static int K_counter = 0;
  switch(vt.code){
  case 'K':{
    K_counter++;
    if(param_i == 0) {
      int line = ts.cy;
      for(int i=ts.cx; i<ts.cols; i++)
	ts.lines[line].cells[i].character = ' ';
    }
    break;
  }
  case 'J': {
    // int param = atoi(vt.param_list[0].buf);
    // 0 erases from the current cursor position (inclusive) to the end of the display

    // 1 erases from the beginning of the display up to
    // and including the current cursor position

    // 2 erases the entire display

    UINT32 param = 0;

    if(param_i == 0) break;
    else  param = params[0];
    // TODO
    switch(param){
      // case  0: break;

    case  1: break;
    case  2:{
      // int line = ts.cy;
      // for(int i=0; i < ts.cols; i++){
      // 	TerminalCharacter ** screen = ts.screen;
      // 	screen[ts.cy][i].character = ' ';
      // }
      break;
    }
      // default: {
      //   // clear until end of current line
      //   for(int i=ts.cx; i<ts.cols; i++)
      // 	ts.screen[ts.cy][i].character = ' ';

      //   for(int i=ts.cy; i<ts.lines; i++){
      // 	for(int j=0; j<ts.cols; j++){
      // 	  ts.screen[i][j].character = ' ';
      // 	}
      //   }


      //   break;
      // }
    }

    break;
  }
  case 'H' : {
    if(param_i == 0){
      ts.cx = ts.cy = 0;
    }

    if(param_i == 2){
      UINT32 y = params[0];
      UINT32 x = params[1];
      if(y < ts.rows && x < ts.cols && x >=0 && y>=0){
	ts.cx = x-1;
	ts.cy = y-1;
      }
    }

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

  // vt_print_state(oldState);

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

  // initialize param_str

  vt.param_cap = 64;
  vt.param_len = 0;
  vt.param_str = HeapAlloc(GetProcessHeap(), 0, vt.param_cap);

}
