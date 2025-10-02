ROOT = /usr/local/gcc-linaro-4
LIB = $(ROOT)/lib/gcc/arm-linux-gnueabihf/4.9.4
LIB1 =$(ROOT)/arm-linux-gnueabihf/lib
LIBMY = ./
USRLOC = /usr/local
INCTGT = /root/a20-olimex/linux-sunxi/include
INCMY = ./
ADMCOM = /home/secu/app/admcom
UIKIT = /home/secu/app/uikit_uv
LIBUV = /home/secu/app/libuv
MYLIB = /home/secu/app/mylib
AVLIB = /home/secu/app/avlib_a20
SCLIB = /home/secu/app/sclib
ALSA = /usr/alsa
PJPROJ = /home/secu/pjproject-2.10
JPEG =  /home/secu/jpeg-9d

CC = arm-linux-gnueabihf-gcc
INC = -I$(INCMY) -I$(ADMCOM) -I$(INCTGT) -I$(UIKIT) -I$(MYLIB) -I$(AVLIB) -I$(SCLIB) -I$(LIBUV)/include -I$(ALSA)/include -I$(USRLOC)/include -I$(PJPROJ)/out/include
CFLAGS = -D_A20 -D_GNU_SOURCE -D_HYNIX -O2 -DPJ_AUTOCONF=1
LDFLAGS = -L$(LIBMY) -L$(LIB) -L$(LIB1) -L$(ALSA)/lib -L$(USRLOC)/lib -L$(USRLOC)/lib/full-package-name -L$(PJPROJ)/out/lib  -L$(JPEG)/.libs
STRIP = arm-linux-gnueabihf-strip

TARGET = t60s
SRCS = main.c mainapp.c maininit.c mainiowm.c mainblem.c maininput.c \
	slvm.c blem.c fpm.c siocred.c \
	beacon.c syscfg.c sysdep.c schecfg.c tacfg.c  \
	mainact.c authreq.c authcap.c authres.c authdone.c \
	acureq.c topmsg.c statbar.c actsleep.c actbgscreen.c phonecall.c pjscall.c \
	authwg.c authmst.c acu.c acuobj.c intrusion.c \
	admtitle.c admprim.c topprim.c aaprim.c \
	adm.c adm_usr.c adm_usrrd.c adm_usrwr.c adm_usrcred.c usrprim.c \
	adm_acct.c adm_acctrd.c adm_acctwr.c \
	adm_hw.c adm_net.c adm_netping.c \
	adm_vf.c adm_set.c adm_setble.c adm_sys.c adm_systest.c \
	msgsvr.c svr.c slvnet.c svrnet.c svrnet_udpcmd.c tftpc.c \
	user_sg.c fsprim.c fsuser.c evt.c sche.c sclib.c acct.c \
	appdev.c cam_a20.c ble.c psu.c \
	cf.c cfprim.c pfprim.c sccfg.c \
	dev.c unit.c cr.c crcred.c subdev.c \
	bio.c ad.c hwprim.c aa.c cf2.c \
	msg.c ta.c func.c netudp.c pjsualib.c pjsua_app.c

SRCS += $(ADMCOM)/cfcfg.c $(ADMCOM)/pinkpd.c \
	$(ADMCOM)/adm_hwcred.c $(ADMCOM)/adm_ac.c \

SRCS += $(LIBUV)/async.c $(LIBUV)/core.c $(LIBUV)/fs.c $(LIBUV)/fs-poll.c \
	$(LIBUV)/getaddrinfo.c $(LIBUV)/getnameinfo.c $(LIBUV)/inet.c \
	$(LIBUV)/linux-core.c $(LIBUV)/linux-inotify.c $(LIBUV)/linux-syscalls.c \
	$(LIBUV)/loop.c $(LIBUV)/loop-watcher.c $(LIBUV)/pipe.c $(LIBUV)/poll.c \
	$(LIBUV)/process.c $(LIBUV)/procfs-exepath.c $(LIBUV)/proctitle.c $(LIBUV)/pthread-fixes.c \
	$(LIBUV)/signal.c $(LIBUV)/stream.c $(LIBUV)/sysinfo-loadavg.c $(LIBUV)/sysinfo-memory.c \
	$(LIBUV)/tcp.c $(LIBUV)/thread.c $(LIBUV)/threadpool.c $(LIBUV)/timer.c $(LIBUV)/tty.c $(LIBUV)/udp.c \
	$(LIBUV)/uv-common.c $(LIBUV)/uv-data-getter-setters.c $(LIBUV)/version.c \

