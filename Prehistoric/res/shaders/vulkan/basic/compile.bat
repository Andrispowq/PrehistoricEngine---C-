del basic_VS.spv
del basic_FS.spv
%VULKAN_SDK%\Bin\glslc.exe basic.vert -o basic_VS.spv
%VULKAN_SDK%\Bin\glslc.exe basic.frag -o basic_FS.spv
pause