# HELTEC Stuff #

I have written some sketches to record sensor readings by sending them to my home MQTT broker in a JSON format. On my broker is a python subscriber script which picks up the published sensor readings and dumps them into a mariaDb database.

Readings are averaged using a rolling average of the last 30 readings. Although the sensors may be running at 1 sample per second the recording interval is longer - like once every minute. The idea is to be able to compare the behaviour of two or more co-located sensors and decide if there is parity.

The readings, where possible, are raw values. The can be mapped to density charts using a spreadsheet but I wanted to see the raw outputs first in order that the sensor driver algorithms could be worked on, if necessary.


## MQTT_SHARP_ZPH.ino ##

This sketch records the outputs froma Sharp gp2y1010 sensor (voltage) and a ZPH01 sensor (% low pulse time).

## MQTT_DSM_ION.ion ##

This sketch, imminent, will record the outputs of a DSM501 and an Ion Chamber sensor I made myself using a smoke alarm ion chamber. 
TBA
