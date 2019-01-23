ZPH01 Library

A stream based object to input the readings from a ZPH01 sensor.

Being stream based you can tell it to use any stream e.g. HardwareSerial,SoftwareSerial

Recently updated with a new non-blocking driver this requires you to call the
driver's loop() method within your loop() to allow it to soak up serial data without
having to wait.

See Examples on how to use it
