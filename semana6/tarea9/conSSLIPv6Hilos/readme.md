SSL socket-servidor-cliente IPv6 con certificados
Con hilos

Para compilar se hace: make

Luego para generar los certificados si no existen se pone en la terminal: openssl req -x509 -nodes -days 365 -newkey rsa:2048 -keyout key0123.pem -out ci0123.pem

Luego hara preguntas de sus datos y genera los .pem

Luego se ejecuta el servidor con el puerto: ./SSLServer 1234

En otra terminal se ejecuta el cliente con la direccion IP y el puerto: ./SSLClient ::1 1234

Esta direccion IP, ::1, indica que la computadora se comunica consigo misma 

Cuando se ejecuta el cliente, nos pide unos datos que puede poner los que quiera, da mensaje invalido porque no son los que el profe proporciona