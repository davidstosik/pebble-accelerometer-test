# pebble-accelerometer-test
Quick test of the accelerometer functions of a Pebble watch

App displays:

 - timestamp of last acceleration data sample
 - period over which sample was collected
 - number of samples kept / total samples for current display (a sample is kept only if watch did not vibrate)
 - acceleration on 3 axis X,Y, Z.

Buttons:

 - Up makes the watch vibrate
 - Select does nothing interesting for the moment
 - Short press on Down increments the number of samples per updates until 25, then starts again from 0
 - Long press on Down selects next sample rate option (10Hz, 25Hz, 50Hz, 100Hz)
