.PHONY: All clean
APP=forwarder
DRIVER=$(APP).drivers
PACK=$(APP).pack

all: $(APP) $(DRIVER) $(PACK)

$(APP): 
	@echo "Cloning AMD-XLNX OpenNIC Shell..."
	@mkdir $@ 
	git clone https://github.com/Xilinx/open-nic-shell.git $@
	cp script/compile.bash $(APP)/.
	cd $(APP) && ./compile.bash | tee compile.log

$(DRIVER): $(APP)
	cp -r $(APP)/build/au280/open_nic_shell/open_nic_shell.gen/sources_1/ip/vitis_net_p4_0/src/sw/drivers $@
	cd $@ && make && cp ../src/c-driver/* install/. &&\
	cd install && make
pack: $(APP) $(DRIVER)
	@echo "Packing all files for transfer"
	mkdir $(PACK)
	cp $(APP)/$(APP).mcs $(PACK)/.
	cp -r $(DRIVER)/install $(PACK)/.
	cp -r pcap $(PACK)/.
	mkdir -p $(PACK)/util
	cp ../../utility/program_config_mem.tcl $(PACK)/util/.
	tar -cvzf $(APP).tar.gz $(PACK)
clean:
	rm -rf $(APP) $(DRIVER) $(PACK)
	
	
