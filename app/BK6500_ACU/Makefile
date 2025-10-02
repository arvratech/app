##### File Definition ####
PRJ  = BK6500
MAIN = BK6000
INIT = 2440init
AM1  = 2440slib
AM2  = heap
CM1  = 2440lib
CM2  = mmu
CM3  = flash
CM4  = flashprim
CM5  = bmoncmd0

#### Destination path Definition ####
OBJ=.\obj
ERR=.\err
CSRCL=..\BootLoader
CSRCM=..\BootMonitor
CSRCC=..\BKCom

#### ARM tool Definition ####
ARMLINK= armlink
ARMASM = armasm
ARMCC  = armcc

#### Option Definition ####
LFLAGS  = -ro-base 0x30000000 -elf -map -xref -list list.txt -first $(INIT).o(Init)
AFLAGS  = -PD "BOOT_MONITOR SETL {FALSE}" -PD "SDRAMSIZE SETA 32" -li -apcs /noswst -cpu ARM920T
CFLAGS0 = -c -g- -li -apcs /noswst -cpu ARM920T -I $(CSRCC)
CFLAGS1 = -c -DBK6000 -D_KOR -g- -li -apcs /noswst -cpu ARM920T -I $(CSRCC)
CFLAGS2 = -c -DBK6000 -g- -li -apcs /noswst -cpu ARM920T -I $(CSRCC)
CFLAGS3 = -c -DBK6000 -DSECUGEN -g- -li -apcs /noswst -cpu ARM920T -I $(CSRCC)
CFLAGS4 = -c -DBK6000 -DSDRAMSIZE_32 -g- -li -apcs /noswst -cpu ARM920T -I $(CSRCC)
CFLAGS6 = -c -DUSED_NTSC -g- -li -apcs /noswst -cpu ARM920T -I $(CSRCC)
CFLAGS7 = -c -DCPU2440A -D_FLASH_128 -DSDRAMCLK_100 -g- -li -apcs /noswst -cpu ARM920T -I $(CSRCC)
#CFLAGS = -c -g+ -li -apcs /noswst -cpu ARM920T
#CFLAGS = -c -g+ -bi -apcs /noswst -cpu ARM920T
#AFLAGS = -bi -apcs /noswst -cpu ARM920T

#### Object combine Definition ####
OBJS = $(OBJ)\$(INIT).o $(OBJ)\$(AM1).o $(OBJ)\$(AM2).o $(OBJ)\$(MAIN).o $(OBJ)\$(CM1).o $(OBJ)\$(CM2).o \
	$(OBJ)\$(CM3).o $(OBJ)\$(CM4).o $(OBJ)\$(CM5).o \
	$(OBJ)\toptask.o $(OBJ)\topprim.o $(OBJ)\menuprim.o \
	$(OBJ)\adm_top.o $(OBJ)\adm_usr.o $(OBJ)\adm_ac.o $(OBJ)\adm_net.o $(OBJ)\adm_set.o $(OBJ)\adm_sys.o $(OBJ)\admprim.o \
	$(OBJ)\admenroll.o $(OBJ)\auth.o $(OBJ)\authprim.o $(OBJ)\admcon.o \
	$(OBJ)\desktop.o $(OBJ)\wnds.o $(OBJ)\fpimage.o \
	$(OBJ)\sinport.o $(OBJ)\svrcmd.o $(OBJ)\svr.o $(OBJ)\msgsvr.o $(OBJ)\net.o \
	$(OBJ)\cli.o $(OBJ)\slave.o \
	$(OBJ)\user.o $(OBJ)\userauth.o $(OBJ)\fsuser.o $(OBJ)\acar.o $(OBJ)\fsar.o \
	$(OBJ)\event.o $(OBJ)\evt.o $(OBJ)\sche.o \
	$(OBJ)\devprim.o $(OBJ)\unit.o $(OBJ)\acad.o $(OBJ)\acap.o $(OBJ)\acaz.o $(OBJ)\bio.o $(OBJ)\alm.o $(OBJ)\fire.o \
	$(OBJ)\syscfg.o $(OBJ)\sysdep.o $(OBJ)\console.o $(OBJ)\lang.o $(OBJ)\msg.o $(OBJ)\ta.o \
	$(OBJ)\prim.o $(OBJ)\cotask.o $(OBJ)\cbuf.o \
	$(OBJ)\rect.o $(OBJ)\gwnd.o $(OBJ)\gmenu.o $(OBJ)\gmenuc.o $(OBJ)\glabel.o $(OBJ)\gedit.o \
	$(OBJ)\gchkradio.o $(OBJ)\gipstring.o $(OBJ)\gdate.o $(OBJ)\gipaddr.o \
	$(OBJ)\symbol.o $(OBJ)\gfont.o $(OBJ)\bmpc.o $(OBJ)\eng16.o \
	$(OBJ)\fs_flash.o $(OBJ)\fs.o $(OBJ)\sdi.o $(OBJ)\diskio.o $(OBJ)\ff.o \
	$(OBJ)\rtc.o $(OBJ)\rtccpu.o $(OBJ)\iodev.o $(OBJ)\wiegand.o $(OBJ)\key.o $(OBJ)\lcdc.o \
	$(OBJ)\sio.o $(OBJ)\sin.o $(OBJ)\ping.o $(OBJ)\dhcp.o \
	$(OBJ)\cr.o $(OBJ)\fpapi_new.o $(OBJ)\usbd.o \
	$(OBJ)\pno.o \
	
