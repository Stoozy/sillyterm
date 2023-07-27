#include "ansi.h"
#include "windows.h"
#include "term.h"
#include "renderer.h"
#include <string.h>

VT100 vt;

BOOL inRange(UINT32 val, UINT32 lower, UINT32 upper){
    return val >= lower && val <= upper;
}

void vt_execute_handler(){
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

void vt_print_handler(){
  // TODO
  OutputDebugStringA("vt_print_handler()\n");

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
    ts.lines[ts.cy].dirty = TRUE;
    TCELL * cell = &ts.lines[ts.cy].cells[ts.cx++];
    cell->character = (wchar_t)vt.code;
    cell->fgColor = vt.fgColor;
    cell->bgColor = vt.bgColor;
  }
}

void vt_ignore_handler(){
  // TODO
  // OutputDebugStringA("vt_ignore_handler()\n");
}

void vt_clear_handler(){
  // TODO
  OutputDebugStringA("vt_clear_handler()\n");
  ZeroMemory(vt.param_str, vt.param_cap);
  vt.param_len = 0;
}

void vt_hook_handler(){
  // TODO
  // OutputDebugStringA("vt_hook_handler()\n");
}

void vt_put_handler(){
  // TODO
  // OutputDebugStringA("vt_put_handler()\n");
}

void vt_unhook_handler(){
  // TODO
  // OutputDebugStringA("vt_unhook_handler()\n");
}

void vt_osc_start_handler(){
  // TODO
  // OutputDebugStringA("vt_osc_start_handler()\n");
}

void vt_osc_put_handler(){
  // TODO
  // OutputDebugStringA("vt_osc_put_handler()\n");
}

void vt_osc_end_handler(){
  // TODO
  // OutputDebugStringA("vt_osc_end_handler()\n");
}

