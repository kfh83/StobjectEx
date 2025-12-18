# StobjectEx
Windows XP Shell tray objects for modern windows

# How to setup
1. Compile the project from source
2. Rename the resulting binary to something like stobjectex.dll then copy it over to C:\Windows\System32
3. Take sndvol32.exe from Windows XP Professional x64 Edition and place it also in System32 (for sound tray icon)
4. Import the ``Stobject.reg`` in this repo normally. This just sets up certain stuff for the sound icon to appear properly and sets SysTray and SysTrayInvoker
5. Open regedit as TrustedInstaller using your preferred program to do such (I use [Winaero Tweaker](https://winaerotweaker.com/) or [ExecTI](https://winaero.com/download-execti-run-as-trustedinstaller/), there's also [RunTI](https://winaero.com/download-execti-run-as-trustedinstaller/)) and go to ``HKEY_CLASSES_ROOT\CLSID\{35CEC8A3-2BE6-11D2-8773-92E220524153}\InProcServer32``
6. Set the ``(Default)`` key to ``%SystemRoot%\system32\stobjectex.dll`` or whatever you named the .dll to
7. Restart ``explorer.exe`` through Task Manager
8. profit