all: $(PRJ).axf

clean:
	del $(OBJ)\*.o
	del $(ERR)\*.err
	del $(PRJ).axf
	del *.bin
	
$(PRJ).axf: $(OBJS) 
	$(ARMLINK) $(LFLAGS) -o $(PRJ).axf $(OBJS) $(CSRCC)\fpmgr_kdk.a
	fromelf -nodebug -bin -output $(PRJ).bin $(PRJ).axf
#	del $(PRJ).axf

#	fromelf  $(PRJ).axf -text/s  syms.sym
#	fromelf  $(PRJ).axf -text/c  symc.sym

#For SDT2.5 fromelf -nodebug -nozeropad $(PRJ).elf -bin $(PRJ).bin 
#For ADS1.0 fromelf -nodebug -bin -output $(PRJ).bin $(PRJ).elf


$(OBJ)\$(MAIN).o:  $(MAIN).c  $(CSRCL)\2440addr.h $(CSRCL)\2440lib.h
	$(ARMCC) $(CFLAGS2) $(MAIN).c -o $(OBJ)\$(MAIN).o -Errors $(ERR)\$(MAIN).err
 
$(OBJ)\$(INIT).o: $(CSRCM)\$(INIT).s $(CSRCM)\2440Option.inc $(CSRCL)\2440addr.inc 
	copy $(CSRCL)\2440addr.inc 2440addr.inc
	copy $(CSRCM)\2440Option.inc 2440Option.inc
	copy $(CSRCM)\2440init.s 2440init.s
	$(ARMASM) $(AFLAGS) $(INIT).s -o $(OBJ)\$(INIT).o -Errors $(ERR)\$(INIT).err
	del 2440init.s
	del 2440Option.inc
	del 2440addr.inc

$(OBJ)\$(AM1).o:  $(CSRCM)\$(AM1).s $(CSRCL)\$(AM1).h
	copy $(CSRCM)\$(AM1).s $(AM1).s
	$(ARMASM) $(AFLAGS) $(AM1).s -o $(OBJ)\$(AM1).o -Errors $(ERR)\$(AM1).err
	del $(AM1).s

$(OBJ)\$(AM2).o:  $(CSRCM)\$(AM2).s
	copy $(CSRCM)\$(AM2).s $(AM2).s
	$(ARMASM) $(AFLAGS) $(AM2).s -o $(OBJ)\$(AM2).o -Errors $(ERR)\$(AM2).err
	del $(AM2).s

$(OBJ)\$(CM1).o:  $(CSRCM)\$(CM1).c $(CSRCL)\2440addr.h $(CSRCL)\2440lib.h
	copy $(CSRCM)\$(CM1).c $(CM1).c
	$(ARMCC) $(CFLAGS7) $(CM1).c -o $(OBJ)\$(CM1).o -Errors $(ERR)\$(CM1).err
	del $(CM1).C
	
