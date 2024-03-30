.PHONY: compile upload

compile:
	arduino-cli compile --fqbn arduino:avr:mega src

upload:
	arduino-cli upload -p /dev/ttyACM0 --fqbn arduino:avr:mega src 
