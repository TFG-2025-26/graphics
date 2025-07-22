# OpenGL projects skeleton

This skeleton uses the following libraries:

* [glfw](https://www.glfw.org/) for managing OpenGL windows
* [glm](https://glm.g-truc.net/) for the CPU mathematical operations that follow the specifications of the GPU mathematical operations in OpenGL
* [glew](https://glew.sourceforge.net/) for loading OpenGL extensions
* [stb_image](https://github.com/nothings/stb) for loading and saving images

When creating a new project, remember to add the property sheet `IGProjects.props` to it.
Menu Ver -> Otras ventanas -> Administrador de propiedades -> Agregar hoja de propiedades existente -> IGProjects.props -> Guardar

In order to add a new library, copy its directory into the solution root and update the elements `IncludePath`, `LibraryPath`, `AdditionalDependencies`, and `LocalDebuggerEnvironment` in `IGProjects.props`.

You can also create new property sheets.
