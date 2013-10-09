#include <stdio.h>
#include <fileio.h>
#include <string.h>
#include <libjpg.h>

//FCEUltra headers
#include "../../driver.h"
#include "../../fceu-types.h"

#include "ps2fceu.h"
extern unsigned char path[4096];
extern vars Settings;
extern skin FCEUSkin;
extern u8 partitions[2];
char mpartitions[4][256];
int FONT_HEIGHT = 16;
/************************************/
/* FCEUltra Variables               */
/************************************/
#ifdef SOUND_ON
  #include <audsrv.h>
  #define SAMPLERATE 44100
#else
  #define SAMPLERATE 0
#endif
FCEUGI *CurGame=NULL;
const char * GetKeyboard(void) {
	return "";
}
int PPUViewScanline = 0;
int PPUViewer = 0;
int UpdatePPUView = 0;

//static unsigned int totallines, srendline, erendline;
/************************************/
/* gsKit Variables                  */
/************************************/
GSTEXTURE NES_TEX;
GSTEXTURE BG_TEX;
GSTEXTURE MENU_TEX;
GSFONTM *gsFontM;

u8 menutex = 0;
u8 bgtex = 0;

extern GSGLOBAL *gsGlobal;
//unsigned int ps2palette[256];
/* normal loopy palette
u32 NesPalette[ 64 ] =
{
      0x757575, 0x271b8f, 0x0000ab, 0x47009f, 0x8f0077, 0xab0013, 0xa70000, 0x7f0b00,
      0x432f00, 0x004700, 0x005100, 0x003f17, 0x1b3f5f, 0x000000, 0x000000, 0x000000,
      0xbcbcbc, 0x0073ef, 0x233bef, 0x8300f3, 0xbf00bf, 0xe7005b, 0xdb2b00, 0xcb4f0f,
      0x8b7300, 0x009700, 0x00ab00, 0x00933b, 0x00838b, 0x000000, 0x000000, 0x000000,
      0xffffff, 0x3fbfff, 0x5f97ff, 0xa78bfd, 0xf77bff, 0xff77b7, 0xff7763, 0xff9b3b,
      0xf3bf3f, 0x83d313, 0x4fdf4b, 0x58f898, 0x00ebdb, 0x000000, 0x000000, 0x000000,
	0xffffff, 0xabe7ff, 0xc7d7ff, 0xd7cbff, 0xffc7ff, 0xffc7db, 0xffbfb3, 0xffdbab,
	0xffe7a3, 0xe3ffa3, 0xabf3bf, 0xb3ffcf, 0x9ffff3, 0x000000, 0x000000, 0x000000
};*/
/*
u32 NesPalette[ 64 ] = //modified palette for GS
{
      0x757575, 0x271b8f, 0x0000ab, 0x47009f, 0x8f0077, 0xab0013, 0xa70000, 0x7f0b00, //1 start
	0xbcbcbc, 0x0073ef, 0x233bef, 0x8300f3, 0xbf00bf, 0xe7005b, 0xdb2b00, 0xcb4f0f, //2 start
	0x432f00, 0x004700, 0x005100, 0x003f17, 0x1b3f5f, 0x000000, 0x000000, 0x000000, //1 finish
	0x8b7300, 0x009700, 0x00ab00, 0x00933b, 0x00838b, 0x000000, 0x000000, 0x000000, //2 finish
	0xffffff, 0x3fbfff, 0x5f97ff, 0xa78bfd, 0xf77bff, 0xff77b7, 0xff7763, 0xff9b3b, //3 start
	0xffffff, 0xabe7ff, 0xc7d7ff, 0xd7cbff, 0xffc7ff, 0xffc7db, 0xffbfb3, 0xffdbab, //4 start
      0xf3bf3f, 0x83d313, 0x4fdf4b, 0x58f898, 0x00ebdb, 0x000000, 0x000000, 0x000000, //3 finish
	0xffe7a3, 0xe3ffa3, 0xabf3bf, 0xb3ffcf, 0x9ffff3, 0x000000, 0x000000, 0x000000  //4 finish
};
*/
u32 NesPalette[64] =  //"AspiringSquire's NES Palette
{
    0x6c6c6c, 0x00268e, 0x0000a8, 0x400090, 0x700070, 0x780040, 0x700000, 0x621600,
    0xbababa, 0x205cdc, 0x3838ff, 0x8020f0, 0xc000c0, 0xd01474, 0xd02020, 0xac4014,
    0x442400, 0x343400, 0x005000, 0x004444, 0x004060, 0x000000, 0x101010, 0x101010,
    0x7c5400, 0x586400, 0x008800, 0x007468, 0x00749c, 0x202020, 0x101010, 0x101010,
    0xffffff, 0x4ca0ff, 0x8888ff, 0xc06cff, 0xff50ff, 0xff64b8, 0xff7878, 0xff9638,
    0xffffff, 0xb0d4ff, 0xc4c4ff, 0xe8b8ff, 0xffb0ff, 0xffb8e8, 0xffc4c4, 0xffd4a8,
    0xdbab00, 0xa2ca20, 0x4adc4a, 0x2ccca4, 0x1cc2ea, 0x585858, 0x101010, 0x101010,
    0xffe890, 0xf0f4a4, 0xc0ffc0, 0xacf4f0, 0xa0e8ff, 0xc2c2c2, 0x202020, 0x101010
};

