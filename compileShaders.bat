glslc tri_mesh.vert -o tri_mesh.vert.spv
@echo off
for %%i in (*.vert *.frag) do "glslc" "%%~i" -o "%%~i.spv"