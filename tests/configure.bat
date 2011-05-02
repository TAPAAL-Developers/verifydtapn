@echo off
echo Checking which platform to use...
where $path:rm.exe > nul
if ERRORLEVEL 1 (
echo RM = del> config.mk
echo rm not found, using Windows rules
) ELSE (
echo RM = rm -rf > config.mk
echo rm found, using Linux rules
)
