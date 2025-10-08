# Simulación

Se realizó un prototipo para la ejecución del servidor, tenedor y cliente, simulando
de forma parcial el trabajo que llevan a cabo utilizando el protocolo que se
trabajó en clase junto al equipo 2. Se incluye en otra carpeta aparte.

Este consiste en pedirle al servidor un ASCII art y el servidor le responde,
habiendo un tenedor como intermediario.

Para que hubiera comunicación entre procesos, se utilizó pipes, ya
que integran el control de concurrencia y permiten a los procesos
comunicarse sin tener que realizar mucho labor en el código.

## Ejecución

Para compilar puede usar el make con el comando
```
make
```
en la terminal, ubicándose la carpeta `src`.

Para ejecutarlo, puede ejecutar el comando:
```
./bin/simulacion
```

Esta solicitud la realiza por defecto.

## Casos de prueba

Para probar el programa, este incluye una interfaz interactiva. Primero se
escribe un comando entre "GET, ADD, DELETE, LIST, EXIT".

Si es LIST, no ocupa más datos, por lo que funcionará de una vez. Casos como
GET o DELETE necesitan el nombre del archivo, el cual se solicita después de
ingresar el comando y enter.