$(OBJ)\$(CM2).o:  $(CSRCC)\$(CM2)_2440.c $(CSRCC)\$(CM2).h $(CSRCL)\2440addr.h $(CSRCL)\2440lib.h
	copy $(CSRCC)\$(CM2)_2440.c $(CM2).c
	$(ARMCC) $(CFLAGS4) $(CM2).c -o $(OBJ)\$(CM2).o -Errors $(ERR)\$(CM2).err
	del $(CM2).C

$(OBJ)\$(CM3).o:  $(CSRCM)\$(CM3).c $(CSRCL)\$(CM3).h $(CSRCM)\$(CM3)2440_nand.c
	copy $(CSRCM)\$(CM3).c $(CM3).c
	copy $(CSRCM)\$(CM3)2440_nand.c $(CM3)2440_nand.c
	$(ARMCC) $(CFLAGS7) $(CM3).c -o $(OBJ)\$(CM3).o -Errors $(ERR)\$(CM3).err
	del $(CM3)2440_nand.c
	del $(CM3).c

$(OBJ)\$(CM4).o:  $(CSRCM)\$(CM4).c $(CSRCL)\$(CM4).h
	copy $(CSRCM)\$(CM4).c $(CM4).c
	$(ARMCC) $(CFLAGS7) $(CM4).c -o $(OBJ)\$(CM4).o -Errors $(ERR)\$(CM4).err
	del $(CM4).c

$(OBJ)\$(CM5).o:  $(CSRCM)\$(CM5).c $(CSRCM)\bmoncmd.h
	copy $(CSRCM)\$(CM5).c $(CM5).c
	$(ARMCC) $(CFLAGS7) $(CM5).c -o $(OBJ)\$(CM5).o -Errors $(ERR)\$(CM5).err
	del $(CM5).c

$(OBJ)\toptask.o:  toptask.c toptask_svr.c toptask_sin.c toptask_ni.c toptask_con.c toptask_unit.c toptask.h
	$(ARMCC) $(CFLAGS2) toptask.c -o $(OBJ)\toptask.o -Errors $(ERR)\toptask.err

$(OBJ)\topprim.o:  topprim.c topprim.h
	$(ARMCC) $(CFLAGS4) topprim.c -o $(OBJ)\topprim.o -Errors $(ERR)\topprim.err

$(OBJ)\menuprim.o:  menuprim.c menuprim.h
	$(ARMCC) $(CFLAGS2) menuprim.c -o $(OBJ)\menuprim.o -Errors $(ERR)\menuprim.err

$(OBJ)\adm_top.o:  adm_top.c adm.h
	$(ARMCC) $(CFLAGS0) adm_top.c -o $(OBJ)\adm_top.o -Errors $(ERR)\adm_top.err

$(OBJ)\adm_usr.o:  adm_usr.c adm.h
	$(ARMCC) $(CFLAGS2) adm_usr.c -o $(OBJ)\adm_usr.o -Errors $(ERR)\adm_usr.err

$(OBJ)\adm_ac.o:  adm_ac.c adm_ac_hw.c adm_ac_hw_item.c adm_ac_hw_view.c adm_ac_cr.c adm_ac_ad.c adm_ac_ap.c adm_ac_in.c adm_ac_out.c adm_ac_az.c adm_ac_fz.c adm_ac_act.c adm_ac_oc.c adm_ac_ta.c adm.h
	$(ARMCC) $(CFLAGS2) adm_ac.c -o $(OBJ)\adm_ac.o -Errors $(ERR)\adm_ac.err

$(OBJ)\adm_net.o:  adm_net.c adm.h
	$(ARMCC) $(CFLAGS0) adm_net.c -o $(OBJ)\adm_net.o -Errors $(ERR)\adm_net.err

$(OBJ)\adm_set.o:  adm_set.c adm_set_cam.c adm.h
	$(ARMCC) $(CFLAGS2) adm_set.c -o $(OBJ)\adm_set.o -Errors $(ERR)\adm_set.err

