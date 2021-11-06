del pbr_VS.spv
del pbr_FS.spv
%VULKAN_SDK%\Bin\glslc.exe pbr.vert -o pbr_VS.spv
%VULKAN_SDK%\Bin\glslc.exe pbr.frag -o pbr_FS.spv
pause