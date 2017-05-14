for /r %%v in (*.vert) do C:/VulkanSDK/1.0.37.0/Bin/glslangValidator.exe -V %%v
for /r %%v in (*.frag) do C:/VulkanSDK/1.0.37.0/Bin/glslangValidator.exe -V %%v
pause