
MAKE = make

.PHONY: all
all:
	$(MAKE) -j -C musl-1.1.18 all
	$(MAKE) -j -C mbedtls-2.16.1 all
	$(MAKE) -j -C zlib-1.2.11 all
	$(MAKE) -j -C azure_iothub all
	$(MAKE) -j -C app_iothub_client/Debug all

.PHONY: clean
clean:
	$(MAKE) -j -C musl-1.1.18 clean
	$(MAKE) -j -C mbedtls-2.16.1 clean
	$(MAKE) -j -C zlib-1.2.11 clean
	$(MAKE) -j -C azure_iothub clean
	$(MAKE) -j -C app_iothub_client/Debug clean

.PHONY: realclean
realclean:
	$(MAKE) -j -C musl-1.1.18 clean
	$(MAKE) -j -C mbedtls-2.16.1 clean
	$(MAKE) -j -C zlib-1.2.11 clean
	$(MAKE) -j -C azure_iothub clean
	$(MAKE) -j -C app_iothub_client/Debug realclean
