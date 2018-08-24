/*
	NDP by SKGleba
	All Rights Reserved
*/

#include <psp2kern/kernel/cpu.h>
#include <psp2kern/kernel/modulemgr.h>
#include <psp2kern/kernel/sysmem.h>
#include <psp2kern/io/fcntl.h>
#include <psp2kern/kernel/threadmgr.h>

#include <stdio.h>
#include <string.h>

static char cinp[128];
static char coup[128];

typedef struct{
	const char* inp;
	const char* oup;
	int crt;
	int devsz;
	int islc;
	int oslc;
	int dsz;
}kndps;

typedef struct{
int arg1;
int arg2;
int arg3;
}kndpb;

volatile kndps dt;
volatile kndpb tb;

int ex(const char* filloc){
  SceUID fd;
  fd = ksceIoOpen(filloc, SCE_O_RDONLY, 0777);
  if (fd < 0) {ksceIoClose(fd); return 0;}
ksceIoClose(fd); return 1;}

int siofix(void *func) {
	int ret = 0;
	int res = 0;
	int uid = 0;
	ret = uid = ksceKernelCreateThread("siofix", func, 64, 0x10000, 0, 0, 0);
	if (ret < 0){ret = -1; goto cleanup;}
	if ((ret = ksceKernelStartThread(uid, 0, NULL)) < 0) {ret = -1; goto cleanup;}
	if ((ret = ksceKernelWaitThreadEnd(uid, &res, NULL)) < 0) {ret = -1; goto cleanup;}
	ret = res;
cleanup:
	if (uid > 0) ksceKernelDeleteThread(uid);
	return ret;}

int workdev(){
if (ex(dt.inp) == 1){
  SceUID fd;
  SceUID wfd;
  fd = ksceIoOpen(dt.inp, SCE_O_RDONLY, 0777);
if (dt.crt == 0) wfd = ksceIoOpen(dt.oup, SCE_O_RDWR, 0777);
if (dt.crt == 1) wfd = ksceIoOpen(dt.oup, SCE_O_WRONLY | SCE_O_CREAT | SCE_O_APPEND, 6);
ksceIoLseek(fd, dt.islc, SCE_SEEK_SET);
ksceIoLseek(wfd, dt.oslc, SCE_SEEK_SET);
unsigned int i = 0;
if (dt.devsz == 0x200){
dt.dsz = 69;
static char ubuf[0x200];
  ksceIoRead(fd, ubuf, 0x200);
  ksceIoWrite(wfd, ubuf, 0x200);
if (fd > 0) ksceIoClose(fd);
if (wfd > 0) ksceIoClose(wfd);}
if (dt.dsz == 0){
static char ubuf[0x2000];
 for(i=0;i<dt.devsz;i=i+0x2000){
  ksceIoRead(fd, ubuf, 0x2000);
  ksceIoWrite(wfd, ubuf, 0x2000);}
if (fd > 0) ksceIoClose(fd);
if (wfd > 0) ksceIoClose(wfd);}
if (dt.dsz == 1){
static char ubuf[0x4000];
 for(i=0;i<dt.devsz;i=i+0x4000){
  ksceIoRead(fd, ubuf, 0x4000);
  ksceIoWrite(wfd, ubuf, 0x4000);}
if (fd > 0) ksceIoClose(fd);
if (wfd > 0) ksceIoClose(wfd);}
if (dt.dsz == 2){
static char ubuf[0x8000];
 for(i=0;i<dt.devsz;i=i+0x8000){
  ksceIoRead(fd, ubuf, 0x8000);
  ksceIoWrite(wfd, ubuf, 0x8000);}
if (fd > 0) ksceIoClose(fd);
if (wfd > 0) ksceIoClose(wfd);}
return 1;}
return 0;}

int deviex(){
dt.crt = ex(dt.inp);}

int kndprw(){
	int i;
	for (i = 0; i < 15; i++) {
		ksceIoUmount(i * 0x100, 0, 0, 0);
		ksceIoMount(i * 0x100, NULL, 2, 0, 0, 0);}}

int kndpiex(const char* rinp){
int ret = 0, state = 0;
ENTER_SYSCALL(state);
ksceKernelStrncpyUserToKernel(cinp, (uintptr_t)rinp, 128);
dt.inp = cinp;
siofix(deviex);
ret = dt.crt;
EXIT_SYSCALL(state);
return ret;}

kndpWriteBuffer(int arg, int bufnum){
	int state = 0;
	ENTER_SYSCALL(state);
	if (bufnum == 1) tb.arg1 = arg;
	if (bufnum == 2) tb.arg2 = arg;
	if (bufnum == 3) tb.arg3 = arg;
	if (bufnum == 0){tb.arg1, tb.arg2, tb.arg3 = arg;}
	EXIT_SYSCALL(state);
return 1;}


kndpReadBuffer(int bufnum){
	int ret = 0, state = 0;
	ENTER_SYSCALL(state);
	if (bufnum == 1) ret = tb.arg1;
	if (bufnum == 2) ret = tb.arg2;
	if (bufnum == 3) ret = tb.arg3;
	EXIT_SYSCALL(state);
return ret;}


int kndpWorkDevice(const char* rinp, const char* roup, int rcrt, int rdevsz, int rislc, int roslc, int rdsz){
	int ret = 0, state = 0;
	ENTER_SYSCALL(state);
	ksceKernelStrncpyUserToKernel(cinp, (uintptr_t)rinp, 128);
	ksceKernelStrncpyUserToKernel(coup, (uintptr_t)roup, 128);
		dt.inp = cinp;
		dt.oup = coup;
		dt.crt = rcrt;
		dt.devsz = rdevsz;
		dt.islc = rislc;
		dt.oslc = roslc;
		dt.dsz = rdsz;
	ret = siofix(workdev);
	EXIT_SYSCALL(state);
return ret;}

void _start() __attribute__ ((weak, alias("module_start")));
int module_start(SceSize args, void *argp) {
	return SCE_KERNEL_START_SUCCESS;
}

int module_stop(SceSize args, void *argp) {
	return SCE_KERNEL_STOP_SUCCESS;
}
