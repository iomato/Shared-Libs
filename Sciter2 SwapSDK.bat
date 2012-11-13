@echo off
if not exist "sciter2-sdk" goto fail
if not exist "sciter2-sdkPREV" if not exist "sciter2-sdkLAST" goto fail

if exist "sciter2-sdkPREV" (
	move "sciter2-sdk" "sciter2-sdkLAST"
	move "sciter2-sdkPREV" "sciter2-sdk"
) else (
	move "sciter2-sdk" "sciter2-sdkPREV"
	move "sciter2-sdkLAST" "sciter2-sdk"
)


:success
exit

:fail
echo Failed
pause>nul