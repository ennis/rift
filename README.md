rift
====

Environnement de travail
------------------------

* `src/`: les sources
* `include/`: les headers
* `src/renderer/`: les sources du renderer
* `deps/`: les dépendances (GLM, GLEW et GLFW)
* `resources/`: les ressources utilisées par l'application (images, shaders, modèles, etc.)
* `cmake/`: modules CMake

Compilation
-----------
    mkdir build/
    cd build/
    cmake ../
    make 

Exécution
---------
    cd build/
    make run