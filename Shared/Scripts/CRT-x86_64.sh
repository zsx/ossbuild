#!/bin/sh

#Initializes which MSVCRT to use (VS6, VS2003, VS2005, VS2008, etc.)

crt_startup() {
	select_crt
}

crt_shutdown() {
	deselect_crt
}

select_crt() {
	#Do nothing...
	nothing=0
}

deselect_crt() {
	#Do nothing...
	nothing=0
}