# Graphics-Engine

Vyzaduje c++2017

pouzito visual studio 2019 (vyssi by melo fungovat)

zavislosti:

vulkanSDK https://vulkan.lunarg.com
glfw 3 https://www.glfw.org/download
glm https://github.com/g-truc/glm/releases
stb https://github.com/nothings/stb

vs configuracni soubory resi pridani knihoven pokud je dodrzena struktura slozky Dependencies
jako zde https://github.com/skrivanroman/Graphics-Engine

jedina vyjimka je vulkansdk ktery je v C:\VulkanSDK\1.3.204.0
pokud by se soubery nachazeli jinde je treba ve visual studiu zmenit nastaveni
project > Graphics-Engine properties > c/c++ > general > aditional include directories
project > Graphics-Engine properties > linker > general > aditional library directories

melo to by jit i na linuxu ale netestoval jsem to
