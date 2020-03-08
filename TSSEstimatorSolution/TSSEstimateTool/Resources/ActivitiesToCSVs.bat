for %%i in (.\Activities\*.fit) do call java -jar FitCSVTool.jar -b Activities/%%~ni.fit CSVs/%%~ni.csv
pause