SRCS += $(UIKIT)/lang.c $(UIKIT)/actprim.c $(UIKIT)/psmem.c $(UIKIT)/viewprim.c $(UIKIT)/wallpaper.c \
	$(UIKIT)/pref.c $(UIKIT)/prefrd.c $(UIKIT)/dlgalert.c $(UIKIT)/dlgwait.c $(UIKIT)/dlgfldalert.c \
	$(UIKIT)/dlginput.c $(UIKIT)/dlgdigit.c $(UIKIT)/dlgpassword.c $(UIKIT)/dlguuid.c \
	$(UIKIT)/dlgspinner.c $(UIKIT)/dlgcheck.c $(UIKIT)/dlgslider.c $(UIKIT)/dlgpicker.c \
	$(UIKIT)/dlgdatepicker.c $(UIKIT)/dlgtimepicker.c \
	$(UIKIT)/dlgprogress.c \
	$(UIKIT)/cgtext.c $(UIKIT)/cgrect.c $(UIKIT)/bmpc.c $(UIKIT)/lcdc_a20.c \
	$(UIKIT)/v4l_a20.c $(UIKIT)/jpegdev_a20.c $(UIKIT)/jpeg_api.c $(UIKIT)/jpegexif.c $(UIKIT)/ctfont.c \
	$(UIKIT)/prefspec.c $(UIKIT)/nsobject.c $(UIKIT)/uikit.c $(UIKIT)/uiimage.c $(UIKIT)/uiview.c \
	$(UIKIT)/uiwindow.c $(UIKIT)/uilabel.c $(UIKIT)/uiimageview.c $(UIKIT)/uiscrollview.c \
	$(UIKIT)/uitableviewcell.c $(UIKIT)/uitableview.c $(UIKIT)/uipickerview.c \
	$(UIKIT)/uidatepicker.c $(UIKIT)/uitimepicker.c \
	$(UIKIT)/uicontrol.c $(UIKIT)/uitextfield.c $(UIKIT)/uibutton.c $(UIKIT)/uiswitch.c \
	$(UIKIT)/uislider.c $(UIKIT)/uistepper.c \
	$(UIKIT)/uitimer.c $(UIKIT)/gactivity.c $(UIKIT)/gapp.c $(UIKIT)/gesture.c \

SRCS += $(MYLIB)/log.c $(MYLIB)/rtc.c $(MYLIB)/prim.c $(MYLIB)/cbuf.c \
	$(MYLIB)/aes.c $(MYLIB)/des.c $(MYLIB)/blowfish.c \
	$(MYLIB)/epolllib.c $(MYLIB)/inputlib.c $(MYLIB)/spimlib_esp.c $(MYLIB)/i2cmlib.c \
	$(MYLIB)/exec.c $(MYLIB)/sysprim.c $(MYLIB)/audio_async.c $(MYLIB)/file.c \
	$(MYLIB)/wglib.c \
	$(MYLIB)/wpa.c $(MYLIB)/wpaprim.c $(MYLIB)/wpactrl.c $(MYLIB)/wpanet.c \
	$(MYLIB)/nilib.c $(MYLIB)/sin.c $(MYLIB)/sll.c $(MYLIB)/dhcp.c \
	$(MYLIB)/arpping.c $(MYLIB)/ping.c $(MYLIB)/mycrc.c \

SRCS += $(AVLIB)/av.c $(AVLIB)/avsink.c $(AVLIB)/avsrc.c $(AVLIB)/avrtpsend.c \
	$(AVLIB)/avplayer.c $(AVLIB)/avrecorder.c $(AVLIB)/avstream.c \
	$(AVLIB)/avam.c $(AVLIB)/avao.c $(AVLIB)/avai.c $(AVLIB)/avvo.c $(AVLIB)/avvi.c \
	$(AVLIB)/avcodec.c $(AVLIB)/avcodecs.c \
	$(AVLIB)/avc_dec.c $(AVLIB)/avc_enc.c $(AVLIB)/aac_dec.c $(AVLIB)/aac_enc.c \
	$(AVLIB)/wav.c $(AVLIB)/mov.c $(AVLIB)/movenc.c \
	$(AVLIB)/isom.c $(AVLIB)/mpeg4audio.c $(AVLIB)/avcprim.c $(AVLIB)/aviobuf.c $(AVLIB)/avio.c \
	$(AVLIB)/avutil.c $(AVLIB)/avmem.c $(AVLIB)/avstring.c \
	$(AVLIB)/rational.c $(AVLIB)/mathematics.c $(AVLIB)/intfloat_readwrite.c \
	$(AVLIB)/bitstream.c $(AVLIB)/crc.c $(AVLIB)/error.c \

