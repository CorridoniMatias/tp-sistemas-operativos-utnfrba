all: 
	-cd kemmens/Debug && $(MAKE) clean && $(MAKE) all
	-cd MDJ/Debug && $(MAKE) clean && $(MAKE) all
	-cd CPU/Debug && $(MAKE) clean && $(MAKE) all
	-cd SAFA/Debug && $(MAKE) clean && $(MAKE) all
	-cd FM9/Debug && $(MAKE) clean && $(MAKE) all
	-cd DAM/Debug && $(MAKE) clean && $(MAKE) all
scratch:
	- rm -rf so-commons-library && git clone https://github.com/sisoputnfrba/so-commons-library.git && cd so-commons-library && sudo $(MAKE) uninstall && sudo $(MAKE) install
	- $(MAKE) all
	- rm -rf so-commons-library

dev:
	-echo "TPENV = dev" > env.txt

prod:
	-echo "TPENV = prod" > env.txt

fifa_examples:
	- rm -rf fifa-examples && git clone https://github.com/sisoputnfrba/fifa-examples.git
fs_checkpoint:
	- rm -rf MDJ/Debug/fifa-checkpoint/ && cp -R fifa-examples/fifa-checkpoint/ MDJ/Debug/
fs_entrega:
	- rm -rf MDJ/Debug/fifa-entrega/ && cp -R fifa-examples/fifa-entrega/ MDJ/Debug/

ips:
	- cd configs && cat ips.txt | tee -a prod/minima/DAM.config prod/minima/CPU.config prod/filesystem/DAM.config prod/filesystem/CPU.config prod/completa/DAM.config prod/completa/CPU.config prod/checkpoint/DAM.config prod/checkpoint/CPU.config prod/algoritmos/DAM.config prod/algoritmos/CPU.config

include ./env.txt

TESTNAME=checkpoint

copy_config:
	- cp --remove-destination configs/$(TPENV)/$(TESTNAME)/SAFA.config SAFA/Debug/Config/SAFA.config
	- cp --remove-destination configs/$(TPENV)/$(TESTNAME)/mdj.config MDJ/Debug/mdj.config
	- cp --remove-destination configs/$(TPENV)/$(TESTNAME)/FM9.config FM9/Debug/FM9.config
	- cp --remove-destination configs/$(TPENV)/$(TESTNAME)/DAM.config DAM/Debug/DAM.config
	- cp --remove-destination configs/$(TPENV)/$(TESTNAME)/CPU.config CPU/Debug/CPU.config

test_completa: TESTNAME=completa
test_completa: copy_config

test_minima: TESTNAME=minima
test_minima: copy_config

test_algoritmos: TESTNAME=algoritmos
test_algoritmos: copy_config

test_filesystem: TESTNAME=filesystem
test_filesystem: copy_config

test_checkpoint: TESTNAME=checkpoint
test_checkpoint: copy_config