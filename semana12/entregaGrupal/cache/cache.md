# Política de reemplazo

Usare LFU, ya que este se basa en frecuencia de las peticiones y parece sencillo de implementar, mis argumentos para no usar los otros
algoritmos o politicas aplicadas son las siguientes:

FIFO: Los problemas que presenta a la hora de trabajar que puede causar una deficiencia, ya que elimina sin tomar en cuenta si es muy pedida o poco.

LRU: Una figura puede que sea pedida cada tanto, pero de manera constante, mientras otra puede ser un caso excepcional, y eso nos lleva a la posibilidad de que en algun momento la figura constante sea eliminada en lugar de aquella excepcional, sin tomar en cuenta esa propiedad de constancia.

## Ajustes

Una figura mayor a 8kb se intentara guardar, incluso si no cabe por completo. Eliminando el resto de las figuras en base a la politica seleccionada. un mismatch al servidor cache sera respondido como un error 404 objeto no encontrado y manejado por el servidor de figuras

# Interacciones y modificaciones al protocolo

La misma con unas ligeras modificaciones:

* El servidor cache estara al final de la cadena tenedor-figuras-**cache**, por lo tanto los puertos definidos seran: 8080, tenedor; 8081, figura; 8082, cache.

* Figuras redireccionara directamente todas las solicitudes get a cache, si no tiene la figura, al regresar se extraera la figura buscada del fs y se le enviara una solicitud add con la figura al cache y se le agregara segun las politicas de reemplazo.

* Figuras puede trabajar con o sin  el servidor cache, aunque con cada solicitud get buscara establecer una conexion.

# Diagrama

![diagrama cache](Diagrama_cache.svg)