$(OBJ)\adm_sys.o:  adm_sys.c adm_sys_init.c adm_sys_sd.c adm_sys_stat.c adm_sys_test.c adm.h
	$(ARMCC) $(CFLAGS2) adm_sys.c -o $(OBJ)\adm_sys.o -Errors $(ERR)\adm_sys.err

$(OBJ)\admprim.o:  admprim.c admprim.h
	$(ARMCC) $(CFLAGS2) admprim.c -o $(OBJ)\admprim.o -Errors $(ERR)\admprim.err

$(OBJ)\admenroll.o:  admenroll.c admenroll_prim.c adm.h
	$(ARMCC) $(CFLAGS2) admenroll.c -o $(OBJ)\admenroll.o -Errors $(ERR)\admenroll.err

$(OBJ)\auth.o:  auth.c auth.h
	$(ARMCC) $(CFLAGS2) auth.c -o $(OBJ)\auth.o -Errors $(ERR)\auth.err

$(OBJ)\authprim.o:  authprim.c auth.h
	$(ARMCC) $(CFLAGS2) authprim.c -o $(OBJ)\authprim.o -Errors $(ERR)\authprim.err

$(OBJ)\admcon.o:  admcon.c admcon.h
	$(ARMCC) $(CFLAGS1) admcon.c -o $(OBJ)\admcon.o -Errors $(ERR)\admcon.err

$(OBJ)\desktop.o:  desktop.c desktop_modu.c desktop_icon.c desktop.h
	$(ARMCC) $(CFLAGS1) desktop.c -o $(OBJ)\desktop.o -Errors $(ERR)\desktop.err

$(OBJ)\wnds.o:  wnds.c wnds.h wnds_uid.c wnds_num.c wnds_date.c wnds_ipaddr.c wnds_phone.c
	$(ARMCC) $(CFLAGS1) wnds.c -o $(OBJ)\wnds.o -Errors $(ERR)\wnds.err

$(OBJ)\fpimage.o:  fpimage_sg.c fpimage.h
	$(ARMCC) $(CFLAGS0) fpimage_sg.c -o $(OBJ)\fpimage.o -Errors $(ERR)\fpimage.err

$(OBJ)\sinport.o:  $(CSRCC)\sinport.c $(CSRCC)\sinport.h
	copy  $(CSRCC)\sinport.c sinport.c
	$(ARMCC) $(CFLAGS0) sinport.c -o $(OBJ)\sinport.o -Errors $(ERR)\sinport.err
	del sinport.c

$(OBJ)\svrcmd.o:  svrcmd.c svrcmd.h svrcmd_read.c svrcmd_write.c  svrcmd_del.c svrcmd_rdlist.c svrcmd_wrlist.c svrcmd_etc.c
	$(ARMCC) $(CFLAGS2) svrcmd.c -o $(OBJ)\svrcmd.o -Errors $(ERR)\svrcmd.err

$(OBJ)\svr.o:  svr.c svr.h
	$(ARMCC) $(CFLAGS0) svr.c -o $(OBJ)\svr.o -Errors $(ERR)\svr.err

$(OBJ)\msgsvr.o:  msgsvr.c msgsvr.h
	$(ARMCC) $(CFLAGS0) msgsvr.c -o $(OBJ)\msgsvr.o -Errors $(ERR)\msgsvr.err

$(OBJ)\net.o:  net.c net.h
	$(ARMCC) $(CFLAGS0) net.c -o $(OBJ)\net.o -Errors $(ERR)\net.err

$(OBJ)\cli.o:  cli.c cli_req.c cli_xfer.c cli.h
	$(ARMCC) $(CFLAGS0) cli.c -o $(OBJ)\cli.o -Errors $(ERR)\cli.err

$(OBJ)\slave.o:  slave.c slave_read.c slave_write.c slave.h
	$(ARMCC) $(CFLAGS0) slave.c -o $(OBJ)\slave.o -Errors $(ERR)\slave.err

$(OBJ)\user.o:  user_sg.c user.h usercfg.c
	$(ARMCC) $(CFLAGS0) user_sg.c -o $(OBJ)\user.o -Errors $(ERR)\user.err

