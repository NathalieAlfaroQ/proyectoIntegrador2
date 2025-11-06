# Ejemplo 1: Listar archivos disponibles en el servidor
        ===== EJEMPLO 1: LIST =====
        ./client.out --list

# Ejemplo 2: Agregar un archivo local (killerWhale.txt) al servidor
        ===== EJEMPLO 2: ADD =====
        ./client.out --add=oso.txt
Para agregar una figura tienes que crear un .txt en la carpeta local que contenga el dibujo y subirlo con el mismo nombre del archivo
# Ejemplo 3: Obtener un archivo específico (killerWhale.txt) del servidor
        ===== EJEMPLO 3: GET =====
        ./client.out --get=halloween.txt
# Ejemplo 4: Eliminar un archivo específico (killerWhale.txt) del servidor
        ===== EJEMPLO 4: RM =====
        ./client.out --rm=killerWhale.txt 
# Aclaraciones
        La solicitud se manda con el protocolo grupal definido hasta el momento
        el cliente lo que hace es armarte la solicitud y enviarlo