/************************************/
/* Prototypes                       */
/************************************/
extern void Set_NESInput();
extern int Get_NESInput();
int PS2_LoadGame(char *path);
void SetupNESTexture();
void SetupNESGS();
void DoFun();

int main(int argc, char *argv[])
{
    int ret,sometime;
    char *temp;
	char boot_path[256];
	char *p;

    mpartitions[0][0] = 0;
    mpartitions[1][0] = 0;
    mpartitions[2][0] = 0;

    //Setup PS2 here
    InitPS2();
    setupPS2Pad();

    //Init Settings

	strcpy(boot_path, argv[0]);
	if	(((p=strrchr(boot_path, '/'))==NULL)&&((p=strrchr(boot_path, '\\'))==NULL))
		p=strrchr(boot_path, ':');
	if	(p!=NULL)
		*(p+1)=0;
	//The above cuts away the ELF filename from argv[0], leaving a pure path
	if(!strncmp(boot_path,"hdd0:", 5)) {
		char hdd_path[256];
		char *t;
		sprintf(hdd_path, "%s", boot_path+5);
		t=strchr(hdd_path, ':');
		if	(t!=NULL)
			*(t)=0;
		//hdd0:HDDPATH:pfs:PFSPATH
		sprintf(boot_path, "hdd0:/%s%s", hdd_path, boot_path+5+strlen(hdd_path)+5);//
		if (boot_path[5+1+strlen(hdd_path)]!='/')
			sprintf(boot_path, "hdd0:/%s/", hdd_path);
		//hdd0:/HDDPATHPFSPATH
	}

	Default_Global_CNF();
    Load_Global_CNF(boot_path);

    for (ret  = 0; ret < 3; ret++) {
        sometime = 0x10000;
        while(sometime--) asm("nop\nnop\nnop\nnop");
	}

    SetupGSKit();

    gsKit_init_screen(gsGlobal); //initialize everything
    //init_custom_screen(); //init user screen settings

    loadFont(0);

    //Init Skin
    FCEUSkin.textcolor = 0;
    Load_Skin_CNF(Settings.skinpath);
    for (ret  = 0; ret < 3; ret++) {
        sometime = 0x10000;
		while(sometime--) asm("nop\nnop\nnop\nnop");
	}
    if(!FCEUSkin.textcolor) { //initialize default values
        printf("Load Skin Failed\n");
        Default_Skin_CNF();
    }

    //Setup GUI Textures
    jpgData *Jpg;
    u8 *ImgData;
    if(strstr(FCEUSkin.bgTexture,".png") != NULL) {
         if(gsKit_texture_png(gsGlobal, &BG_TEX, FCEUSkin.bgTexture) < 0) {
            printf("Error with browser background png!\n");
            bgtex = 1;
         }
    }
    else if(strstr(FCEUSkin.bgTexture,".jpg") || strstr(FCEUSkin.bgTexture,".jpeg") != NULL){
        //if(gsKit_texture_jpeg(gsGlobal, &BG_TEX, FCEUSkin.bgTexture) < 0) {
        FILE *File = fopen(FCEUSkin.bgTexture, "r");
        if(File != NULL) {
            Jpg = jpgOpenFILE( File, JPG_WIDTH_FIX);// > 0)
            ImgData = malloc ( Jpg->width * Jpg->height * (Jpg->bpp / 8) );// > 0)
            jpgReadImage( Jpg, ImgData  );
            BG_TEX.PSM = GS_PSM_CT24;
            BG_TEX.Clut = NULL;
            BG_TEX.VramClut = 0;
	        BG_TEX.Width = Jpg->width;
	        BG_TEX.Height = Jpg->height;
	        BG_TEX.Filter = GS_FILTER_LINEAR;
            BG_TEX.Mem = memalign(128, gsKit_texture_size_ee(BG_TEX.Width, BG_TEX.Height, BG_TEX.PSM));
            BG_TEX.Mem = (void*)ImgData;
            BG_TEX.Vram = gsKit_vram_alloc(gsGlobal, gsKit_texture_size(BG_TEX.Width, BG_TEX.Height, BG_TEX.PSM), GSKIT_ALLOC_USERBUFFER);
            gsKit_texture_upload(gsGlobal, &BG_TEX);
            free(BG_TEX.Mem);
        }
        else {
            printf("Error with browser background jpg!\n");
            bgtex = 1;
        }
    }
    else {
        bgtex = 1;
    }

    Jpg = 0;
    ImgData = 0;

    if(strstr(FCEUSkin.bgMenu,".png") != NULL) {
          if(gsKit_texture_png(gsGlobal, &MENU_TEX, FCEUSkin.bgMenu) == -1) {
            printf("Error with menu background png!\n");
            menutex = 1;
          }
    }
    else if(strstr(FCEUSkin.bgMenu,".jpg") || strstr(FCEUSkin.bgMenu,".jpeg") != NULL) {
        //if(gsKit_texture_jpeg(gsGlobal, &MENU_TEX, FCEUSkin.bgMenu) < 0) { //apparently didn't like the "myps2" libjpg
        FILE *File = fopen(FCEUSkin.bgMenu, "r");
        if(File != NULL) {
            Jpg = jpgOpenFILE( File, JPG_WIDTH_FIX);// > 0)
            ImgData = malloc ( Jpg->width * Jpg->height * (Jpg->bpp / 8) );// > 0)
            jpgReadImage( Jpg, ImgData  );
            MENU_TEX.PSM = GS_PSM_CT24;
            MENU_TEX.Clut = NULL;
            MENU_TEX.VramClut = 0;
	        MENU_TEX.Width = Jpg->width;
	        MENU_TEX.Height = Jpg->height;
	        MENU_TEX.Filter = GS_FILTER_LINEAR;
            MENU_TEX.Mem = memalign(128, gsKit_texture_size_ee(MENU_TEX.Width, MENU_TEX.Height, MENU_TEX.PSM));
            MENU_TEX.Mem = (void*)ImgData;
            MENU_TEX.Vram = gsKit_vram_alloc(gsGlobal, gsKit_texture_size(MENU_TEX.Width, MENU_TEX.Height, MENU_TEX.PSM), GSKIT_ALLOC_USERBUFFER);
            gsKit_texture_upload(gsGlobal, &MENU_TEX);
            free(MENU_TEX.Mem);
        }
        else {
            printf("Error with menu background jpg!\n");
            menutex =1;
        }
    }
    else {
        menutex = 1;
    }


    if(!(ret=FCEUI_Initialize())) { //allocates all memory for FCEU* functions
		printf("FCEUltra did not initialize.\n");
		return(0);
	}

    //Setup FCEUltra here
    FCEUI_SetVidSystem(Settings.emulation); //0=ntsc 1=pal
	FCEUI_SetGameGenie(1);
	FCEUI_DisableSpriteLimitation(1);
//	FCEUI_SetRenderedLines(8, 231, 0, 239);
//	FCEUI_GetCurrentVidSystem(&srendline, &erendline);
//	totallines = erendline - srendline + 1;
#ifdef SOUND_ON
	FCEUI_SetSoundVolume(1024);
#else
	FCEUI_SetSoundVolume(0);
#endif
	FCEUI_SetSoundQuality(0);
	FCEUI_SetLowPass(Settings.lowpass);
	FCEUI_Sound(SAMPLERATE);

#ifdef SOUND_ON
	struct audsrv_fmt_t format;
	format.bits = 16;
	format.freq = 44100;
	format.channels = 1;
	audsrv_set_format(&format);
	audsrv_set_volume(MAX_VOLUME);
#endif

    SetupNESTexture();

//main emulation loop
Start_PS2Browser:
    strcpy(path,Browser(1,0));

    if(PS2_LoadGame(path) == 0) {
        goto Start_PS2Browser;
    }

    Set_NESInput();
    SetupNESGS();

    while(CurGame) //FCEUI_CloseGame turns this false
        DoFun();

#ifdef SOUND_ON
    audsrv_stop_audio();
#endif
    temp = strrchr(path,'/');
    temp++;
    *temp = 0;

    goto Start_PS2Browser;

    return(0);
}

