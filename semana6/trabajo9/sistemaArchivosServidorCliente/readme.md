# Funcionamiento en conjunto del sistema de archivos con el servidor y el cliente

Este programa es desarrollado en C++.


Se tiene un generador de archivo binario que lo procesará el sistema de archivos (con base de inspiración un tipo FAT) para que el servidor (que utiliza hilos) lo pueda utilizar y cumplir con la solicitud del cliente.


Para ver el programa en acción, necesita dos terminales, una donde ejecuta el servidor y otra donde ejecuta el cliente. En el cliente se le despliegan dos opciones enumeradas, 1 y 2, cuando el usuario digite el número, debe dar enter para continuar con el programa.


Al ejecutar el cliente en la terminal, se le presenta al usuario la opción 1 que puede ver la lista de figuras tipo ASCII disponibles en el servidor y como opción 2 puede agregar una figura de tipo ASCII, para agregar la figura nueva, debe colocar la nueva figura en el archivo de texto llamado "otra.txt", pero antes debe eliminar la figura que esté allí, cuando esté en la terminal del cliente haciendo la acción de agregar figura, se le solicitará un nombre para esta y luego de digitar enter, se le notifica al usuario que la acción realizada fue exitosa, ahora, para poder visualizar esta nueva figura, debe volver a ejecutar el cliente para seleccionar la opción 1 y digitar el nombre que le asignó a su figura.


Cuando se empieza a dar el flujo de las solicitudes del cliente con el servidor, se puede notar que el servidor llega un registro de tipo resumido.


Además si se solicita una figura que no existe, se le muestra un mensaje al usuario que menciona que esa figura no existe. 


# Compilar y ejecutar

- Abrir la terminal 1.
- Posicionarse en el directorio del proyecto: ` cd ./proyectoIntegrador2/semana6/tarea10/sistemaArchivosServidorCliente `
- Limpiar el proyecto con: ` make clean `
- Compilar el proyecto con: ` make `
- Ejecutar el servidor con: ` ./ThreadMirrorServer.out `


- Abrir una segunda terminal.
- Compilar el cliente con: ` make MirrorClient.out `
- Ejecutar el cliente con: ` ./MirrorClient.out `