$(OBJ)\userauth.o:  userauth_sg.c user.h
	$(ARMCC) $(CFLAGS0) userauth_sg.c -o $(OBJ)\userauth.o -Errors $(ERR)\userauth.err

$(OBJ)\fsuser.o:  fsuser.c fsuser.h fsuser_fp.c fsuser_ex.c fsuser_photo.c fsuser_txn.c fsevt.c fsuser_ar.c fspno.c fscamimg.c
	$(ARMCC) $(CFLAGS2) fsuser.c -o $(OBJ)\fsuser.o -Errors $(ERR)\fsuser.err

$(OBJ)\acar.o:  acar.c acar.h acarcfg.c
	$(ARMCC) $(CFLAGS0) acar.c -o $(OBJ)\acar.o -Errors $(ERR)\acar.err

$(OBJ)\fsar.o:  fsar.c fsar.h fscal.c fssche.c
	$(ARMCC) $(CFLAGS0) fsar.c -o $(OBJ)\fsar.o -Errors $(ERR)\fsar.err

$(OBJ)\event.o:  event.c event.h
	$(ARMCC) $(CFLAGS2) event.c -o $(OBJ)\event.o -Errors $(ERR)\event.err

$(OBJ)\evt.o: evt.c evt.h
	$(ARMCC) $(CFLAGS2) evt.c -o $(OBJ)\evt.o -Errors $(ERR)\evt.err

$(OBJ)\sche.o:  sche.c sche.h cal.c cal.h schecfg.c calcfg.c
	$(ARMCC) $(CFLAGS0) sche.c -o $(OBJ)\sche.o -Errors $(ERR)\sche.err

$(OBJ)\devprim.o:  devprim.c devprim.h
	$(ARMCC) $(CFLAGS0) devprim.c -o $(OBJ)\devprim.o -Errors $(ERR)\devprim.err

$(OBJ)\unit.o:  unit.c unitprim.c unitcfg.c unit.h unitcfg.h
	$(ARMCC) $(CFLAGS2) unit.c -o $(OBJ)\unit.o -Errors $(ERR)\unit.err

$(OBJ)\acad.o:  acad.c acad.h acadcfg.c acadcfg.h
	$(ARMCC) $(CFLAGS0) acad.c -o $(OBJ)\acad.o -Errors $(ERR)\acad.err

$(OBJ)\acap.o:  acap.c acap.h acapcfg.c acapcfg.h
	$(ARMCC) $(CFLAGS0) acap.c -o $(OBJ)\acap.o -Errors $(ERR)\acap.err

$(OBJ)\acaz.o:  acaz.c acaz.h acazcfg.c acazcfg.h
	$(ARMCC) $(CFLAGS0) acaz.c -o $(OBJ)\acaz.o -Errors $(ERR)\acaz.err

$(OBJ)\bio.o:  bio.c bio.h biocfg.c biocfg.h
	$(ARMCC) $(CFLAGS0) bio.c -o $(OBJ)\bio.o -Errors $(ERR)\bio.err

$(OBJ)\alm.o:  alm.c alm.h almcfg.c almcfg.h
	$(ARMCC) $(CFLAGS0) alm.c -o $(OBJ)\alm.o -Errors $(ERR)\alm.err

$(OBJ)\fire.o:  fire.c fire.h firecfg.c firecfg.h
	$(ARMCC) $(CFLAGS0) fire.c -o $(OBJ)\fire.o -Errors $(ERR)\fire.err

$(OBJ)\syscfg.o:  syscfg.c syscfg.h devcfg.c nwpcfg.c camcfg.c
	$(ARMCC) $(CFLAGS2) syscfg.c -o $(OBJ)\syscfg.o -Errors $(ERR)\syscfg.err

$(OBJ)\sysdep.o:  sysdep.c sysdep.h
	$(ARMCC) $(CFLAGS2) sysdep.c -o $(OBJ)\sysdep.o -Errors $(ERR)\sysdep.err

$(OBJ)\console.o:  $(CSRCC)\console.c $(CSRCC)\console.h
	copy  $(CSRCC)\console.c console.c
	$(ARMCC) $(CFLAGS0) console.c -o $(OBJ)\console.o -Errors $(ERR)\console.err
	del console.c

