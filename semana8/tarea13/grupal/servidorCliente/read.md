En esta carpeta se encuentra el proyecto del sistema de archivos funcionando en conjunto con el servidor y el cliente, estos son en HTTP y usan sockets normales, por lo cual se visualiza en la red y en un navegador web, el único navegador web que funciona completamente es el Firefox.


Para compilar, hay que usar primero una terminal para tener el servidor encendido y en el navegador web hacemos las interacciones.


En una terminal:
- Nos posicionamos en el directorio: ./etapa_II/servidorCliente
- Limpiar el proyecto: make clean
- Compilar el proyecto con: make
- Ejecutamos el servidor: ./HTTPServer.out
- Abrir el navegador web Firefox (solo ahí funciona completamente)
- En la barra de navegación del navegador, se pueden utilizar 2 comandos:
    - Para ver la lista de las figuras disponibles: http://127.0.0.1:5013/listado
    - Para ver la figura agregada: http://127.0.0.1:5013/figura?nombre=jessica.txt

- Si el cliente quiere agregar una figura de tipo ASCCI, se debe abrir otra terminal y ejecutar el cliente, para agregar la figura en la terminal:
- make HTTPClient.out
- ./HTTPClient.out add pantera
En el navegador se realizan las siguientes operaciones
- http://127.0.0.1:5013/figura?nombre=pantera.txt - pide una figura
- http://127.0.0.1:5013/listado - pide la lista de figuras
