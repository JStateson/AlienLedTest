# USBappAlienware
 test alienware lighting
 
To build this program in VS2022 you will need C++, the SDK for win10/11 (v143) and USB Device connectivity.  All this can be configured from the installer.  You will also need "boost"
 
Download boost and unzip into

 Directory of C:\Program Files\boost
 
04/16/2023  11:34 PM           254,976 b2.exe

...

...

04/10/2023  08:45 AM             2,486 bootstrap.bat

After unzipping, run bootstrap.bat and then run b2.exe

This app was based on "AlienFX-LED-tester"
https://github.com/bchretien/AlienFX-LED-tester
and the linux version "alienfx.tar.gz" which seems to be the same code or close to the above
<<<<<<< HEAD
This app is capable of resetting the PCB board should that be necessary.

msi-raider-bios is original reg entry for [HKEY_LOCAL_MACHINE\HARDWARE\DESCRIPTION\System\BIOS]

r4_bios is the reg file temporarily to replace the above.  Needed only to install AWCC 4.8.23

AlenwareAlienFX is reg to change unknown to Area51R4

usbview show connected USB devices and DevManView is device manager command line app

Enable-R4-PCB is script to enable the Led lighting PCB but read the following for how to use it

r4-case-lights  word document describing how to get lights to work on R4 Area51 case

Other resources googleing dell alienfx api

https://github.com/Gurjot95/AlienFX-SDK

https://github.com/topics/alienfx-api

https://github.com/antonpup

https://github.com/T-Troll/alienfx-tools
