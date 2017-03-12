#pragma once

// #define SKIP_INTRO

//#define MEGAPETSCII_DEBUG
//#define MEGAPETSCII_DEBUG_AREA 5
//#define DEBUG_POS 4
//#define MEGAPETSCII_DEBUG_SHOW_FILE

//#define MEGAPETSCII_DEBUG_AREA 1
//#define DEBUG_POS 0

//#define BORDER_DEBUG

#define MANUAL_JOYSTICK_CONTROL


#define MEGAPETSCII_DEBUG_AREA 0

#ifdef MEGAPETSCII_DEBUG
value_t colbuf_l = 0x0400 + 40 * 0;
value_t colbuf_h = 0x0400 + 40 * 1;
value_t colbuf_cur_l = 0x0400 + 40 * 2;
value_t colbuf_cur_h = 0x0400 + 40 * 3;
#else
value_t colbuf_l = 0x0800 + 40 * 0;
value_t colbuf_h = 0x0800 + 40 * 1;
value_t colbuf_cur_l = 0x0800 + 40 * 2;
value_t colbuf_cur_h = 0x0800 + 40 * 3;
#endif





value_t krill_load = 0x900;
value_t krill_load_packed =  krill_load + 3;

value_t music_start = 0xf00;
value_t music_init = music_start + 0;
value_t music_play = music_start + 3;
value_t music_vol = music_start + 6;
value_t actual_start = 0x3300;
value_t msdos_start = 0x4E00; // this should be same as GFX_BUFFER_START

value_t cumvm_start = 0xe000;
value_t cumvm_init = cumvm_start + 0;
value_t cumvm_play = cumvm_start + 3;

value_t GFX_BUFFER_START = 0x4E00;


value_t framecounter = 0x02; // size 2

value_t cumctx_head = 0x04; // siz 2
value_t cumctx_nextthink = 0x06; // siz 2

value_t text_pos_screen = 0x08; // size 2
value_t text_color = 0x0A;


// !! THIS GROWS BACKWARDS FROM gfx buffers
value_t text_colbuf = GFX_BUFFER_START - 40;


	value_t zpbase = 0x0B; // siz 2

	value_t st1 = 0x0D; // siz 2
	value_t st2 = 0x0F; // siz 2
	value_t st3 = 0x11; // siz 2
	value_t st4 = 0x13; // siz 2
	value_t twister_vel = 0x15; // siz 2


value_t scrollpos = 0x17; // siz 3
value_t scrollpos_v = 0x1A; // siz 3
value_t scrollpos_a = 0x1D; // siz 3

value_t refresh_cur_column = 0x20;

value_t dbg_reset_flag = 0x21;


value_t scrolltmp = 0x22; // siz 2
value_t scrolltmp2 = 0x24; // siz 2

value_t loading_bundle = 0x26;

value_t lastpressed = 0x27;

//value_t vsp_pos = 0x28;
value_t d016_val = 0x28;

value_t tmp_col_l = 0x29;
value_t tmp_col_h = 0x2A;

value_t scrollmode = 0x2B;

value_t twister_dist = 0x2C; // siz 2 SAME AS TMP_COL_L !!!!
value_t twister_temp = 0x2E; // siz 2 SAME AS TMP_COL_L !!!!
value_t twister_target = 0x30; // siz 2 SAME AS TMP_COL_L !!!!
value_t twister_pos = 0x32;

value_t sprite_iterator_temp = 0x33;
value_t gh_d010 = 0x34;
value_t gh_d015 = 0x35;

value_t sprite_x = 0x36; // siz 2
value_t sprite_y = 0x38; // siz 2