int PS2_LoadGame(char *path)
{
    FCEUGI *tmp;

//	CloseGame();
    if((tmp=FCEUI_LoadGame(path))) {
        printf("Loaded!\n");
        CurGame=tmp;
        return 1;
    }
    else {
        printf("Didn't load!\n");
        return 0;
    }
}

void SetupNESTexture()
{
    int i,r,g,b;

    //comments after settings are for regular clut lookup
    //Setup NES_TEX Texture
    NES_TEX.PSM = GS_PSM_T8; //GS_PSM_CT32
    NES_TEX.ClutPSM = GS_PSM_CT32; //comment out
    NES_TEX.Clut = memalign(128, gsKit_texture_size_ee(16, 16, NES_TEX.ClutPSM)); //NULL
    NES_TEX.VramClut = gsKit_vram_alloc(gsGlobal, gsKit_texture_size(16, 16, NES_TEX.ClutPSM), GSKIT_ALLOC_USERBUFFER); //0
	NES_TEX.Width = 256;
	NES_TEX.Height = 240;
	NES_TEX.TBW = 4;
    //NES_TEX.Mem = memalign(128, gsKit_texture_size_ee(NES_TEX.Width, NES_TEX.Height, NES_TEX.PSM));
    NES_TEX.Vram = gsKit_vram_alloc(gsGlobal, gsKit_texture_size(NES_TEX.Width, NES_TEX.Height, NES_TEX.PSM), GSKIT_ALLOC_USERBUFFER);

    //Setup NES Clut
    for( i = 0; i< 64 ; i++ )
    {
        // 32-bit bgr -< rgb
        r =  ( NesPalette[ i ] & 0xff0000 )>>16;
        g =  ( NesPalette[ i ] & 0xff00 )>>8;
        b =  ( NesPalette[ i ] & 0xff )<<0;
        NES_TEX.Clut[ i ] = ((b<<16)|(g<<8)|(r<<0));  //NES_TEX.Clut = ps2palette;
        NES_TEX.Clut[i+64] = ((b<<16)|(g<<8)|(r<<0));
        NES_TEX.Clut[i+128] = ((b<<16)|(g<<8)|(r<<0));
        NES_TEX.Clut[i+192] = ((b<<16)|(g<<8)|(r<<0));
    }

}

