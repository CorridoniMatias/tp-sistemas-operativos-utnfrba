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

include ./env.txt

test_checkpoint:
	- cp --remove-destination configs/$(TPENV)/checkpoint/SAFA.config SAFA/Debug/Config/SAFA.config
	- cp --remove-destination configs/$(TPENV)/checkpoint/mdj.config MDJ/Debug/mdj.config
	- cp --remove-destination configs/$(TPENV)/checkpoint/FM9.config FM9/Debug/FM9.config
	- cp --remove-destination configs/$(TPENV)/checkpoint/DAM.config DAM/Debug/DAM.config
	- cp --remove-destination configs/$(TPENV)/checkpoint/CPU.config CPU/Debug/CPU.config
test_minima:
	- cp --remove-destination configs/$(TPENV)/minima/SAFA.config SAFA/Debug/Config/SAFA.config
	- cp --remove-destination configs/$(TPENV)/minima/mdj.config MDJ/Debug/mdj.config
	- cp --remove-destination configs/$(TPENV)/minima/FM9.config FM9/Debug/FM9.config
	- cp --remove-destination configs/$(TPENV)/minima/DAM.config DAM/Debug/DAM.config
	- cp --remove-destination configs/$(TPENV)/minima/CPU.config CPU/Debug/CPU.config
test_algoritmos:
	- cp --remove-destination configs/$(TPENV)/algoritmos/SAFA.config SAFA/Debug/Config/SAFA.config
	- cp --remove-destination configs/$(TPENV)/algoritmos/mdj.config MDJ/Debug/mdj.config
	- cp --remove-destination configs/$(TPENV)/algoritmos/FM9.config FM9/Debug/FM9.config
	- cp --remove-destination configs/$(TPENV)/algoritmos/DAM.config DAM/Debug/DAM.config
	- cp --remove-destination configs/$(TPENV)/algoritmos/CPU.config CPU/Debug/CPU.config
test_filesystem:
	- cp --remove-destination configs/$(TPENV)/filesystem/SAFA.config SAFA/Debug/Config/SAFA.config
	- cp --remove-destination configs/$(TPENV)/filesystem/mdj.config MDJ/Debug/mdj.config
	- cp --remove-destination configs/$(TPENV)/filesystem/FM9.config FM9/Debug/FM9.config
	- cp --remove-destination configs/$(TPENV)/filesystem/DAM.config DAM/Debug/DAM.config
	- cp --remove-destination configs/$(TPENV)/filesystem/CPU.config CPU/Debug/CPU.config
test_completa:
	- cp --remove-destination configs/$(TPENV)/completa/SAFA.config SAFA/Debug/Config/SAFA.config
	- cp --remove-destination configs/$(TPENV)/completa/mdj.config MDJ/Debug/mdj.config
	- cp --remove-destination configs/$(TPENV)/completa/FM9.config FM9/Debug/FM9.config
	- cp --remove-destination configs/$(TPENV)/completa/DAM.config DAM/Debug/DAM.config
	- cp --remove-destination configs/$(TPENV)/completa/CPU.config CPU/Debug/CPU.config