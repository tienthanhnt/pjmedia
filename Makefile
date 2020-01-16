include custom.mk

.PHONY: all clean
#MEP_APP:=sip_streamer
#MEP_APP:=sip_receiver
MEP_APP:=list-devices
#MEP_APP:=file2sound
#MEP_APP:=conf_to_2_sounds
#MEP_APP:=sound_conf_file
#MEP_APP:=simple_ics
#MEP_APP:=pthread_endpoints
#MEP_APP:=same_endpt
#MEP_APP:=internal_stream
#MEP_APP:=splitcomb_file2sound
#MEP_APP:=sc_1file_to_2sound
#MEP_APP:=sc_pjsua_1file_to_2sound
#MEP_APP:=func_sc_pjsua_1file_to_2sound
#MEP_APP:=file_pjsua_sound
#MEP_APP:=just_create_sound
#MEP_APP:=sc_pjsua_phone_to_2sound
#MEP_APP:=sc_1file_to_left_right
#MEP_APP:=pjsua_phone

SRC_DIR:=.
APP_SRCS:=$(MEP_APP).c
SRCS:=endpoint.c

AU_PROCESS_DIR:=audio_processing
AU_PROCESS_SRCES:=processing-port.c

#### AUTO GAIN CONTROL ########################
AGC_DIR:=$(AU_PROCESS_DIR)/agc
AGC_SRCS:=$(shell cd $(AGC_DIR)/; ls -1 *.c)

#### VOICE ACTIVE DETECTION ############
VAD_DIR:=$(AU_PROCESS_DIR)/vad
VAD_SRCS:=$(shell cd $(VAD_DIR)/; ls -1 *.c)

#### NOISE SUPPRESSION #######################
NS_DIR:=$(AU_PROCESS_DIR)/ns
NS_SRCS:=$(shell cd $(NS_DIR)/; ls -1 *.c)

#### SIGNAL PROCESSING #############
SPL_DIR:=$(AU_PROCESS_DIR)/signal_processing
SPL_SRCS:=$(shell cd $(SPL_DIR)/;ls -1 *.c)

CFLAGS:=-DPJ_AUTOCONF=1 -O2 -DPJ_IS_BIG_ENDIAN=0 -DPJ_IS_LITTLE_ENDIAN=1 -fms-extensions -Wno-deprecated-declarations
CFLAGS+=-I$(SRC_DIR)/include
CFLAGS+=-I$(LIBS_DIR)/include
CFLAGS+=-I$(C_DIR)/include
CFLAGS+=-I$(AU_PROCESS_DIR)/include
CFLAGS+=-I$(AU_PROCESS_DIR)/signal_processing/include
CFLAGS+=-I$(AU_PROCESS_DIR)/vad/include
CFLAGS+=-I$(AU_PROCESS_DIR)/ns/include
CFLAGS+=-I$(AU_PROCESS_DIR)/agc/include
CFLAGS+=-I$(AU_PROCESS_DIR)//include
CFLAGS+=-I include -I $(SPL_DIR) -I $(SPL_DIR)/include -I $(VAD_DIR) -I $(VAD_DIR)/include -I $(NS_DIR) -I $(NS_DIR)/include $(shell PKG_CONFIG_PATH=$(PKG_CONFIG_PATH) pkg-config --cflags libpjproject)
LDFLAGS:=$(shell PKG_CONFIG_PATH=$(PKG_CONFIG_PATH) pkg-config --libs libpjproject)

CFLAGS+=-D__ICS_INTEL__
CFLAGS+=-g

LIBS+=-lcrypto

all: $(MEP_APP)

$(MEP_APP):$(APP_SRCS:.c=.o) $(AU_PROCESS_SRCES:.c=.o) $(AGC_SRCS:.c=.o) $(VAD_SRCS:.c=.o) $(NS_SRCS:.c=.o) $(SPL_SRCS:.c=.o)
	$(CROSS_TOOL) -o $@ $^ $(LIBS)
$(APP_SRCS:.c=.o): %.o: $(SRC_DIR)/test_src/%.c
	$(CROSS_TOOL) -c -o $@ $< $(CFLAGS)
$(AU_PROCESS_SRCES:.c=.o): %.o: $(AU_PROCESS_DIR)/src/%.c
	$(CROSS_TOOL) -c -o $@ $< $(CFLAGS)
$(AGC_SRCS:.c=.o): %.o: $(AGC_DIR)/%.c
	$(CROSS_TOOL) -c -o $@ $< $(CFLAGS)
$(VAD_SRCS:.c=.o): %.o: $(VAD_DIR)/%.c
	$(CROSS_TOOL) -c -o $@ $< $(CFLAGS)
$(NS_SRCS:.c=.o): %.o: $(NS_DIR)/%.c
	$(CROSS_TOOL) -c -o $@ $< $(CFLAGS)
$(SPL_SRCS:.c=.o): %.o: $(SPL_DIR)/%.c
	$(CROSS_TOOL) -c -o $@ $< $(CFLAGS)

deploy:
	sudo install -m 755 list-devices /opt/bin
clean:
	rm -fr *.o $(MEP_APP)
	rm -rf gen*
