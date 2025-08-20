# Se ejecuta con: python3 server.py

'''
Crea un servidor HTTP en IPv6 que escucha en :: (todas las interfaces) y puerto 8080.
Si el usuario accede a /ip, devuelve su dirección IP.
Para otras rutas, usa el manejador HTTP normal (SimpleHTTPRequestHandler).
El servidor nunca se detiene (serve_forever()).
'''

# Para trabajar con direcciones y protocolos de red.
import socket

# Implementa un servidor HTTP básico / Maneja las solicitudes HTTP.
from http.server import HTTPServer, SimpleHTTPRequestHandler

'''
Manejador de Peticiones HTTP:

Hereda de SimpleHTTPRequestHandler para manejar peticiones HTTP.
Sobrescribe do_GET() para modificar la respuesta cuando se accede a /ip:
  Envía una respuesta 200 OK.
  Especifica que el contenido es text/html.
  Imprime la dirección IP del cliente (self.client_address[0]).
Si la ruta no es /ip, usa el comportamiento estándar de SimpleHTTPRequestHandler.
'''
class MyHandler(SimpleHTTPRequestHandler):
  def do_GET(self):
    # Si el usuario accede a /ip.
    if self.path == '/ip':
      # Código HTTP 200 (OK).
      self.send_response(200)
      # Tipo de contenido
      self.send_header('Content-type', 'text/html')
      # Fin de los encabezados.
      self.end_headers()
      # Enviar IP del cliente.
      self.wfile.write(f'Your IP address is {self.client_address[0]}'.encode())
      return
    else:
      # Manejo normal de otros GET
      return SimpleHTTPRequestHandler.do_GET(self)

# Servidor HTTP en IPv6.
class HTTPServerV6(HTTPServer):
  # Define que usará IPv6.
  address_family = socket.AF_INET6

def main():
  # Escucha en todas las direcciones IPv6 (::) en el puerto 8080.
  server = HTTPServerV6(('::', 8080), MyHandler)
   # Mantiene el servidor en ejecución.
  server.serve_forever()

if __name__ == '__main__':
  main()