SRCS += $(SCLIB)/as3911_io.c $(SCLIB)/as3911.c $(SCLIB)/as3911_interrupt.c \
	$(SCLIB)/as3911_modulation_adjustment.c $(SCLIB)/as3911_stream.c $(SCLIB)/cardcrypto.c \
	$(SCLIB)/sc_iso14443a.c $(SCLIB)/sc_iso14443b.c $(SCLIB)/sc_iso14443_4.c \
	$(SCLIB)/sc_iso15693_2.c $(SCLIB)/sc_iso15693_3.c \
	$(SCLIB)/sc_mifare.c $(SCLIB)/sc_mifare_crc.c $(SCLIB)/sc_mifare_crypto1.c \
	$(SCLIB)/sc_mifare_crypto1_clean.c $(SCLIB)/sc_mifare_parity.c $(SCLIB)/sc_mifare_uint64emu.c \
	$(SCLIB)/sctask.c $(SCLIB)/sctask_com.c $(SCLIB)/sctask_iso14443a.c $(SCLIB)/sctask_iso14443b.c \
	$(SCLIB)/sctask_iso15693.c $(SCLIB)/scthread.c \

OBJS  = $(addsuffix .o, $(basename $(SRCS)))

LIBS = -lm -lz -lpthread -lrt -lasound -lfaad -lcedar_base -lcedar_common -lcedar_vdecoder -lcedar_vencoder -ljpeg

$(LIBUV)/%: INC := -I$(INCTGT) -I$(LIBUV)/include

PJLIBS = -lpjsua2-arm-unknown-linux-gnueabihf -lstdc++ -lpjsua-arm-unknown-linux-gnueabihf \
-lpjsip-ua-arm-unknown-linux-gnueabihf -lpjsip-simple-arm-unknown-linux-gnueabihf \
-lpjsip-arm-unknown-linux-gnueabihf -lpjmedia-codec-arm-unknown-linux-gnueabihf \
-lpjmedia-arm-unknown-linux-gnueabihf -lpjmedia-videodev-arm-unknown-linux-gnueabihf \
-lpjmedia-audiodev-arm-unknown-linux-gnueabihf -lpjmedia-arm-unknown-linux-gnueabihf \
-lpjnath-arm-unknown-linux-gnueabihf -lpjlib-util-arm-unknown-linux-gnueabihf \
-lsrtp-arm-unknown-linux-gnueabihf -lresample-arm-unknown-linux-gnueabihf \
-lgsmcodec-arm-unknown-linux-gnueabihf -lspeex-arm-unknown-linux-gnueabihf \
-lilbccodec-arm-unknown-linux-gnueabihf -lg7221codec-arm-unknown-linux-gnueabihf \
-lyuv-arm-unknown-linux-gnueabihf -lwebrtc-arm-unknown-linux-gnueabihf  -lpj-arm-unknown-linux-gnueabihf

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) -o $(TARGET) $(LDFLAGS) $(OBJS) $(LIBS) $(PJLIBS)
	$(STRIP) $(TARGET) 
	cp $(TARGET) /home/secu/tftpboot

%.o: %.c
	$(CC) $(INC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) 
	rm -f $(TARGET)	
	rm -f *.gdb

