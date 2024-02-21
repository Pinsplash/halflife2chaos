@echo off
setlocal enabledelayedexpansion
setlocal enableextensions

py --version > nul 2>&1
if %errorlevel% neq 0 (
    choice /C YN /M "Python is not installed. Do you want the script to install it"
    IF errorlevel 2 (
        echo Install Python and run this script again. You can download Python here: https://www.python.org
        pause
        exit
    ) else (
        if not exist "pythoninstaller.exe" (
            powershell -Command "Invoke-WebRequest https://www.python.org/ftp/python/3.11.4/python-3.11.4-amd64.exe -OutFile pythoninstaller.exe"
            echo Python installer downloaded. Installing...
        )
        .\pythoninstaller.exe /passive AppendPath=1
        del pythoninstaller.exe
        echo Step 4 done.
    )
) else (
    Echo Step 4 already done.
)

py -c "import platform; print(f'Installed Python version: {platform.python_version()}'); exit(0) if tuple(map(int, platform.python_version().split('.'))) >= (3,10) else exit(1)"
if %errorlevel% neq 0 (
    REM rcon==2.4.2 requires Python 3.10 or higher.
    echo Python version 3.10 or higher is required. Please upgrade or uninstall Python and run this script again.
    pause
    exit
)

py -m pip install -r requirements.txt
if %errorlevel% neq 0 (
    echo An error occurred while installing the requirements.
    pause
    exit
)
echo Steps 5, 6, 7 and 8 done.


set "iniFile=%appdata%\obs-studio\global.ini"
if exist "%iniFile%" (
    findstr /c:"[Python]" "%iniFile%" > nul 2>&1
    if !errorlevel! neq 0 (
        py -c "import os; import sys; executable = '/'.join(sys.executable.split('\\')[:-1]); f = open(os.path.expandvars('${appdata}/obs-studio/global.ini'), 'a'); f.writelines(['\n', '[Python]\n', f'Path64bit={executable}\n']); f.close();"
        echo Step 9 done.
    ) else (
        echo Step 9 already done.
    )   
) else (
    echo Can't find global.ini file. Try starting and closing OBS, then run this script again, if that doesn't help, perform step 9 manually.
)

set cnt=0
for %%A in (%appdata%\obs-studio\basic\scenes\*.json) do set /a cnt+=1

if %cnt% == 0 (
    echo Couldn't find OBS scenes, you'll have to do step 10 manually.
) else if %cnt% == 1 (
    py -c "import glob; import os; import json; print('You must run this script in youtube-integration folder.') or quit(0) if not os.path.exists(os.getcwd() + '\\youtube_integration.py') else None; f = open(glob.glob(os.path.expandvars('${appdata}/obs-studio/basic/scenes/*.json'))[0], 'r'); data = json.loads(f.read()); f.close(); quit(0) if any('youtube_integration.py' in script['path'] for script in data['modules']['scripts-tool']) else None; data['modules']['scripts-tool'].append({'path': os.getcwd().replace('\\', '/') + '/youtube_integration.py', 'settings': {}}); dump = json.dumps(data); f = open(glob.glob(os.path.expandvars('${appdata}/obs-studio/basic/scenes/*.json'))[0], 'w'); f.write(dump); f.close();"
    echo Step 10 done.
) else (
    py -c "import glob; import os; import json; print('You must run this script in youtube-integration folder.') or quit(0) if not os.path.exists(os.getcwd() + '\\youtube_integration.py') else None; user_input = input('In which profile to add twitch integration? Possible answer [' + ', '.join(path.split('\\')[-1].split('.')[0].lower() for path in glob.glob(os.path.expandvars('${appdata}/obs-studio/basic/scenes/*.json'))) + '] - '); print('Incorrect value. If you want to try again, run the script again.') or quit(0) if not os.path.exists(os.path.expandvars('${appdata}/obs-studio/basic/scenes/'+ user_input + '.json')) else None; f = open(os.path.expandvars('${appdata}/obs-studio/basic/scenes/'+ user_input + '.json'), 'r'); data = json.loads(f.read()); f.close(); quit(0) if any('youtube_integration.py' in script['path'] for script in data['modules']['scripts-tool']) else None; data['modules']['scripts-tool'].append({'path': os.getcwd().replace('\\', '/') + '/youtube_integration.py', 'settings': {}}); dump = json.dumps(data); f = open(os.path.expandvars('${appdata}/obs-studio/basic/scenes/'+ user_input + '.json'), 'w'); f.write(dump); f.close();"
    echo Step 10 done.
)

echo Done.
pause