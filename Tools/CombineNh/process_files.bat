@ECHO OFF
for /f %%a IN ('dir /b /s *_HEIGHT.jpg') do call combine_nh.bat %%a