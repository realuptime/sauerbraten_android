LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := SDLGL
TOP := $(LOCAL_PATH)/../../..

SDL_PATH := $(TOP)/3rdparty/SDL/SDL
SDL_IMAGE_PATH := $(TOP)/3rdparty/SDL/SDL2_image
#SDL_TTF_PATH := $(TOP)/3rdparty/SDL/SDL2_ttf
SDL_MIXER_PATH := $(TOP)/3rdparty/SDL/SDL2_mixer

APP_NAME := cube2

LOCAL_CFLAGS := -DANDROID -DHAVE_GLES
LOCAL_CFLAGS += -DAPP_NAME=$(APP_NAME)
LOCAL_CFLAGS += -D_BACKGAMMON_GAME
LOCAL_CPPFLAGS := $(LOCAL_CFLAGS) -std=c++11 -frtti -fexceptions -Wfatal-errors -fsigned-char

#LOCAL_CPPFLAGS += -O3 -g0
#LOCAL_CPPFLAGS += -O0 -g3

LOCAL_LDLIBS := -landroid -llog -lEGL -lGLESv2
LOCAL_LDLIBS += -lz

LOCAL_C_INCLUDES := $(SDL_PATH)/include $(SDL_IMAGE_PATH) $(SDL_TTF_PATH) $(SDL_MIXER_PATH) $(TOP)/3rdparty/glm $(TOP)
LOCAL_C_INCLUDES += $(TOP)/common
LOCAL_C_INCLUDES += $(TOP)/src/enet/include
LOCAL_C_INCLUDES += $(TOP)/src/engine
LOCAL_C_INCLUDES += $(TOP)/src/fpsgame
#LOCAL_C_INCLUDES += $(TOP)/src/rpggame
LOCAL_C_INCLUDES += $(TOP)/src/shared


FILE_LIST := \
	$(SDL_PATH)/src/main/android/SDL_android_main.c \
	$(wildcard $(TOP)/src/enet/*.c) \
	$(wildcard $(TOP)/src/engine/*.cpp) \
	$(wildcard $(TOP)/src/shared/*.cpp) \
	$(wildcard $(TOP)/src/fpsgame/*.cpp)

	
LOCAL_SRC_FILES := $(FILE_LIST:$(LOCAL_PATH)/%=%)

LOCAL_SHARED_LIBRARIES := SDL2
LOCAL_SHARED_LIBRARIES += SDL2_image
#LOCAL_SHARED_LIBRARIES += SDL2_ttf
LOCAL_SHARED_LIBRARIES += SDL2_mixer
LOCAL_SHARED_LIBRARIES += mikmod
LOCAL_SHARED_LIBRARIES += smpeg2
include $(BUILD_SHARED_LIBRARY)

LOCAL_PATH := $(call my-dir)

$(call import-module,SDL)
$(call import-module,SDL2_image)
#$(call import-module,SDL2_ttf)
$(call import-module,SDL2_mixer)
$(call import-module,SDL2_mixer/external/libmikmod-3.1.12)
$(call import-module,SDL2_mixer/external/smpeg2-2.0.0)
