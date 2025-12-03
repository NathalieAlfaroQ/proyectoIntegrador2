## Etapa 4


En una terminal ./HTTPServer.out 
En otra terminal ./cache.out 
En otra terminal ./Intermediario.out 


En el navegador:
Consultar la lista de figuras:
http://127.0.0.1:8080/LIST /BEGIN//END/


Obtener una figura en especifico:
http://127.0.0.1:8080/GET /BEGIN/jessica.txt/END/


Si no hay figuras en el listado, hay que agregarlas en otra terminal con:*
Agregar figuras al listado, solo en la terminal del cliente:
./HTTPClient.out --add=jessica.txt


*Volver a hacer LIST para ver que ha sido agregada y asi poder hacer GET y desplegarla en el navegador.