.PHONY: clean

CC=gcc
LIBS=-lm
OBJ=calc_main.c lib/libvitisnetp4drv.so
TARGET=driver

TARGET_DBG=driver_debug
DBG_FLAG=-D_DEBUG

$(TARGET): $(OBJ)
	$(CC) $^ $(LIBS) -o $@ 

$(TARGET_DBG): $(OBJ)
	echo "DEBUG Enabled"
	$(CC) $^ $(LIBS) $(DBG_FLAG) -o $@ 

clean:
	rm -f $(TARGET) $(TARGET_DBG) 
