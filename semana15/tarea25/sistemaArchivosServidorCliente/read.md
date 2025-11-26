## Etapa 3

Para esta etapa se tiene un servidor de tipo HTTP y un cliente de tipo HTTP que funcionan en un navegador, pero
solo funciona por completo en el navegador de FireFox, también se tiene el sistema de archivos y una caché. Además
cabe resaltar qu todo este programa utiliza sockets y se compila con Makefile.

El flujo de navegación es: El cliente solicita una figura al servidor, entonces el servidor le pregunta a la
caché si tiene la figura y aquí se divide la respuesta, si la caché tiene la figura solicitada, se la puede
dar al servidor y el servidor se la da al cliente. Pero si caché no tiene la figura se lo comunica al servidor
y así el servidor se encarga de ir al sistema de archivos a sacarla para posteriormente darsela al cliente y
guardarla en la caché, para que en un futuro si vuelven a solicitar esa figura, pues esta vez la caché ya la
posea y así tener menos accesos al sistema de archivos.


## Compilar y ejecutar:

En una terminal:
- Nos posicionamos en el directorio: ./proyectoIntegrador2/semana12/trabajo20/sistemaArchivosServidorCliente
- Limpiar el proyecto: make clean
- Compilar el proyecto con: make
- Ejecutar el servidor en una terminal con: ./HTTPServer.out
- Ejecutar la caché en otra terminal con: ./cache.out
- Abrir el navegador web Firefox (solo ahí funciona completamente)
- En la barra de navegación del navegador, se pueden utilizar 3 comandos:
    - Listar figuras: http://127.0.0.1:9090/listado
    - Agregar figura: http://127.0.0.1:9090/add?nombre=jessica
    - Ver figura: http://127.0.0.1:9090/figura?nombre=jessica