void SetupNESGS(void)
{
    gsGlobal->DrawOrder = GS_OS_PER;
    gsKit_mode_switch(gsGlobal, GS_PERSISTENT);
    gsKit_queue_reset(gsGlobal->Per_Queue);

    if(Settings.filter) {
        NES_TEX.Filter = GS_FILTER_LINEAR;
    }
    else {
        NES_TEX.Filter = GS_FILTER_NEAREST;
    }

    gsKit_clear(gsGlobal, GS_SETREG_RGBA(0x00,0x00,0x00,0x80));

    //gsKit_prim_sprite_striped_texture( gsGlobal, &NES_TEX,  //thought this might be needed for different modes, but it just looks bad
    gsKit_prim_sprite_texture( gsGlobal, &NES_TEX,
						0.0f, /* X1 */
						0.0f, /* Y1 */
						0.0f, /* U1 */
						0.0f, /* V1 */
                        gsGlobal->Width, /* X2 */ //stretch to screen width
						gsGlobal->Height, /* Y2 */ //stretch to screen height
						NES_TEX.Width, /* U2 */
						NES_TEX.Height, /* V2*/
						2, /* Z */
						GS_SETREG_RGBA(0x80,0x80,0x80,0x80) /* RGBA */
						);

}

void RenderFrame(const uint8 *frame)
{
    //int w,h,c;
    //int i;

/*
    for(h=0; h<240; h++) { //correctly displays 256x240 nes screen
        for(w=0; w<256; w++) {
            c = (h << 8) + w; //color index, increments height by 256, then adds width
            NES_TEX.Mem[c] = ps2palette[frame[c]];
        }
    }
*/
    NES_TEX.Mem=(u32 *)frame; //set frame as NES_TEX.Mem location

    gsKit_texture_upload(gsGlobal, &NES_TEX);

    /* vsync and flip buffer */
    gsKit_sync_flip(gsGlobal);

    /* execute render queue */
    gsKit_queue_exec(gsGlobal);
}