$(OBJ)\lang.o:  lang.c $(CSRCC)\lang.h
	$(ARMCC) $(CFLAGS1) lang.c -o $(OBJ)\lang.o -Errors $(ERR)\lang.err

$(OBJ)\msg.o:  msg.c msg.h msgeng.c msgkor.c
	$(ARMCC) $(CFLAGS1) msg.c -o $(OBJ)\msg.o -Errors $(ERR)\msg.err

$(OBJ)\ta.o:  ta.c ta.h
	$(ARMCC) $(CFLAGS1) ta.c -o $(OBJ)\ta.o -Errors $(ERR)\ta.err

$(OBJ)\prim.o:  $(CSRCC)\prim.c $(CSRCC)\prim.h
	copy  $(CSRCC)\prim.c prim.c
	$(ARMCC) $(CFLAGS0) prim.c -o $(OBJ)\prim.o -Errors $(ERR)\prim.err
	del prim.c

$(OBJ)\cotask.o:  $(CSRCC)\cotask.c $(CSRCC)\cotask.h
	copy  $(CSRCC)\cotask.c cotask.c
	$(ARMCC) $(CFLAGS0) cotask.c -o $(OBJ)\cotask.o -Errors $(ERR)\cotask.err
	del cotask.c

$(OBJ)\cbuf.o:  $(CSRCC)\cbuf.c $(CSRCC)\cbuf.h
	copy  $(CSRCC)\cbuf.c cbuf.c
	$(ARMCC) $(CFLAGS0) cbuf.c -o $(OBJ)\cbuf.o -Errors $(ERR)\cbuf.err
	del cbuf.c

$(OBJ)\rect.o:  $(CSRCC)\rect.c $(CSRCC)\rect.h
	copy $(CSRCC)\rect.c rect.c
	$(ARMCC) $(CFLAGS0) rect.c -o $(OBJ)\rect.o -Errors $(ERR)\rect.err
	del rect.c

$(OBJ)\gwnd.o:  $(CSRCC)\gwnd.c $(CSRCC)\gwnd.h
	copy $(CSRCC)\gwnd.c gwnd.c
	$(ARMCC) $(CFLAGS0) gwnd.c -o $(OBJ)\gwnd.o -Errors $(ERR)\gwnd.err
	del gwnd.c

$(OBJ)\gmenu.o:  $(CSRCC)\gmenu_new.c $(CSRCC)\gmenu.h
	copy $(CSRCC)\gmenu_new.c gmenu.c
	$(ARMCC) $(CFLAGS0) gmenu.c -o $(OBJ)\gmenu.o -Errors $(ERR)\gmenu.err
	del gmenu.c

$(OBJ)\gmenuc.o:  $(CSRCC)\gmenuc0.c $(CSRCC)\gmenuc1.c $(CSRCC)\gmenuc.h
	copy $(CSRCC)\gmenuc0.c gmenuc0.c
	copy $(CSRCC)\gmenuc1.c gmenuc1.c
	$(ARMCC) $(CFLAGS0) gmenuc0.c -o $(OBJ)\gmenuc.o -Errors $(ERR)\gmenuc.err
	del gmenuc0.c gmenuc1.c

$(OBJ)\glabel.o:  $(CSRCC)\glabel.c $(CSRCC)\glabel.h
	copy $(CSRCC)\glabel.c glabel.c
	$(ARMCC) $(CFLAGS0) glabel.c -o $(OBJ)\glabel.o -Errors $(ERR)\glabel.err
	del glabel.c

$(OBJ)\gedit.o:  $(CSRCC)\gedit.c $(CSRCC)\gedit.h
	copy $(CSRCC)\gedit.c gedit.c
	$(ARMCC) $(CFLAGS0) gedit.c -o $(OBJ)\gedit.o -Errors $(ERR)\gedit.err
	del gedit.c
	
$(OBJ)\gchkradio.o:  $(CSRCC)\gchkradio.c $(CSRCC)\gcheck.h $(CSRCC)\gradio.h
	copy $(CSRCC)\gchkradio.c gchkradio.c
	$(ARMCC) $(CFLAGS0) gchkradio.c -o $(OBJ)\gchkradio.o -Errors $(ERR)\gchkradio.err
	del gchkradio.c

