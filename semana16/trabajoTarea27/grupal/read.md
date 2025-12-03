# Ejemplo 1: Listar archivos disponibles en el servidor
        ===== EJEMPLO 1: LIST =====
        ./bin/client.out --list

# Ejemplo 2: Agregar un archivo local (killerWhale.txt) al servidor
        ===== EJEMPLO 2: ADD =====
        ./bin/client.out --add=killerWhale.txt
Para agregar una figura tienes que crear un .txt en la carpeta local que contenga el dibujo y subirlo con el mismo nombre del archivo
# Ejemplo 3: Obtener un archivo específico (killerWhale.txt) del servidor
        ===== EJEMPLO 3: GET =====
        ./bin/client.out --get=killerWhale.txt
# Ejemplo 4: Eliminar un archivo específico (killerWhale.txt) del servidor
        ===== EJEMPLO 4: RM =====
        ./bin/client.out --rm=killerWhale.txt
# Aclaraciones
        La solicitud se manda con el protocolo grupal definido hasta el momento
        el cliente lo que hace es armarte la solicitud y enviarlo
# compilar y ejecutar
solo haz make y luego de que compile los ejecutas desde proyecto:

        ./bin/cacheServer.out
        ./bin/forkServer.out
        ./bin/figureServer.out
y asi