main.o: main.c
maininit.o: maininit.c maininit.h
mainiowm.o: mainiowm.c maininput.h slvmlib.h
mainblem.o: mainblem.c maininput.h
maininput.o: maininput.c maininput.h
slvm.o: slvm.c slvm.h slvmlib.h
blem.o: blem.c blem.h
fpm.o: fpm.c fpm.h
siocred.o: siocred.c siocred.h
beacon.o: beacon.c beacon.h
syscfg.o: syscfg.c syscfg.h
sysdep.o: sysdep.c sysdep.h
schecfg.o: schecfg.c schecfg.h
tacfg.o: tacfg.c tacfg.h
mainact.o: mainact.c appact.h
authreq.o: authreq.c appact.h
authcap.o: authcap.c appact.h
authres.o: authres.c appact.h
authdone.o: authdone.c appact.h
acureq.o: acureq.c appact.h
topmsg.o: topmsg.c appact.h
statbar.o: statbar.c appact.h
actsleep.o: actsleep.c appact.h
actbgscreen.o: actbgscreen.c appact.h
phonecall.o: phonecall.c appact.h
pjscall.o: pjscall.c appact.h
authwg.o: authwg.c authwg.h
authmst.o: authmst.c appact.h
acu.o: acu.c acu.h acu_com.c acu_wr.c acu_xfer.c acu_req.c
acuobj.o: acuobj.c acuobj.h
intrusion.o: intrusion.c intrusion.h
admtitle.o: admtitle.c admtitle.h
admprim.o: admprim.c admprim.h
topprim.o: topprim.c topprim.h
aaprim.o: aaprim.c aaprim.h
adm.o: adm.c appact.h
adm_usr.o: adm_usr.c appact.h
adm_usrrd.o: adm_usrrd.c appact.h
adm_usrwr.o: adm_usrwr.c appact.h
adm_usrcred.o: adm_usrcred.c appact.h
usrprim.o: usrprim.c usrprim.h
adm_hw.o: adm_hw.c adm_hwdev.c adm_hwnwp.c adm_hwunit.c adm_hwsubdev.c adm_hwch.c adm_hwbi.c adm_hwbo.c adm_hwad.c adm_hwcr.c appact.h
adm_net.o: adm_net.c appact.h
adm_netping.o: adm_netping.c appact.h
adm_vf.o: adm_vf.c appact.h
adm_set.o: adm_set.c appact.h
adm_setble.o: adm_setble.c appact.h
adm_sys.o: adm_sys.c appact.h
adm_systest.o: adm_systest.c appact.h
msgsvr.o: msgsvr.c msgsvr.h
svr.o: svr.c svr.h
slvnet.o: slvnet.c slvnet.h slvnet_udp.c slvnet_sio.c
svrnet.o: svrnet.c svrnet.h svrnet_udp.c
svrnet_udpcmd.o: svrnet_udpcmd.c svrnet.h
tftpc.o: tftpc.c tftpc.h
user_sg.o: user_sg.c user.h
fsprim.o: fsprim.c fsprim.h
fsuser.o: fsuser.c fsprim.c fsuser.h fsuser_ex.c fsevt.c fscamevt.c fsacct.c
evt.o: evt.c evt.h
sche.o: sche.c sche.h
sclib.o: sclib.c sclib.h
acct.o: acct.c acct.h
appdev.o: appdev.c appdev.h
cam_a20.o: cam_a20.c cam.h
ble.o: ble.c ble.h
psu.o: psu.c psu.h
cf.o: cf.c cf.h
cfprim.o: cfprim.c cfprim.h
pfprim.o: pfprim.c pfprim.h
sccfg.o: sccfg.c sccfg.h
dev.o: dev.c dev.h
unit.o: unit.c unit.h
cr.o: cr.c cr.h
crcred.o: crcred.c cr.h
subdev.o: subdev.c subdev.h
bio.o: bio.c bio.h
ad.o: ad.c ad.h
hwprim.o: hwprim.c hwprim.h
aa.o: aa.c aa.h
cf2.o: cf2.c $(ADMCOM)/cf.h
wglib.o: wglib.c wglib.h
msg.o: msg.c msgeng.c msgkor.c msg.h
ta.o: ta.c tacfg.h
func.o: func.c func.h
netudp.o: netudp.c netudp.h
pjsualib.o: pjsualib.c pjsualib.h
pjsua_app.o: pjsua_app.c pjsua_app.h
$(ADMCOM)/cfcfg.o: $(ADMCOM)/cfcfg.c $(ADMCOM)/cfcfg.h
$(ADMCOM)/pinkpd.o: $(ADMCOM)/pinkpd.c $(ADMCOM)/pinkpd.h
$(ADMCOM)/adm_hwcred.o: $(ADMCOM)/adm_hwcred.c
$(ADMCOM)/adm_ac.o: $(ADMCOM)/adm_ac.c $(ADMCOM)/adm_accf.c $(ADMCOM)/adm_acwf.c $(ADMCOM)/adm_acpf.c $(ADMCOM)/adm_acwpf.c $(ADMCOM)/adm_acaa.c
$(ADMCOM)/adm_vf.o: $(ADMCOM)/adm_vf.c appact.h
$(LIBUV)/async.o: $(LIBUV)/async.c
$(LIBUV)/core.o: $(LIBUV)/core.c
$(LIBUV)/dl.o: $(LIBUV)/dl.c
$(LIBUV)/fs.o: $(LIBUV)/fs.c
$(LIBUV)/fs-poll.o: $(LIBUV)/fs-poll.c
$(LIBUV)/getaddrinfo.o: $(LIBUV)/getaddrinfo.c
$(LIBUV)/getnameinfo.o: $(LIBUV)/getnameinfo.c
$(LIBUV)/inet.o: $(LIBUV)/inet.c
$(LIBUV)/linux-core.o: $(LIBUV)/linux-core.c
$(LIBUV)/linux-inotify.o: $(LIBUV)/linux-inotify.c
$(LIBUV)/linux-syscalls.o: $(LIBUV)/linux-syscalls.c
$(LIBUV)/loop.o: $(LIBUV)/loop.c
$(LIBUV)/loop-watcher.o: $(LIBUV)/loop-watcher.c
$(LIBUV)/pipe.o: $(LIBUV)/pipe.c
$(LIBUV)/poll.o: $(LIBUV)/poll.c
$(LIBUV)/process.o: $(LIBUV)/process.c
$(LIBUV)/procfs-exepath.o: $(LIBUV)/procfs-exepath.c
$(LIBUV)/proctitle.o: $(LIBUV)/proctitle.c
$(LIBUV)/pthread-fixes.o: $(LIBUV)/pthread-fixes.c
$(LIBUV)/signal.o: $(LIBUV)/signal.c
$(LIBUV)/stream.o: $(LIBUV)/stream.c
$(LIBUV)/sysinfo-loadavg.o: $(LIBUV)/sysinfo-loadavg.c
$(LIBUV)/sysinfo-memory.o: $(LIBUV)/sysinfo-memory.c
$(LIBUV)/tcp.o: $(LIBUV)/tcp.c
$(LIBUV)/thread.o: $(LIBUV)/thread.c
$(LIBUV)/threadpool.o: $(LIBUV)/threadpool.c
$(LIBUV)/timer.o: $(LIBUV)/timer.c
$(LIBUV)/tty.o: $(LIBUV)/tty.c
$(LIBUV)/udp.o: $(LIBUV)/udp.c
$(LIBUV)/uv-common.o: $(LIBUV)/uv-common.c
$(LIBUV)/uv-data-getter-setters.o: $(LIBUV)/uv-data-getter-setters.c
$(LIBUV)/version.o: $(LIBUV)/version.c
$(UIKIT)/lang.o: $(UIKIT)/lang.c $(UIKIT)/lang.h
$(UIKIT)/actprim.o: $(UIKIT)/actprim.c $(UIKIT)/actprim.h
$(UIKIT)/psmem.o: $(UIKIT)/psmem.c $(UIKIT)/psmem.h
$(UIKIT)/viewprim.o: $(UIKIT)/viewprim.c $(UIKIT)/view_kpd.c $(UIKIT)/view_kpdimg.c $(UIKIT)/view_clock.c $(UIKIT)/viewprim.h
$(UIKIT)/wallpaper.o: $(UIKIT)/wallpaper.c $(UIKIT)/wallpaper.h
$(UIKIT)/pref.o: $(UIKIT)/pref.c $(UIKIT)/pref.h
$(UIKIT)/prefrd.o: $(UIKIT)/prefrd.c $(UIKIT)/pref.h
$(UIKIT)/dlgalert.o: $(UIKIT)/dlgalert.c $(UIKIT)/dlg.h
$(UIKIT)/dlgwait.o: $(UIKIT)/dlgwait.c $(UIKIT)/dlg.h
$(UIKIT)/dlgfldalert.o: $(UIKIT)/dlgfldalert.c $(UIKIT)/dlg.h
$(UIKIT)/dlginput.o: $(UIKIT)/dlginput.c $(UIKIT)/dlg.h
$(UIKIT)/dlgdigit.o: $(UIKIT)/dlgdigit.c $(UIKIT)/dlg.h
$(UIKIT)/dlgpassword.o: $(UIKIT)/dlgpassword.c $(UIKIT)/dlg.h
$(UIKIT)/dlguuid.o: $(UIKIT)/dlguuid.c $(UIKIT)/dlg.h
$(UIKIT)/dlgspinner.o: $(UIKIT)/dlgspinner.c $(UIKIT)/dlg.h
$(UIKIT)/dlgcheck.o: $(UIKIT)/dlgcheck.c $(UIKIT)/dlg.h
$(UIKIT)/dlgslider.o: $(UIKIT)/dlgslider.c $(UIKIT)/dlg.h
$(UIKIT)/dlgpicker.o: $(UIKIT)/dlgpicker.c $(UIKIT)/dlg.h
$(UIKIT)/dlgdatepicker.o: $(UIKIT)/dlgdatepicker.c $(UIKIT)/dlg.h
$(UIKIT)/dlgtimepicker.o: $(UIKIT)/dlgtimepicker.c $(UIKIT)/dlg.h
$(UIKIT)/dlgprogress.o: $(UIKIT)/dlgprogress.c $(UIKIT)/dlg.h
$(UIKIT)/cgtext.o: $(UIKIT)/cgtext.c $(UIKIT)/cgtext.h
$(UIKIT)/cgrect.o: $(UIKIT)/cgrect.c $(UIKIT)/cgrect.h
$(UIKIT)/bmpc.o: $(UIKIT)/bmpc.c $(UIKIT)/bmpc.h
$(UIKIT)/lcdc_a20.o: $(UIKIT)/lcdc_a20.c $(UIKIT)/lcdc_com.c $(UIKIT)/lcdc.h
$(UIKIT)/v4l_a20.o: $(UIKIT)/v4l_a20.c $(UIKIT)/v4l.h
$(UIKIT)/jpegdev.o: $(UIKIT)/jpegdev_a20.c $(UIKIT)/jpegdev.h
$(UIKIT)/jpeg_api.o: $(UIKIT)/jpeg_api.c $(UIKIT)/jpegdev.h
$(UIKIT)/jpegexif.o: $(UIKIT)/jpegexif.c $(UIKIT)/jpegdev.h
$(UIKIT)/ctfont.o: $(UIKIT)/ctfont.c $(UIKIT)/ctfont.h
$(UIKIT)/prefspec.o: $(UIKIT)/prefspec.c $(UIKIT)/prefspec.h
$(UIKIT)/nsobject.o: $(UIKIT)/nsobject.c $(UIKIT)/nsobject_.h
$(UIKIT)/uikit.o: $(UIKIT)/uikit.c $(UIKIT)/UIKit.h
$(UIKIT)/uiimage.o: $(UIKIT)/uiimage.c $(UIKIT)/uiimage_.h
$(UIKIT)/uiview.o: $(UIKIT)/uiview.c $(UIKIT)/uiview_.h
$(UIKIT)/uiwindow.o: $(UIKIT)/uiwindow.c $(UIKIT)/uiwindow_.h
$(UIKIT)/uilabel.o: $(UIKIT)/uilabel.c $(UIKIT)/uilabel_.h
$(UIKIT)/uiimageview.o: $(UIKIT)/uiimageview.c $(UIKIT)/uiimageview_.h
$(UIKIT)/uiscrollview.o: $(UIKIT)/uiscrollview.c $(UIKIT)/uiscrollview_.h
$(UIKIT)/uitableviewcell.o: $(UIKIT)/uitableviewcell.c $(UIKIT)/uitableviewcell_.h
$(UIKIT)/uitableview.o: $(UIKIT)/uitableview.c $(UIKIT)/uitableview_.h
$(UIKIT)/uipickerview.o: $(UIKIT)/uipickerview.c $(UIKIT)/uipickerview_.h
$(UIKIT)/uidatepicker.o: $(UIKIT)/uidatepicker.c $(UIKIT)/uidatepicker_.h
$(UIKIT)/uitimepicker.o: $(UIKIT)/uitimepicker.c $(UIKIT)/uitimepicker_.h
$(UIKIT)/uicontrol.o: $(UIKIT)/uicontrol.c $(UIKIT)/uicontrol_.h
$(UIKIT)/uitextfield.o: $(UIKIT)/uitextfield.c $(UIKIT)/uitextfield_.h
$(UIKIT)/uibutton.o: $(UIKIT)/uibutton.c $(UIKIT)/uibutton_.h
$(UIKIT)/uiswitch.o: $(UIKIT)/uiswitch.c $(UIKIT)/uiswitch_.h
$(UIKIT)/uislider.o: $(UIKIT)/uislider.c $(UIKIT)/uislider_.h
$(UIKIT)/uistepper.o: $(UIKIT)/uistepper.c $(UIKIT)/uistepper_.h
$(UIKIT)/uitimer.o: $(UIKIT)/uitimer.c $(UIKIT)/UITimer.h
$(UIKIT)/gactivity.o: $(UIKIT)/gactivity.c $(UIKIT)/gactivity.h
$(UIKIT)/gapp.o: $(UIKIT)/gapp.c $(UIKIT)/gapp.h
$(UIKIT)/gesture.o: $(UIKIT)/gesture.c $(UIKIT)/gesture.h
$(MYLIB)/log.o: $(MYLIB)/log.c $(MYLIB)/log.h
$(MYLIB)/rtc.o: $(MYLIB)/rtc.c $(MYLIB)/rtc.h
$(MYLIB)/prim.o: $(MYLIB)/prim.c $(MYLIB)/prim.h
$(MYLIB)/cbuf.o: $(MYLIB)/cbuf.c $(MYLIB)/cbuf.h
$(MYLIB)/aes.o: $(MYLIB)/aes.c $(MYLIB)/aes.h
$(MYLIB)/des.o: $(MYLIB)/des.c $(MYLIB)/des.h
$(MYLIB)/blowfish.o: $(MYLIB)/blowfish.c $(MYLIB)/blowfish.h
$(MYLIB)/epolllib.o: $(MYLIB)/epolllib.c $(MYLIB)/epolllib.h
$(MYLIB)/inputlib.o: $(MYLIB)/inputlib.c $(MYLIB)/inputlib.h
$(MYLIB)/spimlib_esp.o: $(MYLIB)/spimlib_esp.c $(MYLIB)/spimlib.h
$(MYLIB)/i2cmlib.o: $(MYLIB)/i2cmlib.c $(MYLIB)/i2cmlib.h
$(MYLIB)/exec.o: $(MYLIB)/exec.c $(MYLIB)/exec.h
$(MYLIB)/sysprim.o: $(MYLIB)/sysprim.c $(MYLIB)/sysprim.h
$(MYLIB)/audio_async.o: $(MYLIB)/audio_async.c $(MYLIB)/audio.h
$(MYLIB)/file.o: $(MYLIB)/file.c $(MYLIB)/file.h
$(MYLIB)/wglib.o: $(MYLIB)/wglib.c $(MYLIB)/wglib.h
$(MYLIB)/wpa.o: $(MYLIB)/wpa.c $(MYLIB)/wpa.h
$(MYLIB)/wpaprim.o: $(MYLIB)/wpaprim.c $(MYLIB)/wpaprim.h
$(MYLIB)/wpactrl.o: $(MYLIB)/wpactrl.c $(MYLIB)/wpactrl.h
$(MYLIB)/wpanet.o: $(MYLIB)/wpanet.c $(MYLIB)/wpanet.h
$(MYLIB)/nilib.o: $(MYLIB)/nilib.c $(MYLIB)/nilib.h
$(MYLIB)/sin.o: $(MYLIB)/sin.c $(MYLIB)/sin.h
$(MYLIB)/sll.o: $(MYLIB)/sll.c $(MYLIB)/sll.h
$(MYLIB)/dhcp.o: $(MYLIB)/dhcp.c $(MYLIB)/dhcp.h
$(MYLIB)/arpping.o:  $(MYLIB)/arpping.c $(MYLIB)/arpping.h
$(MYLIB)/ping.o: $(MYLIB)/ping.c $(MYLIB)/ping.h
$(MYLIB)/mycrc.o: $(MYLIB)/mycrc.c $(MYLIB)/mycrc.h
$(AVLIB)/av.o: $(AVLIB)/av.c $(AVLIB)/av.h
$(AVLIB)/avsink.o: $(AVLIB)/avsink.c $(AVLIB)/avsink.h
$(AVLIB)/avsrc.o: $(AVLIB)/avsrc.c $(AVLIB)/avsrc.h
$(AVLIB)/avrtpsend.o: $(AVLIB)/avrtpsend.c $(AVLIB)/avrtpsend.h
$(AVLIB)/avplayer.o: $(AVLIB)/avplayer.c $(AVLIB)/avplayer.h
$(AVLIB)/avrecorder.o: $(AVLIB)/avrecorder.c $(AVLIB)/avrecorder.h
$(AVLIB)/avstream.o: $(AVLIB)/avstream.c $(AVLIB)/avstream.h
$(AVLIB)/avam.o: $(AVLIB)/avam.c $(AVLIB)/avdevice.h
$(AVLIB)/avao.o: $(AVLIB)/avao.c $(AVLIB)/avdevice.h
$(AVLIB)/avai.o: $(AVLIB)/avai.c $(AVLIB)/avdevice.h
$(AVLIB)/avvo.o: $(AVLIB)/avvo.c $(AVLIB)/avdevice.h
$(AVLIB)/avvi.o: $(AVLIB)/avvi.c $(AVLIB)/avdevice.h
$(AVLIB)/avcodec.o: $(AVLIB)/avcodec.c $(AVLIB)/avcodec.h
$(AVLIB)/avcodecs.o: $(AVLIB)/avcodecs.c $(AVLIB)/avcodecs.h
$(AVLIB)/avc_dec.o: $(AVLIB)/avc_dec.c $(AVLIB)/avc.h
$(AVLIB)/avc_enc.o: $(AVLIB)/avc_enc.c $(AVLIB)/avc.h
$(AVLIB)/aac_dec.o: $(AVLIB)/aac_dec.c $(AVLIB)/aac.h
$(AVLIB)/aac_enc.o: $(AVLIB)/aac_enc.c $(AVLIB)/aac.h
$(AVLIB)/wav.o: $(AVLIB)/wav.c $(AVLIB)/wav.h
$(AVLIB)/mov.o: $(AVLIB)/mov.c $(AVLIB)/mov.h
$(AVLIB)/movenc.o: $(AVLIB)/movenc.c $(AVLIB)/movenc.h
$(AVLIB)/isom.o: $(AVLIB)/isom.c $(AVLIB)/isom.h
$(AVLIB)/mpeg4audio.o: $(AVLIB)/mpeg4audio.c $(AVLIB)/mpeg4audio.h
$(AVLIB)/avcprim.o: $(AVLIB)/avcprim.c $(AVLIB)/avcprim.h
$(AVLIB)/aviobuf.o: $(AVLIB)/aviobuf.c $(AVLIB)/avio.h
$(AVLIB)/avio.o: $(AVLIB)/avio.c $(AVLIB)/avio.h
$(AVLIB)/avutil.o: $(AVLIB)/avutil.c $(AVLIB)/avutil.h
$(AVLIB)/mem.o: $(AVLIB)/avmem.c $(AVLIB)/mvem.h
$(AVLIB)/avstring.o: $(AVLIB)/avstring.c $(AVLIB)/avstring.h
$(AVLIB)/rational.o: $(AVLIB)/rational.c $(AVLIB)/rational.h
$(AVLIB)/mathematics.o: $(AVLIB)/mathematics.c $(AVLIB)/mathematics.h
$(AVLIB)/intfloat_readwrite.o: $(AVLIB)/intfloat_readwrite.c $(AVLIB)/intfloat_readwrite.h
$(AVLIB)/bitstream.o: $(AVLIB)/bitstream.c $(AVLIB)/put_bits.h
$(AVLIB)/crc.o: $(AVLIB)/crc.c $(AVLIB)/crc.h
$(AVLIB)/error.o: $(AVLIB)/error.c $(AVLIB)/error.h
$(SCLIB)/as3911_io.o: $(SCLIB)/as3911_io.c $(SCLIB)/as3911_io.h
$(SCLIB)/as3911.o: $(SCLIB)/as3911.c $(SCLIB)/as3911.h
$(SCLIB)/as3911_interrupt.o: $(SCLIB)/as3911_interrupt.c
$(SCLIB)/as3911_modulation_adjustment.o: $(SCLIB)/as3911_modulation_adjustment.c
$(SCLIB)/as3911_stream.o: $(SCLIB)/as3911_stream.c
$(SCLIB)/cardcrypto.o: $(SCLIB)/cardcrypto.c
$(SCLIB)/sc_iso14443a.o: $(SCLIB)/sc_iso14443a.c
$(SCLIB)/sc_iso14443b.o: $(SCLIB)/sc_iso14443b.c
$(SCLIB)/sc_iso14443_4.o: $(SCLIB)/sc_iso14443_4.c
$(SCLIB)/sc_iso15693_2.o: $(SCLIB)/sc_iso15693_2.c
$(SCLIB)/sc_iso15693_3.o: $(SCLIB)/sc_iso15693_3.c
$(SCLIB)/sc_mifare.o: $(SCLIB)/sc_mifare.c
$(SCLIB)/sc_mifare_crc.o: $(SCLIB)/sc_mifare_crc.c
$(SCLIB)/sc_mifare_crypto1.o: $(SCLIB)/sc_mifare_crypto1.c
$(SCLIB)/sc_mifare_crypto1_clean.o: $(SCLIB)/sc_mifare_crypto1_clean.c
$(SCLIB)/sc_mifare_parity.o: $(SCLIB)/sc_mifare_parity.c
$(SCLIB)/sc_mifare_uint64emu.o: $(SCLIB)/sc_mifare_uint64emu.c
$(SCLIB)/sctask.o: $(SCLIB)/sctask.c
$(SCLIB)/sctask_com.o: $(SCLIB)/sctask_com.c
$(SCLIB)/sctask_iso14443a.o: $(SCLIB)/sctask_iso14443a.c
$(SCLIB)/sctask_iso14443b.o: $(SCLIB)/sctask_iso14443b.c
$(SCLIB)/sctask_iso15693.o: $(SCLIB)/sctask_iso15693.c
$(SCLIB)/scthread.o: $(SCLIB)/scthread.c $(SCLIB)/scthread.h
