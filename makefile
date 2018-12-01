all: 
	-cd kemmens/Debug && $(MAKE) clean && $(MAKE) all
	-cd MDJ/Debug && $(MAKE) clean && $(MAKE) all
	-cd CPU/Debug && $(MAKE) clean && $(MAKE) all
	-cd SAFA/Debug && $(MAKE) clean && $(MAKE) all
	-cd FM9/Debug && $(MAKE) clean && $(MAKE) all
	-cd DAM/Debug && $(MAKE) clean && $(MAKE) all