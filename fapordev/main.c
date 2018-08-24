/*
	NDP by SKGleba
	All Rights Reserved
*/

// IME shit based on Freakler's vita-uriCaller

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <psp2/io/fcntl.h>
#include <psp2/appmgr.h>
#include <psp2/apputil.h>
#include <psp2/types.h>
#include <psp2/kernel/processmgr.h>
#include <psp2/message_dialog.h>
#include <psp2/ime_dialog.h>
#include <psp2/display.h>
#include <psp2/apputil.h>

#include <vita2d.h>

#define SCE_IME_DIALOG_MAX_TITLE_LENGTH	(128)
#define SCE_IME_DIALOG_MAX_TEXT_LENGTH	(512)

#define ARRAYSIZE(x) ((sizeof(x)/sizeof(0[x])) / ((size_t)(!(sizeof(x) % sizeof(0[x])))))
#define IME_DIALOG_RESULT_NONE 0
#define IME_DIALOG_RESULT_RUNNING 1
#define IME_DIALOG_RESULT_FINISHED 2
#define IME_DIALOG_RESULT_CANCELED 3


static uint16_t ime_title_utf16[SCE_IME_DIALOG_MAX_TITLE_LENGTH];
static uint16_t ime_initial_text_utf16[SCE_IME_DIALOG_MAX_TEXT_LENGTH];
static uint16_t ime_input_text_utf16[SCE_IME_DIALOG_MAX_TEXT_LENGTH + 1];
static uint8_t ime_input_text_utf8[SCE_IME_DIALOG_MAX_TEXT_LENGTH + 1];


typedef struct {
	uint32_t off;
	uint32_t sz;
	uint8_t code;
	uint8_t type;
	uint8_t active;
	uint32_t flags;
	uint16_t unk;
} __attribute__((packed)) partition_t;

typedef struct {
	char magic[0x20];
	uint32_t version;
	uint32_t device_size;
	char unk1[0x28];
	partition_t partitions[0x10];
	char unk2[0x5e];
	char unk3[0x10 * 4];
	uint16_t sig;
} __attribute__((packed)) master_block_t;

typedef struct {
	char unk1[0x20]; //shh too lazy
	uint32_t device_size;
	char unk2[0x1C]; // IDK, device specific?
} __attribute__((packed)) master_block_t_crop16;

typedef struct {
	char unk1[0x40]; //shh too lazy
	char flags1[0x8]; // flags?
	uint32_t device_size;
	char flags2[0x4]; // flags?
} __attribute__((packed)) master_block_t_cropex;


int ex(const char *fname) {
    FILE *file;
    if ((file = fopen(fname, "r")))
    {
        fclose(file);
        return 1;
    }
    return 0;
}


void utf16_to_utf8(uint16_t *src, uint8_t *dst) {
	int i;
	for (i = 0; src[i]; i++) {
		if ((src[i] & 0xFF80) == 0) {
			*(dst++) = src[i] & 0xFF;
		} else if((src[i] & 0xF800) == 0) {
			*(dst++) = ((src[i] >> 6) & 0xFF) | 0xC0;
			*(dst++) = (src[i] & 0x3F) | 0x80;
		} else if((src[i] & 0xFC00) == 0xD800 && (src[i + 1] & 0xFC00) == 0xDC00) {
			*(dst++) = (((src[i] + 64) >> 8) & 0x3) | 0xF0;
			*(dst++) = (((src[i] >> 2) + 16) & 0x3F) | 0x80;
			*(dst++) = ((src[i] >> 4) & 0x30) | 0x80 | ((src[i + 1] << 2) & 0xF);
			*(dst++) = (src[i + 1] & 0x3F) | 0x80;
			i += 1;
		} else {
			*(dst++) = ((src[i] >> 12) & 0xF) | 0xE0;
			*(dst++) = ((src[i] >> 6) & 0x3F) | 0x80;
			*(dst++) = (src[i] & 0x3F) | 0x80;
		}
	}

	*dst = '\0';
}

void utf8_to_utf16(uint8_t *src, uint16_t *dst) {
	int i;
	for (i = 0; src[i];) {
		if ((src[i] & 0xE0) == 0xE0) {
			*(dst++) = ((src[i] & 0x0F) << 12) | ((src[i + 1] & 0x3F) << 6) | (src[i + 2] & 0x3F);
			i += 3;
		} else if ((src[i] & 0xC0) == 0xC0) {
			*(dst++) = ((src[i] & 0x1F) << 6) | (src[i + 1] & 0x3F);
			i += 2;
		} else {
			*(dst++) = src[i];
			i += 1;
		}
	}

	*dst = '\0';
}
 
void initImeDialog(char *title, char *initial_text, int max_text_length) {
    // Convert UTF8 to UTF16
	utf8_to_utf16((uint8_t *)title, ime_title_utf16);
	utf8_to_utf16((uint8_t *)initial_text, ime_initial_text_utf16);
 
    SceImeDialogParam param;
	sceImeDialogParamInit(&param);

	param.sdkVersion = 0x03150021,
	param.supportedLanguages = 0x0001FFFF;
	param.languagesForced = SCE_TRUE;
	param.type = SCE_IME_TYPE_BASIC_LATIN;
	param.title = ime_title_utf16;
	param.maxTextLength = max_text_length;
	param.initialText = ime_initial_text_utf16;
	param.inputTextBuffer = ime_input_text_utf16;

	//int res = 
	sceImeDialogInit(&param);
	return ;
}