#ifdef SOUND_ON
void inline OutputSound(const int32 *tmpsnd, int32 ssize)
{
    //used as an example from the windows driver
    /*static int16 MBuffer[2 * 96000 / 50];  // * 2 for safety.
    int P;

    if(!bittage) {
        for(P=0;P<Count;P++)
            *(((uint8*)MBuffer)+P)=((int8)(Buffer[P]>>8))^128;
        RawWrite(MBuffer,Count);
    }
    else {
        for(P=0;P<Count;P++)
        MBuffer[P]=Buffer[P];
        //FCEU_printf("Pre: %d\n",RawCanWrite() / 2);
        RawWrite(MBuffer,Count * 2);
        //FCEU_printf("Post: %d\n",RawCanWrite() / 2);
     }*/

    int i;
    s16 ssound[ssize]; //no need for an 2*ssized 8bit array with this

    //audsrv_wait_audio(ssize<<1); //commented out because the sound buffer is filled at need
    for (i=0;i<ssize;i++) {
        //something[i]=((tmpsnd[i]>>8))^128; //for 8bit sound
        ssound[i]=tmpsnd[i];
    }

    audsrv_play_audio((s8 *)ssound,ssize<<1); //
}
#endif

void FCEUD_Update(const uint8 *XBuf, const int32 *tmpsnd, int32 ssize)
{
    RenderFrame(XBuf);
#ifdef SOUND_ON
    OutputSound(tmpsnd, ssize);
#endif
    if ( Get_NESInput() ) {
        FCEUI_CloseGame();
        CurGame=0;
    }
}

void DoFun()
{
    uint8 *gfx;
    int32 *sound;
    int32 ssize;

    FCEUI_Emulate(&gfx, &sound, &ssize, 0);
    FCEUD_Update(gfx, sound, ssize);
}


