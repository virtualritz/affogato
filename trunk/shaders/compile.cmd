@echo off
rem 3Delight
for %%i in (src\*.sl) do shaderdl -Iinclude %%i
rem AIR
for %%i in (src\*.sl) do shaded -Iinclude %%i
rem Aqsis
for %%i in (src\*.sl) do aqsl -Iinclude %%i
rem Pixie
for %%i in (src\*.sl) do sdrc -Iinclude %%i
rem PRMan
for %%i in (src\*.sl) do shader -Iinclude %%i
rem RenderDotC
for %%i in (src\*.sl) do shaderdc -Iinclude %%i
del *.i
