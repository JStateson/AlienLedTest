# USBappAlienware
 test alienware lighting
To build this program in VS2022 you will need c++, the SDK for win10/11 (v143) and USB Device connectivity.  All this can be configured from the installer.  You will also need "boost"
 
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
App made a connection using 0x526 and did affect a change to the LED

The purpose of this app is to get the LED lighting to work on a case that I added an MSI x299 raider motherboard
