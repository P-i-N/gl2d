@echo off

if exist premaker.bat (
  call premaker.bat
) else (
  if exist ..\Premaker\premaker.bat (
    call ..\Premaker\premaker.bat
  ) else (
    echo "Premaker not found!"
  )
)