$(OBJ)\gipstring.o:  $(CSRCC)\gipstring.c $(CSRCC)\gipstring.h
	copy $(CSRCC)\gipstring.c gipstring.c
	$(ARMCC) $(CFLAGS0) gipstring.c -o $(OBJ)\gipstring.o -Errors $(ERR)\gipstring.err
	del gipstring.c

$(OBJ)\gdate.o:  $(CSRCC)\gdate.c $(CSRCC)\gdate.h
	copy $(CSRCC)\gdate.c gdate.c
	$(ARMCC) $(CFLAGS0) gdate.c -o $(OBJ)\gdate.o -Errors $(ERR)\gdate.err
	del gdate.c

$(OBJ)\gipaddr.o:  $(CSRCC)\gipaddr.c $(CSRCC)\gipaddr.h
	copy $(CSRCC)\gipaddr.c gipaddr.c
	$(ARMCC) $(CFLAGS0) gipaddr.c -o $(OBJ)\gipaddr.o -Errors $(ERR)\gipaddr.err
	del gipaddr.c

$(OBJ)\symbol.o:  symbol.c symbol.h
	$(ARMCC) $(CFLAGS0) symbol.c -o $(OBJ)\symbol.o -Errors $(ERR)\symbol.err

$(OBJ)\gfont.o:  $(CSRCC)\gfont.c $(CSRCC)\gfont.h $(CSRCC)\gfonteng.c $(CSRCC)\gfontkor.c $(CSRCC)\gfontjpn.c
	copy $(CSRCC)\gfont.c gfont.c	
	$(ARMCC) $(CFLAGS1) gfont.c -o $(OBJ)\gfont.o -Errors $(ERR)\gfont.err
	del gfont.c

$(OBJ)\bmpc.o:  $(CSRCC)\bmpc.c $(CSRCC)\bmpc.h
	copy  $(CSRCC)\bmpc.c bmpc.c
	$(ARMCC) $(CFLAGS0) bmpc.c -o $(OBJ)\bmpc.o -Errors $(ERR)\bmpc.err
	del bmpc.c

$(OBJ)\eng16.o:  eng16.c eng16.h
	$(ARMCC) $(CFLAGS0) eng16.c -o $(OBJ)\eng16.o -Errors $(ERR)\eng16.err

$(OBJ)\fs_flash.o:  $(CSRCM)\fs_flash.c $(CSRCM)\fs_flash.h
	copy $(CSRCM)\fs_flash.c fs_flash.c
	$(ARMCC) $(CFLAGS7) fs_flash.c -o $(OBJ)\fs_flash.o -Errors $(ERR)\fs_flash.err
	del fs_flash.c

$(OBJ)\fs.o:  $(CSRCM)\fs.c $(CSRCM)\fs_plus.c $(CSRCM)\fs.h
	copy $(CSRCM)\fs.c fs.c
	$(ARMCC) $(CFLAGS7) fs.c -o $(OBJ)\fs.o -Errors $(ERR)\fs.err
	del fs.c

$(OBJ)\sdi.o:  $(CSRCC)\sdi.c $(CSRCC)\sdi.h
	copy  $(CSRCC)\sdi.c sdi.c
	$(ARMCC) $(CFLAGS2) sdi.c -o $(OBJ)\sdi.o -Errors $(ERR)\sdi.err
	del sdi.c

$(OBJ)\diskio.o:  $(CSRCC)\diskio.c $(CSRCC)\diskio.h
	copy  $(CSRCC)\diskio.c diskio.c
	$(ARMCC) $(CFLAGS0) diskio.c -o $(OBJ)\diskio.o -Errors $(ERR)\diskio.err
	del diskio.c

$(OBJ)\ff.o:  $(CSRCC)\ff.c $(CSRCC)\ff.h $(CSRCC)\ffconf.h
	copy  $(CSRCC)\ff.c ff.c
	$(ARMCC) $(CFLAGS0) ff.c -o $(OBJ)\ff.o -Errors $(ERR)\ff.err
	del ff.c

