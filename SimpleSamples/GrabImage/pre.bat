@echo off

adb connect 192.168.51.203:5555
ping -n 3 127.0.0.1 > nul
adb -s 192.168.51.203:5555 shell am start -n com.roborock.sensordemo/.MainActivity
ping -n 2 127.0.0.1 > nul
adb -s 192.168.51.203:5555 shell input keyevent 3
ping -n 1 127.0.0.1 > nul
adb -s 192.168.51.203:5555 shell setprop showpic 0
pause