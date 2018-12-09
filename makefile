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
fifa_examples:
	- rm -rf fifa-examples && git clone https://github.com/sisoputnfrba/fifa-examples.git
fs_checkpoint:
	- rm -rf MDJ/Debug/fifa-checkpoint/ && cp -R fifa-examples/fifa-checkpoint/ MDJ/Debug/
fs_final:
	- rm -rf MDJ/Debug/fifa-entrega/ && cp -R fifa-examples/fifa-entrega/ MDJ/Debug/
test_checkpoint:
	- cp --remove-destination configs/checkpoint/SAFA.config SAFA/Debug/Config/SAFA.config
	- cp --remove-destination configs/checkpoint/mdj.config MDJ/Debug/mdj.config
	- cp --remove-destination configs/checkpoint/FM9.config FM9/Debug/FM9.config
	- cp --remove-destination configs/checkpoint/DAM.config DAM/Debug/DAM.config
	- cp --remove-destination configs/checkpoint/CPU.config CPU/Debug/CPU.config
test_minima:
	- cp --remove-destination configs/minima/SAFA.config SAFA/Debug/Config/SAFA.config
	- cp --remove-destination configs/minima/mdj.config MDJ/Debug/mdj.config
	- cp --remove-destination configs/minima/FM9.config FM9/Debug/FM9.config
	- cp --remove-destination configs/minima/DAM.config DAM/Debug/DAM.config
	- cp --remove-destination configs/minima/CPU.config CPU/Debug/CPU.config
test_algoritmos:
	- cp --remove-destination configs/algoritmos/SAFA.config SAFA/Debug/Config/SAFA.config
	- cp --remove-destination configs/algoritmos/mdj.config MDJ/Debug/mdj.config
	- cp --remove-destination configs/algoritmos/FM9.config FM9/Debug/FM9.config
	- cp --remove-destination configs/algoritmos/DAM.config DAM/Debug/DAM.config
	- cp --remove-destination configs/algoritmos/CPU.config CPU/Debug/CPU.config
test_filesystem:
	- cp --remove-destination configs/filesystem/SAFA.config SAFA/Debug/Config/SAFA.config
	- cp --remove-destination configs/filesystem/mdj.config MDJ/Debug/mdj.config
	- cp --remove-destination configs/filesystem/FM9.config FM9/Debug/FM9.config
	- cp --remove-destination configs/filesystem/DAM.config DAM/Debug/DAM.config
	- cp --remove-destination configs/filesystem/CPU.config CPU/Debug/CPU.config
test_completa:
	- cp --remove-destination configs/completa/SAFA.config SAFA/Debug/Config/SAFA.config
	- cp --remove-destination configs/completa/mdj.config MDJ/Debug/mdj.config
	- cp --remove-destination configs/completa/FM9.config FM9/Debug/FM9.config
	- cp --remove-destination configs/completa/DAM.config DAM/Debug/DAM.config
	- cp --remove-destination configs/completa/CPU.config CPU/Debug/CPU.config