void oslOskGetText(char *text){
	// Convert UTF16 to UTF8
	utf16_to_utf8(ime_input_text_utf16, ime_input_text_utf8);
	strcpy(text,(char*)ime_input_text_utf8);
}


int ndpWorkDevBRBasedFullfap(const char* inp, const char* oup, int type){
int cdevsz;
if (ex(oup) == 1) sceIoRemove(oup);
if (ex("ur0:temp/tempmbr.x") == 1) sceIoRemove("ur0:temp/tempmbr.x");
kndpWorkDevice(inp, "ur0:temp/tempmbr.x", 1, 0x200, 0, 0, 2);
if (ex("ur0:temp/tempmbr.x") == 1){
if (type == 0){ //MBR
  SceUID fd;
  static master_block_t master;
  fd = sceIoOpen("ur0:temp/tempmbr.x", SCE_O_RDONLY, 0777);
  sceIoRead(fd, &master, sizeof(master));
  cdevsz = master.device_size * 0x200;
  sceIoClose(fd);
}
if (type == 1){ //FAT16 PBR
  SceUID fd;
  static master_block_t_crop16 master;
  fd = sceIoOpen("ur0:temp/tempmbr.x", SCE_O_RDONLY, 0777);
  sceIoRead(fd, &master, sizeof(master));
  cdevsz = master.device_size * 0x200;
  sceIoClose(fd);
}
if (type == 2){ //exFAT PBR
  SceUID fd;
  static master_block_t_cropex master;
  fd = sceIoOpen("ur0:temp/tempmbr.x", SCE_O_RDONLY, 0777);
  sceIoRead(fd, &master, sizeof(master));
  cdevsz = master.device_size * 0x200;
  sceIoClose(fd);
}
sceIoRemove("ur0:temp/tempmbr.x");
kndpWorkDevice(inp, oup, 1, cdevsz, 0, 0, 2);
}}


int main(int argc, const char *argv[]) {
if (ex("ur0:temp/sdstor0.x") == 1) sceIoRemove("ur0:temp/sdstor0.x");
int cmode = 0;
int smode = 0;
int ocdevsz = 0;
cmode = kndpReadBuffer(1);
smode = kndpReadBuffer(2);
ocdevsz = kndpReadBuffer(3);
	sceAppUtilInit(&(SceAppUtilInitParam){}, &(SceAppUtilBootParam){});
    sceCommonDialogSetConfigParam(&(SceCommonDialogConfigParam){});
 
    vita2d_init();

    int shown_dial = 0;
	int try = 0;
	char userText[512] = "";
	char ddev[128];
	char rdev[128];
	const char ldev[5][32] = {"ux0", "ur0", "uma0", "imc0", "xmc0"};

    while (1) {
        if(!shown_dial){
       		if (ex("ur0:temp/sdstor0.x") == 0 && try == 0) initImeDialog("ENTER DEVICE BLK NAME", userText, 128);
       		if (ex("ur0:temp/sdstor0.x") == 0 && try == 1) initImeDialog("NOT EXISTS", userText, 128);
       		if (ex("ur0:temp/sdstor0.x") == 1 && cmode == 0) initImeDialog("EXISTS", userText, 128);
       		if (ex("ur0:temp/sdstor0.x") == 1 && cmode != 0) initImeDialog("DUMPED", userText, 128);
			shown_dial=1;
        }

        vita2d_start_drawing();
        vita2d_clear_screen();

        SceCommonDialogStatus status = sceImeDialogGetStatus();
		if (status == IME_DIALOG_RESULT_FINISHED) {
			SceImeDialogResult result;
			memset(&result, 0, sizeof(SceImeDialogResult));
			sceImeDialogGetResult(&result);

			if (result.button == SCE_IME_DIALOG_BUTTON_CLOSE) {
				status = IME_DIALOG_RESULT_CANCELED;
				sceAppMgrLoadExec("app0:eboot.bin", NULL, NULL);
				break;
			} else {
				oslOskGetText(userText);
			}

			sceImeDialogTerm();
			shown_dial = 0;
			try = 1;
			if (ex("ur0:temp/sdstor0.x") == 1) sceIoRemove("ur0:temp/sdstor0.x");
			sprintf(rdev, "sdstor0:%s", userText);
			kndpWorkDevice(rdev, "ur0:temp/sdstor0.x", 1, 0, 0, 0, 0);
			if (ex("ur0:temp/sdstor0.x") == 1 && cmode != 0) {
			sprintf(ddev, "%s:ndp/%s.x", ldev[smode], userText);
			if (ex(ddev) == 1) sceIoRemove(ddev);
			if (cmode == 1) kndpWorkDevice(rdev, ddev, 1, 0x200, 0, 0, 2);
			if (cmode == 2) kndpWorkDevice(rdev, ddev, 1, 0x40000, 0, 0, 2);
			if (cmode == 3) ndpWorkDevBRBasedFullfap(rdev, ddev, 0);
			if (cmode == 4) ndpWorkDevBRBasedFullfap(rdev, ddev, 1);
			if (cmode == 5) ndpWorkDevBRBasedFullfap(rdev, ddev, 2);
			if (cmode == 69) kndpWorkDevice(rdev, ddev, 1, ocdevsz, 0, 0, 2);



}
		}
       
        vita2d_end_drawing();
        vita2d_common_dialog_update();
        vita2d_swap_buffers();
        sceDisplayWaitVblankStart();
    }
    sceKernelExitProcess(0);
    return 0;
}