$(OBJ)\rtc.o:  $(CSRCC)\rtc.c $(CSRCC)\rtc.h
	copy  $(CSRCC)\rtc.c rtc.c
	$(ARMCC) $(CFLAGS2) rtc.c -o $(OBJ)\rtc.o -Errors $(ERR)\rtc.err
	del rtc.c

$(OBJ)\rtccpu.o:  $(CSRCC)\rtccpu.c $(CSRCC)\rtc.h
	copy  $(CSRCC)\rtccpu.c rtccpu.c
	$(ARMCC) $(CFLAGS0) rtccpu.c -o $(OBJ)\rtccpu.o -Errors $(ERR)\rtccpu.err
	del rtccpu.c

$(OBJ)\iodev.o:  iodev.c iodev.h
	$(ARMCC) $(CFLAGS2) iodev.c -o $(OBJ)\iodev.o -Errors $(ERR)\iodev.err

$(OBJ)\wiegand.o:  wiegand_6000.c wiegand_lib.c wiegand.h
	$(ARMCC) $(CFLAGS2) wiegand_6000.c -o $(OBJ)\wiegand.o -Errors $(ERR)\wiegand.err

$(OBJ)\key.o:  key.c key.h
	$(ARMCC) $(CFLAGS2) key.c -o $(OBJ)\key.o -Errors $(ERR)\key.err

$(OBJ)\lcdc.o:  $(CSRCC)\lcdc.c $(CSRCC)\lcdc_com.c $(CSRCC)\lcdc.h
	copy  $(CSRCC)\lcdc.c lcdc.c
	$(ARMCC) $(CFLAGS2) lcdc.c -o $(OBJ)\lcdc.o -Errors $(ERR)\lcdc.err
	del lcdc.c

$(OBJ)\sio.o:  sio.c sio_crc.c slavenet_sio.c slavenet_sin.c sio.h slavenet.h
	$(ARMCC) $(CFLAGS2) sio.c -o $(OBJ)\sio.o -Errors $(ERR)\sio.err

$(OBJ)\sin.o:  $(CSRCC)\sin_w5300_New.c $(CSRCC)\sinnet.c $(CSRCC)\sin_w5300.h $(CSRCC)\sin.h $(CSRCC)\sinnet.h
	copy  $(CSRCC)\sin_w5300_New.c sin_w5300.c
	copy  $(CSRCC)\sinnet.c sinnet.c
	$(ARMCC) $(CFLAGS2) sin_w5300.c -o $(OBJ)\sin.o -Errors $(ERR)\sin.err
	del sin_w5300.c
	del sinnet.c

$(OBJ)\ping.o:  $(CSRCC)\ping.c $(CSRCC)\ping.h
	copy  $(CSRCC)\ping.c ping.c
	$(ARMCC) $(CFLAGS0) ping.c -o $(OBJ)\ping.o -Errors $(ERR)\ping.err
	del ping.c

$(OBJ)\dhcp.o:  $(CSRCC)\dhcp_new.c $(CSRCC)\dhcp_new.h
	copy  $(CSRCC)\dhcp_new.c dhcp.c
	$(ARMCC) $(CFLAGS0) dhcp.c -o $(OBJ)\dhcp.o -Errors $(ERR)\dhcp.err
	del dhcp.c

$(OBJ)\cr.o:  cr.c crcfg.c cr.h
	$(ARMCC) $(CFLAGS2) cr.c -o $(OBJ)\cr.o -Errors $(ERR)\cr.err

$(OBJ)\fpapi_new.o:  fpapi_new.c $(CSRCC)\fpapi_new.h
	$(ARMCC) $(CFLAGS2) fpapi_new.c -o $(OBJ)\fpapi_new.o -Errors $(ERR)\fpapi_new.err

$(OBJ)\usbd.o:  usbd.c usbd.h
	$(ARMCC) $(CFLAGS0) usbd.c -o $(OBJ)\usbd.o -Errors $(ERR)\usbd.err

$(OBJ)\pno.o:  pnocfg.c pno.h
	$(ARMCC) $(CFLAGS2) pno.c -o $(OBJ)\pno.o -Errors $(ERR)\pno.err
