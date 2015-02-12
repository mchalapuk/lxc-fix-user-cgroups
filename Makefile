PREFIX=~

fix: fix.cpp
	$(CXX) -std=c++11 -o fix fix.cpp

install: fix
	cp -a fix $(PREFIX)/bin/lxc-fix-user-cgroups

.PHONY: install
.DEFAULT: fix

