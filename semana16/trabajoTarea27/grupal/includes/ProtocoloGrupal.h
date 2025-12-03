#pragma once

#include <string>

#include "Socket.h"
#include "FileSystem.h"

/// @brief Estructura de la petición del Protocolo Grupal (PG)
struct PG_Message {
    /// @brief El método PG.
    std::string method = "";
    /// @brief El nombre del archivo
    std::string filename = "";
    /// @brief El tamaño del cuerpo de la figura
    int size = 0;
    /// @brief La IP del host
    std::string ip = "";
    /// @brief El nombre del host
    std::string hostname = "";
    /// @brief El nombre del host
    char operation = ' ';
    /// @brief El cuerpo de la petición, si tiene.
    std::string cuerpo = "";
};

/// @brief Parsea la solicitud PG y la valida.
/// @param request El mensaje de la solicitud del PG.
/// @return Devuelve la cadena de la solicitud PG.
struct PG_Message parseAndVaildatePG_Request(const std::string& request);
/// @brief Construye una solicitud PG simple con el comando y la dirección.
/// @param method El comando a emplear, como GET, ADD o DELETE.
/// @param header La información del encabezado (entre END y BEGIN).
/// @param body el cuerpo HTML de la request, si tiene.
/// @return Devuelve la cadena de la solicitud PG.
std::string buildPG_Request(const std::string& method, const std::string& header, const std::string& body);
/// @brief Construye la respuesta de LIST el directorio
/// @param request La solicitud parseada. Debe ser válida
/// @param fs El sistema de archivos de donde obtener la figura
/// @return Devuelve un string con la respuesta PG  y la figura
std::string handlePG_LIST(FileSystem& fs);
/// @brief Construye la respuesta de GET con el archivo
/// @param request La solicitud parseada. Debe ser válida
/// @param fs El sistema de archivos de donde obtener el directorio
/// @return Devuelve un string con la respuesta PG del GET
std::string handlePG_GET(const std::string& filename, FileSystem& fs);
/// @brief Construye la respuesta de POST
/// @param request La solicitud parseada. Debe ser válida
/// @param fs El sistema de archivos de donde agregar la figura
/// @return Devuelve un string con la respuesta PG del POST
std::string handlePG_ADD(const std::string& filename, const std::string& body, FileSystem& fs);
/// @brief Construye la respuesta de DELETE
/// @param request La solicitud parseada. Debe ser válida
/// @param fs El sistema de archivos de donde eliminar la figura
/// @return Devuelve un string con la respuesta PG del DELETE
std::string handlePG_DELETE(const std::string& filename, FileSystem& fs);
/// @brief Construye la respuesta a una petición de CONNECT
/// @param id La id de quien envía el mensaje
/// @param hostname El nombre del host que envía el mensaje
/// @param fs El sistema de archivos de donde eliminar la figura
/// @return Devuelve la respuesta al CONNECT
std::string handlePG_CONNECT(const int id, const std::string& hostname, FileSystem& fs);
/// @brief Construye la respuesta del UPDATE
/// @param success Indica el update tuvo éxito
/// @param request El mensaje de UPDATE recibido
/// @return Devuelve la respuesta del UPDATE
std::string handlePG_UPDATE(bool success, const std::string& request);
/// @brief Crea una respuesta con formato PG.
/// @param code El código de estado PG (200, 404, 500, etc.)
/// @param HTMLBody El contenido del cuerpo de la respuesta.
/// @return Devuelve la cadena de la respuesta PG.
std::string buildPG_Response(int code, const std::string& HTMLBody);
/// @brief Procesa la respuesta y extrae sus partes.
/// @param response La respuesta completa.
/// @return Una pareja con el código de estado y el cuerpo HTML.
std::pair<int, std::string> parsePG_Response(std::string& response);