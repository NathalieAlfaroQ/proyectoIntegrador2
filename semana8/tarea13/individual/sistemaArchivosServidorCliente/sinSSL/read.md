Esta versión es del sistema de archivos más funcional, con un servidor y cliente de formato HTTP para verlo en la web, esta versión del programa utiliza los sockets normales.

Para compilar, hay que usar primero una terminal para tener el servidor encendido y en el navegador web hacemos las interacciones.

En una terminal:
- Nos posicionamos en el directorio: ./proyectoIntegrador2/semana7/tarea11/sistemaArchivosServidorCliente/sinSSL
- Limpiar el proyecto: make clean
- Compilar el proyecto con: make
- Abrir el navegador web Firefox (solo ahí funciona completamente)
- En la barra de navegación del navegador, se pueden utilizar 3 comandos

- Para ver la lista de las figuras disponibles: http://127.0.0.1:5013/listado
- Para agregar una figura: http://127.0.0.1:5013/add?nombre=jessica
- Para ver la figura agregada: http://127.0.0.1:5013/figura?nombre=jessica.txt