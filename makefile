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

