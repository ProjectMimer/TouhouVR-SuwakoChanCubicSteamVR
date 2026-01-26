@echo off

IF NOT EXIST ".\\suwapyon2oculus144e\\" (
    IF NOT EXIST ".\\suwapyon2oculus144e.zip" (
        echo ****
        echo * Downloading suwapyon2oculus144e.zip
        echo ****
        curl -L --ssl-no-revoke "https://web.archive.org/web/20160208155406/http://www.utgsoftware.net/archives/dlcount.php?fname=suwapyon2oculus144e.zip&dir=suwapyon2oculus/" --output ".\\suwapyon2oculus144e.zip"
    )

    IF EXIST ".\\suwapyon2oculus144e.zip" (
        echo ****
        echo * Extracting suwapyon2oculus144e.zip
        echo ****
        tar -xf ".\\suwapyon2oculus144e.zip"
    )
)

IF NOT EXIST "%APPDATA%\\UTGSoftware\\Suwapyon2HD\\SteamVR" (
    echo ****
    echo * Creating "%APPDATA%\\UTGSoftware\\Suwapyon2HD"
    echo ****
    mkdir "%APPDATA%\\UTGSoftware\\Suwapyon2HD"
    mkdir "%APPDATA%\\UTGSoftware\\Suwapyon2HD\\Save"
    mkdir "%APPDATA%\\UTGSoftware\\Suwapyon2HD\\SteamVR"

    IF EXIST ".\\vr\\settings\\initialize_oculus.dat" (
        copy ".\\vr\\settings\\initialize_oculus.dat" "%APPDATA%\\UTGSoftware\\Suwapyon2HD\\initialize_oculus.dat"
		copy ".\\vr\\settings\\oculusConfig.sav" "%APPDATA%\\UTGSoftware\\Suwapyon2HD\\Save\\oculusConfig.sav"
    )
)

IF EXIST ".\\suwapyon2oculus144e\\" (
    IF EXIST ".\\vr\\actions.json" (
        IF NOT EXIST ".\\suwapyon2oculus144e\\actions.json" (
            copy ".\\vr\\actions.json" ".\\suwapyon2oculus144e\\actions.json"
        )
    )

    IF EXIST ".\\vr\\actions_touch.json" (
        IF NOT EXIST ".\\suwapyon2oculus144e\\actions_touch.json" (
            copy ".\\vr\\actions_touch.json" ".\\suwapyon2oculus144e\\actions_touch.json"
        )
    )

    IF EXIST ".\\vr\\dinput8.dll" (
        IF NOT EXIST ".\\suwapyon2oculus144e\\dinput8.dll" (
            copy ".\\vr\\dinput8.dll" ".\\suwapyon2oculus144e\\dinput8.dll"
        )
    )

    IF EXIST ".\\vr\\openvr_api.dll" (
        IF NOT EXIST ".\\suwapyon2oculus144e\\openvr_api.dll" (
            copy ".\\vr\\openvr_api.dll" ".\\suwapyon2oculus144e\\openvr_api.dll"
        )
    )

    cd ".\\suwapyon2oculus144e\\"
    ".\\suwapyon2Oculus.exe"
    cd ".."
)

@echo on
