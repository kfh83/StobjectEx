# StobjectEx
Windows XP Shell tray objects for modern windows
<img width="800" height="600" alt="Windows 10 explorer" src="https://github.com/user-attachments/assets/774ccc68-a861-47a5-b0cb-78514c070896" />
 
Intended for usage with [ExplorerEx](https://github.com/kfh83/ExplorerEx)

## How to setup
1. Grab an artifact from GitHub Actions (or compile it yoursef for whatever reason)
2. Rename the resulting binary to something like stobjectex.dll then copy it over to ``%SystemRoot%\System32``
3. Take sndvol32.exe from Windows XP Professional x64 Edition and place it also in System32 (for sound tray icon)
4. Import the ``Stobject.reg`` in this repo normally. This just sets up certain stuff for the sound icon to appear properly and sets SysTray and SysTrayInvoker
5. Open regedit as TrustedInstaller using your preferred program to do so (I use [Winaero Tweaker](https://winaerotweaker.com/) or [ExecTI](https://winaero.com/download-execti-run-as-trustedinstaller/), there's also [RunTI](https://winaero.com/download-execti-run-as-trustedinstaller/)) and go to ``HKEY_CLASSES_ROOT\CLSID\{35CEC8A3-2BE6-11D2-8773-92E220524153}\InProcServer32``
6. Set the ``(Default)`` key to ``%SystemRoot%\system32\stobjectex.dll`` or whatever you named the .dll to
7. Restart ``explorer.exe`` through Task Manager
8. profit

## SndVol32 fix
The standard sndvol32.exe may not work as intended without this patch. Credits go to [comdlg32](https://github.com/comdlg32)
1. Download the ``sndvol32.sdb`` and ``sndvol32-64.sdb`` files in this repo
2. Place the files in ``%SystemRoot%\apppatch``
3. Open Command Prompt and run the following:
```
sdbinst C:\Windows\apppatch\sndvol32.sdb -q
sdbinst C:\Windows\apppatch\sndvol32-64.sdb -q
```
You may need to reimport the ``Stobject.reg`` again for this to truly work.
