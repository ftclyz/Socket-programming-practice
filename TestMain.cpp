#include "pch.h"
#include "IOmode.h"

int main() {
	IOmode iomode(12345, "192.168.1.6");
	iomode.select_mode();
}