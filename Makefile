XOVI_REPO ?= $(HOME)/github/asivery/xovi
name = ebook-fit-to-height

VERSION ?= $(shell git describe --tags --always --dirty 2>/dev/null || echo "dev")

XOVI_VERSION = $(shell echo "$(VERSION)" | sed 's/^v//' | awk -F'[.-]' '{if (NF >= 3 && $$1 ~ /^[0-9]+$$/) printf "%d", ($$1 * 65536) + ($$2 * 256) + $$3; else print "0"}')

CC_AARCH64 ?= aarch64-linux-gnu-gcc
CFLAGS = -D_GNU_SOURCE -fPIC -O2 -Wall -Wextra -DVERSION=\"$(VERSION)\"
LDFLAGS =

BUILD_AARCH64 = build-aarch64

OBJECTS_AARCH64 = $(BUILD_AARCH64)/ebook-fit-to-height.o $(BUILD_AARCH64)/xovi.o

.PHONY: all clean clean-aarch64 print-version

print-version:
	@echo "VERSION=$(VERSION)"
	@echo "XOVI_VERSION=$(XOVI_VERSION)"

all: $(name).so

$(name).so: $(OBJECTS_AARCH64)
	$(CC_AARCH64) $(CFLAGS) -shared -o $@ $^ $(LDFLAGS)

$(BUILD_AARCH64)/%.o: src/%.c $(BUILD_AARCH64)/xovi.h
	$(CC_AARCH64) $(CFLAGS) -I$(BUILD_AARCH64) -c $< -o $@

$(BUILD_AARCH64)/xovi.o: $(BUILD_AARCH64)/xovi.c $(BUILD_AARCH64)/xovi.h
	$(CC_AARCH64) $(CFLAGS) -I$(BUILD_AARCH64) -c $< -o $@

$(BUILD_AARCH64)/xovi.h: $(BUILD_AARCH64)/xovi.c
$(BUILD_AARCH64)/xovi.c: $(name).xovi
	@mkdir -p $(BUILD_AARCH64)
	python3 $(XOVI_REPO)/util/xovigen.py -a aarch64 -o $(BUILD_AARCH64)/xovi.c -H $(BUILD_AARCH64)/xovi.h $(name).xovi
	@sed -i.bak 's/const int EXTENSIONVERSION = [0-9]*/const int EXTENSIONVERSION = $(XOVI_VERSION)/' $(BUILD_AARCH64)/xovi.c && rm -f $(BUILD_AARCH64)/xovi.c.bak

clean-aarch64:
	rm -f $(name).so $(BUILD_AARCH64)/*.o

clean: clean-aarch64
	rm -rf $(BUILD_AARCH64)
