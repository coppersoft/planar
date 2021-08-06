Per fargli trovare gli include, fare quick fix sull'errore degli include, lui crea un file

.vscode/c_cpp_properties.json nella root del progetto

Dentro "includePath" aggiungere il punto in cui si trova la cartella include_h nell'NDK dell'Amiga, per esempio

"includePath": [
                "${workspaceFolder}/**",
                "/Users/mbplorenzo/amiga_sdk/NDK_3.9/Include/include_h"
            ],