void vt_param_handler(){
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

void vt_collect_handler(){
  // TODO
  OutputDebugStringA("vt_collect_handler()\n");
}

void vt_esc_dispatch_handler(){
  // TODO
  OutputDebugStringA("vt_esc_dispatch_handler()\n");
}



void vt_csi_dispatch_handler(){

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
    token = strtok(NULL, ";");
  }


  switch(vt.code){
  case 'A':{
    if(param_i == 1){
      ts.cy-=params[0];
      if(ts.cy < 0) ts.cy = 0;
    }
  } break;
  case 'B':{
    if(param_i == 1){
      ts.cy+=params[0];
      if(ts.cy > ts.rows) ts.cy = ts.rows-1;
    }
  } break;
  case 'C':{
    if(param_i == 1){
      ts.cx += params[0];
      if(ts.cx > ts.cols) ts.cx = ts.cols-1;
    }
  } break;
  case 'D': {
    if(param_i == 1){
      ts.cx -= params[0];
      if(ts.cx < 0) ts.cx = 0;
    }
  } break;
  case 'E':{
    if(param_i == 1){
      ts.cy+=params[0];
      if(ts.cy > ts.rows) ts.cy = ts.rows-1;
      ts.cx = 0;
    }
  } break;
  case 'F':{
    if(param_i == 1){
      ts.cy-=params[0];
      if(ts.cy < 0) ts.cy = 0;
      ts.cx = 0;
    }
  } break;
  case 'G':{
    if(param_i == 1){
      ts.cx = params[0];
    }
  } break;
  case 'l': {
    if(param_i == 1 && params[0] == 25){
      ts.showCursor = FALSE;
    }
  } break;

  case 'K':{
    if(param_i == 0 || (param_i == 1 && params[0] == 0)) {
      int line = ts.cy;
      for(int i=ts.cx; i<ts.cols; i++)
	ts.lines[line].cells[i].character = ' ';
    }

    switch(params[0]){
    case 1:{
      for(UINT32 i = 0 ; i < ts.cx; i++)
	ts.lines[ts.cy].cells[i].character = ' ';
    } break;
    case 2:{
      for(UINT32 i = 0; i<ts.cols; i++)
	ts.lines[ts.cy].cells[i].character = ' ';
    } break;
    }
  } break;

  case 'J': {
    UINT32 param = 0;

    if(param_i == 0 || (param_i == 1 && params[0] == 0))
      for(UINT32 i = ts.cx; i < ts.cols; i++)
	for(UINT32 j = ts.cy; j < ts.rows; j++)
	  ts.lines[j].cells[i].character = ' ';


    else  param = params[0];
    // TODO
    switch(param){
    case  1: {
      for(UINT32 i = ts.cx; i > 0; i--)
	for(UINT32 j = ts.cy; j > 0; j--)
	  ts.lines[j].cells[i].character = ' ';
    } break;
    case  2: {
	for(UINT32 i = 0; i < ts.cols; i++)
	  for(UINT32 j = 0; j < ts.rows; j++)
	    ts.lines[j].cells[i].character = ' ';
    } break;
    case 3: break; // TODO: erase saved lines... whatever that means
    }

    
  } break;
  case 'H' : {
    // reset cursor to home (0,0)
    if(param_i == 0) ts.cx = ts.cy = 0;

    if(param_i == 2){
      UINT32 y = params[0];
      UINT32 x = params[1];
      if(y < ts.rows && x < ts.cols && x >=0 && y>=0){
	ts.cx = x-1;
	ts.cy = y-1;
      }
    }


  } break;

  case 'h':{
    if(param_i == 1 && params[0] == 25)
      ts.showCursor = TRUE;
  } break;
    
  case 'm':{
    // https://gist.github.com/fnky/458719343aabd01cfb17a3a4f7296797

    // reset everything
    if(param_i == 0 ){
      vt.graphics = NORMAL;
      vt.fgColor = colors[15];
      vt.bgColor = colors[0];
    }

    if(param_i == 1){
      // TODO
      switch(params[0]){
      case 0: {
	vt.graphics = NORMAL; 
	vt.fgColor = colors[15];
	vt.bgColor = colors[0];
      } break; // reset everything
      case 1: vt.graphics |= BOLD; break; // set bold mode 
      case 2: vt.graphics |= DIM; break; // set dim mode 
      case 3: vt.graphics |= ITALIC; break; // set italic mode 
      case 4: vt.graphics |= UNDERLINE; break; // set underline mode 
      case 5: vt.graphics |= BLINK; break; // set blinking mode 
      case 7: vt.graphics |= NEGATIVE; break; // set negative mode
      case 8: vt.graphics |= HIDDEN; break; // set hidden mode
      case 9: vt.graphics |= STRIKETHROUGH; break; // set strikethrough mode

      case 22: vt.graphics &= ~(BOLD | DIM); break; // reset bold and dim mode
      case 23: vt.graphics &= ~(ITALIC); break; // reset italic mode
      case 24: vt.graphics &= ~(UNDERLINE); break; // reset underline mode
      case 25: vt.graphics &= ~(BLINK); break; // reset blinking mode
      case 27: vt.graphics &= ~(NEGATIVE); break; // reset negative mode
      case 28: vt.graphics &= ~(HIDDEN); break; // reset hidden mode
      case 29: vt.graphics &= ~(STRIKETHROUGH); break; // reset strikethrough mode


      case 30: vt.fgColor = colors[0];  break;
      case 31: vt.fgColor = colors[9];  break; 
      case 32: vt.fgColor = colors[10]; break; 
      case 33: vt.fgColor = colors[11]; break; 
      case 34: vt.fgColor = colors[12]; break; 
      case 35: vt.fgColor = colors[13]; break; 
      case 36: vt.fgColor = colors[14]; break; 
      case 37: vt.fgColor = colors[15]; break; 

      case 40: vt.bgColor = colors[0]; break;
      case 41: vt.bgColor = colors[1]; break; 
      case 42: vt.bgColor = colors[2]; break; 
      case 43: vt.bgColor = colors[3]; break; 
      case 44: vt.bgColor = colors[4]; break; 
      case 45: vt.bgColor = colors[5]; break; 
      case 46: vt.bgColor = colors[6]; break; 
      case 47: vt.bgColor = colors[7]; break; 

      default: break;
      }

    }else if(param_i == 3){
      struct rgb color = colors[params[2]];

      if(params[0] == 48 && params[1] == 5)
	vt.fgColor = color;

      if(params[0] == 38 && params[1] == 5)
	vt.bgColor = color;
    }
    


  } break;

  default: break;
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

  vt.fgColor = colors[15];
  vt.bgColor = colors[0];
}
