.PHONY = all clean
CURDIR_TMP := $(CURDIR)

all:
	@echo Make Stub
	@$(MAKE) --no-print-directory -C $(CURDIR_TMP)/stub \
		-f $(CURDIR_TMP)/stub/Makefile
	@mv -u $(CURDIR_TMP)/stub/stub.bin \
		$(CURDIR_TMP)/make_app/stub.bin
	@echo Make NAND Loader
	@$(MAKE) --no-print-directory -C $(CURDIR_TMP) \
		-f $(CURDIR_TMP)/Makefile.loader
	@mv -u $(CURDIR_TMP)/loader.bin \
		$(CURDIR_TMP)/make_app/loader.bin
clean:
	@$(MAKE) --no-print-directory -C $(CURDIR_TMP)/stub \
		-f $(CURDIR_TMP)/stub/Makefile clean
	@$(MAKE) --no-print-directory -C $(CURDIR_TMP) \
		-f $(CURDIR_TMP)/Makefile.loader clean
	rm -rf $(CURDIR_TMP)/make_app/stub.bin
	rm -rf $(CURDIR_TMP)/make_app/loader.bin
	rm -rf $(CURDIR_TMP)/make_app/00000001.app
