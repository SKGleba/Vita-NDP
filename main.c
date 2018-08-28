/*
	NDP by SKGleba
	All Rights Reserved
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <psp2/message_dialog.h>
#include <psp2/ime_dialog.h>
#include <psp2/display.h>
#include <psp2/apputil.h>
#include <psp2/types.h>

#include <psp2/kernel/processmgr.h>
#include <psp2/ctrl.h>
#include <psp2/io/fcntl.h>
#include <psp2/io/stat.h>
#include <taihen.h>
#include "graphics.h"

#define ARRAYSIZE(x) ((sizeof(x)/sizeof(0[x])) / ((size_t)(!(sizeof(x) % sizeof(0[x])))))

//kndp
#define MOD_PATH "ux0:app/SKGNDPDMP/kp"

//Partitions:
#define iosblk "int-lp-ina-os"
#define aosblk "int-lp-act-os"
#define vshblk "int-lp-ign-vsh"
#define sysblk "int-lp-ign-sysdata"
#define trmblk "int-lp-ign-vtrm"
#define vsdblk "int-lp-ign-vshdata"
#define pidblk "int-lp-ign-pidata"
#define iblblk "int-lp-ina-sloader"
#define ablblk "int-lp-act-sloader"
#define idsblk "int-lp-ign-idstor"
#define uriblk "int-lp-ign-user"
#define mcdblk "mcd-lp-act-entire"
#define uxeblk "mcd-lp-act-userext"
#define mmdblk "mcd-lp-act-mediaid"
#define gmdblk "gcd-lp-act-mediaid"
#define gcdblk "gcd-lp-act-entire"
#define emcblk "int-lp-act-entire"
#define imcblk "int-lp-ign-userext"
#define groblk "gcd-lp-ign-gamero"
#define grwblk "gcd-lp-ign-gamerw"

//Combos
#define WMODECG (SCE_CTRL_SQUARE | SCE_CTRL_TRIANGLE)
#define SMODECGR (SCE_CTRL_SQUARE | SCE_CTRL_RIGHT)
#define SMODECGL (SCE_CTRL_SQUARE | SCE_CTRL_LEFT)
#define SZODECGR (SCE_CTRL_TRIANGLE | SCE_CTRL_RIGHT)
#define SZODECGL (SCE_CTRL_TRIANGLE | SCE_CTRL_LEFT)
#define MDODECGR (SCE_CTRL_CIRCLE | SCE_CTRL_RIGHT)
#define MDODECGL (SCE_CTRL_CIRCLE | SCE_CTRL_LEFT)
#define PDEVSZCC (SCE_CTRL_CIRCLE | SCE_CTRL_UP)
#define PDEVSZXX (SCE_CTRL_SQUARE | SCE_CTRL_UP)
#define PDEVSZIX (SCE_CTRL_CROSS | SCE_CTRL_UP)
#define PDEVSZXC (SCE_CTRL_TRIANGLE | SCE_CTRL_UP)
#define PDEVSZCM (SCE_CTRL_LTRIGGER | SCE_CTRL_UP)
#define PDEVBLKO (SCE_CTRL_RTRIGGER | SCE_CTRL_UP)
#define MDEVSZCC (SCE_CTRL_CIRCLE | SCE_CTRL_DOWN)
#define MDEVSZXX (SCE_CTRL_SQUARE | SCE_CTRL_DOWN)
#define MDEVSZIX (SCE_CTRL_CROSS | SCE_CTRL_DOWN)
#define MDEVSZXC (SCE_CTRL_TRIANGLE | SCE_CTRL_DOWN)
#define MDEVSZCM (SCE_CTRL_LTRIGGER | SCE_CTRL_DOWN)
#define MDEVBLKO (SCE_CTRL_RTRIGGER | SCE_CTRL_DOWN)

#define LOG(...) \
	do { \
		char buffer[256]; \
		snprintf(buffer, sizeof(buffer), ##__VA_ARGS__); \
		log(buffer, strlen(buffer)); \
} while (0)

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

const char ddev[5][32] = {"ux0", "ur0", "uma0", "imc0", "xmc0"};
const char mddev[4][32] = {"INT   ", "GCD   ", "MCD   ", "CUSTOM"};
char mit[][100] = {" -> vs0 ( int-lp-ign-vsh ) "," -> vd0 ( int-lp-ign-vshdata )"," -> tm0 ( int-lp-ign-vtrm )"," -> ur0 ( int-lp-ign-user )"," -> sa0 ( int-lp-ign-sysdata )"," -> pd0 ( int-lp-ign-pidata )"," -> Enable RW access to all partitions"," -> Exit"};
char wit[][150] = {" -> Perform a factory backup"," -> Active os0 ( int-lp-act-os )"," -> Inactive os0 ( int-lp-ina-os )"," -> Active slb2 ( int-lp-act-sloader )"," -> Inactive slb2 ( int-lp-ina-sloader )"," -> idstorage ( int-lp-ign-idstor )"," -> MBR"," -> Reboot"};
char cit[][150] = {" -> ux0 ( mcd-lp-act-userext )"," -> MediaId ( mcd-lp-act-mediaid )"," -> MBR"," -> Reboot", " -> Exit"};
char rit[][150] = {" -> grw0 ( gcd-lp-ign-gamerw )"," -> gro0 ( gcd-lp-ign-gamero )"," -> MediaId ( gcd-lp-act-mediaid )"," -> MBR"," -> Reboot", " -> Exit"};
char kit[][256] = {" -> Check if the device exists (BLKNAME)"," -> Dump one block"," -> Dump MBR"," -> Dump device (MASTER)"," -> Dump device (FAT16)"," -> Dump device (exFAT)"," -> Dump device (custom size)"," -> Clone device (INT/GCD/MCD/USB)"," -> Bruteforce device names"};
const char hw[7][32] = {"int", "ext", "gcd", "mcd", "xmc", "uma", "usd"};
char hwdesc[7][128] = {"Internal eMMC","Device in Game Card slot","Game Card","Device in Memory Card slot","Memory Card","USB","Unknown"};

int item_count = 8;
int smodes = 6;
int szodes = 4;

int smd = 0;
int opmode = 1;

int urmode = 0;

char utx[256];
char dumpdev[256];
char rmdev[128];
char iexdev[256];
char lexdev[256];
char fnflagf[13];
char ddevf[13];
char logf[16] = "ux0:ndp/devices.log";

int ocdevsz = 0;

int omd = 0;

int cinf = 0;
int couf = 1;

int cbfsz;
int wmode = 0;
int smode = 0;
int szode = 2;

int rapechk = 0;

int sel = 0;
int sub_sel = 0;
int i;
int prs;

void log(const char *buffer, size_t length)
{

	SceUID fd = sceIoOpen(logf,
		SCE_O_WRONLY | SCE_O_CREAT | SCE_O_APPEND, 6);
	if (fd < 0)
		return;

	sceIoWrite(fd, buffer, length);
	sceIoClose(fd);
}

int filecopy(const char *from, const char *to) {
	long plugin_size;
	FILE *fp = fopen(from,"rb");

	fseek(fp, 0, SEEK_END);
	plugin_size = ftell(fp);
	rewind(fp);

	char* plugin_buffer = (char*) malloc(sizeof(char) * plugin_size);
	fread(plugin_buffer, sizeof(char), (size_t)plugin_size, fp);

	FILE *pFile = fopen(to, "wb");
	
	for (int i = 0; i < plugin_size; ++i) {
			fputc(plugin_buffer[i], pFile);
	}
   
	fclose(fp);
	fclose(pFile);
	return 1;
}

int exists(const char *fname) {
    FILE *file;
    if ((file = fopen(fname, "r")))
    {
        fclose(file);
        return 1;
    }
    return 0;
}

int fpsz(master_block_t *master, uint8_t wpc) {
	int prsz;
	for (size_t i = 0; i < ARRAYSIZE(master->partitions); ++i) {
		partition_t *p = &master->partitions[i];
		if (p->code == wpc)
			prsz = i;}
	return prsz;
}

int chknorape(){
sceKernelDelayThread(0.35 * 1000 * 1000);
SceCtrlData pad;
rapechk = 1;
psvDebugScreenPrintf("\nPress CROSS to continue or CIRCLE to abort...\n\n");
while (rapechk == 1){
sceCtrlPeekBufferPositive(0, &pad, 1);
if (pad.buttons == SCE_CTRL_CROSS) {rapechk = 0; return 0;}
if (pad.buttons == SCE_CTRL_CIRCLE) {rapechk = 0; return 1;}}}


void smn(){
if (omd == 0) item_count = 8;
if (omd == 1) item_count = 6;
if (omd == 2) item_count = 5;
if (omd == 3) item_count = 9;
	psvDebugScreenClear(COLOR_BLACK);
	psvDebugScreenSetFgColor(COLOR_RED);
		if (wmode == 0) psvDebugScreenPrintf("MODE:RR&R");
		if (wmode == 2) psvDebugScreenPrintf("MODE:RX&W");
		if (szode == 1) cbfsz = 0x4000;
		if (szode == 2) cbfsz = 0x8000;
		if (szode == 0) cbfsz = 0x2000;
					psvDebugScreenPrintf("       BUFSZ:0x%X", cbfsz);
		if (omd < 4) psvDebugScreenPrintf("       DEV:%s", mddev[omd]);
		if (omd > 3) psvDebugScreenPrintf("       DEV:CUSTOM");
		if (omd != 5) psvDebugScreenPrintf("      LOC:%s", ddev[smode]);
	psvDebugScreenSetFgColor(COLOR_GREEN);
	psvDebugScreenPrintf("\n\n                          NDP v6.1                               \n");
		psvDebugScreenPrintf("                         By SKGleba                              \n");
			psvDebugScreenSetFgColor(COLOR_CYAN);
		if (smd == 0 && omd == 0) psvDebugScreenPrintf("               LIST MODE:  [SOFTWARE]  FIRMWARE                  \n");
		if (smd == 1 && omd == 0) psvDebugScreenPrintf("               LIST MODE:   SOFTWARE  [FIRMWARE]                 \n");
		  if (opmode == 1 && omd < 3) psvDebugScreenPrintf("              OPERATION MODE:  [BACKUP]  RESTORE                 \n");
		  if (opmode == 0 && omd < 3) psvDebugScreenPrintf("              OPERATION MODE:   BACKUP  [RESTORE]                \n");
		  if (omd < 3 && opmode == 1) psvDebugScreenPrintf("                 PRESS [SELECT] TO DUMP: %s                     \n", mddev[omd]);
		  if (omd < 3 && opmode == 0) psvDebugScreenPrintf("                PRESS [SELECT] TO RESTORE: %s                   \n", mddev[omd]);
					if (omd > 3) psvDebugScreenPrintf("\n\n                  PRESS [SELECT] TO GO BACK                      \n");
						if (omd > 3) psvDebugScreenPrintf("             PRESS [START] TO ACCEPT AND CONTINUE                \n");
			psvDebugScreenSetFgColor(COLOR_WHITE);
					  if (omd == 4) psvDebugScreenPrintf("\n                     ENTER DEVICE SIZE:                          \n");
					  if (omd == 5) psvDebugScreenPrintf("\n                 CHOOSE DEVICE(S) TO CLONE:                      \n");
			psvDebugScreenSetFgColor(COLOR_GREEN);
				  if (omd == 4) psvDebugScreenPrintf("\n\n\n         DEVICE SIZE (BLOCKS): 0x%X\n", ocdevsz);
				  if (omd == 5) psvDebugScreenPrintf("\n\n\n       DEVICE (INPUT):  %s ( %s )\n", hwdesc[cinf], hw[cinf]);
			psvDebugScreenSetFgColor(COLOR_YELLOW);
				    if (omd == 5) psvDebugScreenPrintf("\n\n       DEVICE (OUTPUT): %s ( %s )\n\n", hwdesc[couf], hw[couf]);
			        if (omd == 4) psvDebugScreenPrintf("\n\n         DEVICE SIZE (FINAL):  0x%X\n", (ocdevsz * 0x200));
		psvDebugScreenSetFgColor(COLOR_RED);
	for(i = 0; i < item_count; i++){
		if(sel==i){
			psvDebugScreenSetFgColor(COLOR_GREEN);
		}
		if (smd == 0 && omd == 0) psvDebugScreenPrintf("%s\n", mit[i]);
		if (smd == 1 && omd == 0) psvDebugScreenPrintf("%s\n", wit[i]);
		if (omd == 1) psvDebugScreenPrintf("%s\n", rit[i]);
		if (omd == 2) psvDebugScreenPrintf("%s\n", cit[i]);
		if (omd == 3) psvDebugScreenPrintf("%s\n", kit[i]);
		psvDebugScreenSetFgColor(COLOR_RED);
	}
	
	psvDebugScreenSetFgColor(COLOR_GREEN);
}


int ndpWorkDeviceDefault(const char* inp, const char* oup, int devsz, int islc, int oslc) {
psvDebugScreenPrintf("Preparing ( %s )...\n", oup);
if (opmode == 1){
sprintf(utx, "sdstor0:%s", inp);
sprintf(dumpdev, "%s:ndp/%s.img", ddev[smode], oup);
if (exists(dumpdev) == 1) sceIoRemove(dumpdev);}
if (opmode == 0){
sprintf(dumpdev, "sdstor0:%s", inp);
sprintf(utx, "%s:ndp_f/%s.img", ddev[smode], oup);}
psvDebugScreenPrintf("Target IN: %s ( %s )...\n", utx, oup);
psvDebugScreenPrintf("Target OUT: %s ( %s )...\n", dumpdev, oup);
psvDebugScreenPrintf("Target size: 0x%X ( %s )...\n", devsz, oup);
int rape = chknorape();
if (rape == 0){
psvDebugScreenPrintf("Working ( %s )...\n", oup);
kndpWorkDevice(utx, dumpdev, opmode, devsz, islc, oslc, szode);}
psvDebugScreenPrintf("Done...\n");
sceKernelDelayThread(1 * 1000 * 1000);
return 1;
}

int ndpReWriteDev(){
int cdevsz;
psvDebugScreenPrintf("Preparing ( CLONE )...\n");
sprintf(utx, "sdstor0:%s-lp-act-entire", hw[cinf]);
sprintf(dumpdev, "sdstor0:%s-lp-act-entire", hw[couf]);
if (exists("ur0:temp/tempmbr.x") == 1) sceIoRemove("ur0:temp/tempmbr.x");
kndpWorkDevice(utx, "ur0:temp/tempmbr.x", 1, 0x200, 0, 0, 2);
if (exists("ur0:temp/tempmbr.x") == 1){
  SceUID fd;
  static master_block_t master;
  fd = sceIoOpen("ur0:temp/tempmbr.x", SCE_O_RDONLY, 0777);
  sceIoRead(fd, &master, sizeof(master));
  cdevsz = master.device_size * 0x200;
  sceIoClose(fd);
sceIoRemove("ur0:temp/tempmbr.x");
psvDebugScreenPrintf("Target IN: %s ( CLONE )...\n", utx);
psvDebugScreenPrintf("Target OUT: %s ( CLONE )...\n", dumpdev);
psvDebugScreenPrintf("Device size: 0x%X ( CLONE )...\n", cdevsz);
int rape = chknorape();
if (rape == 0){
psvDebugScreenPrintf("Working ( CLONE )...\n");
kndpWorkDevice(utx, dumpdev, 0, cdevsz, 0, 0, szode);}
psvDebugScreenPrintf("Done...\n");
sceKernelDelayThread(1 * 1000 * 1000);
}
return 1;
}

int ndpWorkDevBRBasedFull(const char* inp, const char* oup, int type){
int cdevsz;
psvDebugScreenPrintf("Preparing ( %s )...\n", oup);
if (opmode == 1){
sprintf(utx, "sdstor0:%s", inp);
sprintf(dumpdev, "%s:ndp/%s.img", ddev[smode], oup);
if (exists(dumpdev) == 1) sceIoRemove(dumpdev);}
if (opmode == 0){
sprintf(dumpdev, "sdstor0:%s", inp);
sprintf(utx, "%s:ndp_f/%s.img", ddev[smode], oup);}
if (exists("ur0:temp/tempmbr.x") == 1) sceIoRemove("ur0:temp/tempmbr.x");
kndpWorkDevice(utx, "ur0:temp/tempmbr.x", 1, 0x200, 0, 0, 2);
if (exists("ur0:temp/tempmbr.x") == 1){
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
psvDebugScreenPrintf("Target IN: %s ( %s )...\n", utx, oup);
psvDebugScreenPrintf("Target OUT: %s ( %s )...\n", dumpdev, oup);
psvDebugScreenPrintf("Device size: 0x%X ( %s )...\n", cdevsz, oup);
int rape = chknorape();
if (rape == 0){
psvDebugScreenPrintf("Working ( %s )...\n", oup);
kndpWorkDevice(utx, dumpdev, opmode, cdevsz, 0, 0, szode);}
}
psvDebugScreenPrintf("Done...\n");
sceKernelDelayThread(1 * 1000 * 1000);}

int ndpWorkDeviceMBRBased(const char* minp, const char* inp, const char* oup, int pcd){
int cdevsz;
psvDebugScreenPrintf("Preparing ( %s )...\n", oup);
if (opmode == 1){
sprintf(utx, "sdstor0:%s", inp);
sprintf(dumpdev, "%s:ndp/%s.img", ddev[smode], oup);
if (exists(dumpdev) == 1) sceIoRemove(dumpdev);}
if (opmode == 0){
sprintf(dumpdev, "sdstor0:%s", inp);
sprintf(utx, "%s:ndp_f/%s.img", ddev[smode], oup);}
sprintf(rmdev, "sdstor0:%s", minp);
if (exists("ur0:temp/tempmbr.x") == 1) sceIoRemove("ur0:temp/tempmbr.x");
kndpWorkDevice(rmdev, "ur0:temp/tempmbr.x", 1, 0x200, 0, 0, 2);
if (exists("ur0:temp/tempmbr.x") == 1){
  SceUID fd;
  static master_block_t master;
  fd = sceIoOpen("ur0:temp/tempmbr.x", SCE_O_RDONLY, 0777);
  sceIoRead(fd, &master, sizeof(master));
  int devszp = fpsz(&master, pcd);
  cdevsz = master.partitions[devszp].sz * 0x200;
  sceIoClose(fd);
sceIoRemove("ur0:temp/tempmbr.x");
psvDebugScreenPrintf("Target IN: %s ( %s )...\n", utx, oup);
psvDebugScreenPrintf("Target OUT: %s ( %s )...\n", dumpdev, oup);
psvDebugScreenPrintf("Device size: 0x%X ( %s )...\n", cdevsz, oup);
int rape = chknorape();
if (rape == 0){
psvDebugScreenPrintf("Working ( %s )...\n", oup);
kndpWorkDevice(utx, dumpdev, opmode, cdevsz, 0, 0, szode);}
}
psvDebugScreenPrintf("Done...\n");
sceKernelDelayThread(1 * 1000 * 1000);}
	
int dmloc() {
ndpWorkDeviceMBRBased(emcblk, iosblk, "os0_ina", 0x3);
ndpWorkDeviceMBRBased(emcblk, iblblk, "slb2_ina", 0x2);
ndpWorkDeviceMBRBased(emcblk, idsblk, "idstorage", 0x1);
}

int fapcall(int fnflag){
kndpWriteBuffer(fnflag, 1);
kndpWriteBuffer(smode, 2);
kndpWriteBuffer((ocdevsz * 0x200), 3);
sceAppMgrLoadExec("app0:fapordev", NULL, NULL);} //Find Another Partition OR DEVice

int brutefap(){
psvDebugScreenPrintf("Bruteforcing, please be patient.\n");

	const char tbl[2][8] = {"lp", "pp"};
	const char act[3][16] = {"ina", "act", "ign"};
	const char pt[17][256] = {"a", "unused", "idstor", "sloader", "os", "vsh", "vshdata", "vtrm", "user", "userext", "gamero", "gamerw", "updater", "sysdata", "mediaid", "pidata", "entire"};

	int ret = 0;
	int wk = 1;
	int hwd = 0;
	int tbld = 0;
	int actd = 0;
	int ptd = 0;

sprintf(logf, "%s:ndp/devices.log", ddev[smode]);
psvDebugScreenPrintf("Logging to %s...\n\n", logf);

	while (wk == 1){
	sprintf(iexdev, "sdstor0:%s-%s-%s-%s", hw[hwd], tbl[tbld], act[actd], pt[ptd]);
	ret = kndpiex(iexdev);
	if (ret == 1){
	psvDebugScreenPrintf("%s exists\n", iexdev); 
	sprintf(lexdev, "ndp log: %s-%s-%s-%s exists\n", hw[hwd], tbl[tbld], act[actd], pt[ptd]);
	LOG(lexdev);} else psvDebugScreenPrintf("%s NOT exists\n", iexdev);
	hwd++;
	
	if (hwd == 7 && tbld == 1 && actd == 2 && ptd == 16) wk = 0;
	if (hwd == 7 && tbld == 1 && actd == 2) {ptd++; actd = 0; tbld = 0; hwd = 0; }
	if (hwd == 7 && tbld == 1) {actd++;tbld = 0; hwd = 0; }
	if (hwd == 7) {tbld++; hwd = 0;}
	}}

int main()
{
	psvDebugScreenInit();
	psvDebugScreenClear(COLOR_BLACK);
	psvDebugScreenSetFgColor(COLOR_GREEN);
	SceCtrlData pad;

SceUID mod_id;
tai_module_args_t argg;
psvDebugScreenPrintf("Preparing...\n");
	argg.size = sizeof(argg);
	argg.pid = KERNEL_PID;
	argg.args = 0;
	argg.argp = NULL;
	argg.flags = 0;
	mod_id = taiLoadStartKernelModuleForUser(MOD_PATH, &argg);
if (mod_id >= 0) {const char * const argv[] = { "restart", NULL }; sceAppMgrLoadExec("app0:eboot.bin", NULL, NULL);}
	extern int sceIoMkdir(const char *, int);
	sceIoMkdir("ux0:ndp/", 6);
	sceIoMkdir("uma0:ndp/", 6);
	sceIoMkdir("ur0:ndp/", 6);
	sceIoMkdir("xmc0:ndp/", 6);
	sceIoMkdir("imc0:ndp/", 6);
	int l = 1;// happens
	
	smn();
		
	while (l == 1) {
			sceCtrlPeekBufferPositive(0, &pad, 1);

			if (omd < 4){
			if (pad.buttons == SCE_CTRL_CROSS) {
				switch (sel){

					case 0:
						if (smd == 0 && omd == 0) ndpWorkDeviceMBRBased(emcblk, vshblk, "vs0", 0x4);
						if (smd == 1 && omd == 0) dmloc();
						if (omd == 1) ndpWorkDeviceMBRBased(gcdblk, grwblk, "grw0", 0xA);
						if (omd == 2) ndpWorkDeviceMBRBased(mcdblk, uxeblk, "ux0_mc", 0x8);
						if (omd == 3) fapcall(0);
					break;
					case 1:
						if (smd == 0 && omd == 0) ndpWorkDeviceMBRBased(emcblk, vsdblk, "vd0", 0x5);
						if (smd == 1 && omd == 0) ndpWorkDeviceMBRBased(emcblk, aosblk, "os0_act", 0x3);
						if (omd == 1) ndpWorkDeviceMBRBased(gcdblk, groblk, "gro0", 0x9);
						if (omd == 2) ndpWorkDeviceMBRBased(mcdblk, mmdblk, "mediaid_mc", 0xD);
						if (omd == 3) fapcall(1);
					break;
					case 2:
						if (smd == 0 && omd == 0) ndpWorkDeviceMBRBased(emcblk, trmblk, "tm0", 0x6);
						if (smd == 1 && omd == 0) ndpWorkDeviceMBRBased(emcblk, iosblk, "os0_ina", 0x3);
						if (omd == 1) ndpWorkDeviceMBRBased(gcdblk, gmdblk, "mediaid_gc", 0xD);
						if (omd == 2) ndpWorkDeviceDefault(mcdblk, "mbr_mc", 0x40000, 0, 0);
						if (omd == 3) fapcall(2);
					break;
					case 3:
						if (smd == 0 && omd == 0) ndpWorkDeviceMBRBased(emcblk, uriblk, "ur0", 0x7);
						if (smd == 1 && omd == 0) ndpWorkDeviceMBRBased(emcblk, ablblk, "slb2_act", 0x2);
						if (omd == 1) ndpWorkDeviceDefault(gcdblk, "mbr_gc", 0x200, 0, 0);
						if (omd == 2) scePowerRequestColdReset();
						if (omd == 3) fapcall(3);

					break;
					case 4:
						if (smd == 0 && omd == 0) ndpWorkDeviceMBRBased(emcblk, sysblk, "sa0", 0xC);
						if (smd == 1 && omd == 0) ndpWorkDeviceMBRBased(emcblk, iblblk, "slb2_ina", 0x2);
						if (omd == 1) scePowerRequestColdReset();
						if (omd == 2) sceKernelExitProcess(0);
						if (omd == 3) fapcall(4);

					break;
					case 5:
						if (smd == 0 && omd == 0) ndpWorkDeviceMBRBased(emcblk, pidblk, "pd0", 0xE);
						if (smd == 1 && omd == 0) ndpWorkDeviceMBRBased(emcblk, idsblk, "idstorage", 0x1);
						if (omd == 1) sceKernelExitProcess(0);
						if (omd == 3) fapcall(5);

					break;
					case 6:
						if (smd == 0 && omd == 0) {kndprw(); sel = 0;}
						if (smd == 1 && omd == 0) ndpWorkDeviceDefault(emcblk, "mbr", 0x40000, 0, 0);
						if (omd == 3) { ocdevsz = 0; omd = 4; }
					break;
					case 7:
						if (smd == 0 && omd == 0) sceKernelExitProcess(0);
						if (smd == 1 && omd == 0) scePowerRequestColdReset();
						if (omd == 3) omd = 5;
					break;
					case 8:
						if (omd == 3) brutefap(0);
					break;
					case 9:
						if (omd == 3) brutefap(1);
					break;
				}
				smn();
				sceKernelDelayThread(0.35 * 1000 * 1000);
			}}
			
			if (pad.buttons == SCE_CTRL_SELECT) { 
				if (omd == 0) ndpWorkDevBRBasedFull(emcblk, "emmc", 0);
				if (omd == 1) ndpWorkDevBRBasedFull(gcdblk, "gcd", 0);
				if (omd == 2) ndpWorkDevBRBasedFull(mcdblk, "mcd", 0);
				if (omd == 4) omd = 3;
				if (omd == 5) omd = 3;
				smn();
				sceKernelDelayThread(0.35 * 1000 * 1000);}


			if (pad.buttons == SCE_CTRL_START) {
				if (omd == 5) ndpReWriteDev();
				if (omd == 4) fapcall(69);
				smn();
				sceKernelDelayThread(0.35 * 1000 * 1000);}


			if (pad.buttons == SCE_CTRL_RTRIGGER) {
			if(omd < 4){
				if (wmode == 1){
					if (opmode == 1) { opmode = 0;
					} else { opmode = 1; }sceKernelDelayThread(0.35 * 1000 * 1000);smn();}
				if (wmode == 2){
					if (opmode == 1) { opmode = 0;
					} else { opmode = 1; }sceKernelDelayThread(0.35 * 1000 * 1000);smn();}}
			if (omd == 4) {ocdevsz = 0;smn();}}
			if (pad.buttons == SCE_CTRL_LTRIGGER) {
			if(omd < 4){
				if (smd == 1) { smd = 0;
			} else { smd = 1; }sceKernelDelayThread(0.35 * 1000 * 1000);smn();}}
			
			if (pad.buttons == SCE_CTRL_UP) {
			if (omd < 4){
				if(sel!=0){
					sel--;
					sub_sel = 0;
				}
				smn();
				sceKernelDelayThread(0.3 * 1000 * 1000);}
			if (omd == 4){
				ocdevsz++;
				smn();
				sceKernelDelayThread(0.3 * 1000 * 1000);}
			}
			
			if (pad.buttons == SCE_CTRL_DOWN) {
			if (omd < 4){
				if(sel+1<item_count){
					sel++;
					sub_sel = 0;
				}
				smn();
				sceKernelDelayThread(0.3 * 1000 * 1000);}
			if (omd == 4){
				ocdevsz--;
				smn();
				sceKernelDelayThread(0.3 * 1000 * 1000);}
			}

			if (pad.buttons == WMODECG) {
					wmode = 2;
				smn();
				sceKernelDelayThread(0.35 * 1000 * 1000);
			}
			if (pad.buttons == SMODECGR) {
				if(smode+1<smodes){
				smode++;
				}
				if(smode==5){
					smode = 0;
				}
				smn();
				sceKernelDelayThread(0.35 * 1000 * 1000);
			}
			if (pad.buttons == SMODECGL) {
				if(smode!=0){
				smode--;
				}
				smn();
				sceKernelDelayThread(0.35 * 1000 * 1000);
			}
			if (pad.buttons == SZODECGR) {
				if(szode!=2){
					szode++;
				}
				smn();
				sceKernelDelayThread(0.35 * 1000 * 1000);
			}
			if (pad.buttons == SZODECGL) {
				if(szode!=0){
				szode--;
				}
				smn();
				sceKernelDelayThread(0.35 * 1000 * 1000);
			}
			if (pad.buttons == MDODECGR) {
				if(omd!=3){
					omd++;
				}
				smn();
				sceKernelDelayThread(0.35 * 1000 * 1000);
			}
			if (pad.buttons == MDODECGL) {
				if(omd!=0){
				omd--;
				}
				smn();
				sceKernelDelayThread(0.35 * 1000 * 1000);
			}


			if (pad.buttons == PDEVSZIX) {
				if(omd==4){
				ocdevsz = ocdevsz + 0x10;
				smn();
				sceKernelDelayThread(0.35 * 1000 * 1000);}
			}
			if (pad.buttons == PDEVSZXX) {
				if(omd==4){
				ocdevsz = ocdevsz + 0x100;
				smn();
				sceKernelDelayThread(0.35 * 1000 * 1000);}
			}
			if (pad.buttons == PDEVSZXC) {
				if(omd==4){
				ocdevsz = ocdevsz + 0x1000;
				smn();
				sceKernelDelayThread(0.35 * 1000 * 1000);}
			}
			if (pad.buttons == PDEVSZCC) {
				if(omd==4){
				ocdevsz = ocdevsz + 0x10000;
				smn();
				sceKernelDelayThread(0.35 * 1000 * 1000);}
			}
			if (pad.buttons == PDEVSZCM) {
				if(omd==4){
				ocdevsz = ocdevsz + 0x100000;
				smn();
				sceKernelDelayThread(0.35 * 1000 * 1000);}
				if(omd==5){
				if (cinf+1 != 7) cinf++;
				smn();
				sceKernelDelayThread(0.35 * 1000 * 1000);}
			}
			if (pad.buttons == PDEVBLKO) {
				if(omd==5){
				if (couf+1 != 7) couf++;
				smn();
				sceKernelDelayThread(0.35 * 1000 * 1000);}
			}

			if (pad.buttons == MDEVSZIX) {
				if(omd==4){
				ocdevsz = ocdevsz - 0x10;
				smn();
				sceKernelDelayThread(0.35 * 1000 * 1000);}
			}
			if (pad.buttons == MDEVSZXX) {
				if(omd==4){
				ocdevsz = ocdevsz - 0x100;
				smn();
				sceKernelDelayThread(0.35 * 1000 * 1000);}
			}
			if (pad.buttons == MDEVSZXC) {
				if(omd==4){
				ocdevsz = ocdevsz - 0x1000;
				smn();
				sceKernelDelayThread(0.35 * 1000 * 1000);}
			}
			if (pad.buttons == MDEVSZCC) {
				if(omd==4){
				ocdevsz = ocdevsz - 0x10000;
				smn();
				sceKernelDelayThread(0.35 * 1000 * 1000);}
			}
			if (pad.buttons == MDEVSZCM) {
				if(omd==4){
				ocdevsz = ocdevsz - 0x100000;
				smn();
				sceKernelDelayThread(0.35 * 1000 * 1000);}
				if(omd==5){
				if (cinf > 0) cinf--;
				smn();
				sceKernelDelayThread(0.35 * 1000 * 1000);}
			}
			if (pad.buttons == MDEVBLKO) {
				if(omd==5){
				if (wmode == 0){if (couf > 1) couf--;}
				if (wmode == 2){if (couf > 0) couf--;}
				smn();
				sceKernelDelayThread(0.35 * 1000 * 1000);}
			}
	}
	
	sceKernelDelayThread(10 * 1000 * 1000);
	sceKernelExitProcess(0);
    return 0;
}
