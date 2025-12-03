#include <string>
#include <iostream>
#include <regex>

#include "ProtocoloGrupal.h"
#include "Socket.h"
#include "FileSystem.h"

#define OK_CODE 200
#define BAD_REQUEST_CODE 400
#define FILE_NOT_FOUND_CODE 404
#define INTERNAL_ERROR_CODE 500

struct PG_Message parseAndVaildatePG_Request(const std::string& request) {
    struct PG_Message parsedMessage;

    // Primero se verifica los métodos
    std::regex listMsgRegex(R"(^(LIST) /BEGIN/ /END/)");

    std::regex getMsgRegex(R"(^(GET) /BEGIN/(.+?)/(\d+)/END/)");

    std::regex deleteMsgRegex(R"(^(DELETE) /BEGIN/(.+?)/END/)");

    std::regex connectMsgRegex(R"(^(CONNECT) /BEGIN/(\d+?\.\d+?\.\d+?\.\d+?)/(.+?)/END/)");

    std::regex quitMsgRegex(R"(^(QUIT) /BEGIN/(\d+?\.\d+?\.\d+?\.\d+?)/END/)");

    std::regex updateMsgRegex(R"(^(UPDATE) /BEGIN/(A|D|Q|C)/(.+?)/(\d+)/END/ (.+?))");

    std::regex addMsgRegex(R"(^(ADD) /BEGIN/(.+?)/(\d+)/END/ (.+?))");

    std::smatch msgMatches;

    // Caso: list
    if (std::regex_match(request, msgMatches, listMsgRegex)) {
        parsedMessage.method = msgMatches[1];

    // Caso: get
    } else if (std::regex_match(request, msgMatches, getMsgRegex)) {
        parsedMessage.method = msgMatches[1];
        parsedMessage.filename = msgMatches[2];

    // Caso: delete
    } else if (std::regex_match(request, msgMatches, deleteMsgRegex)) {
        parsedMessage.method = msgMatches[1];
        parsedMessage.filename = msgMatches[2];

    // Caso: connect
    } else if (std::regex_match(request, msgMatches, connectMsgRegex)) {
        parsedMessage.method = msgMatches[1];
        parsedMessage.ip = msgMatches[2];
        parsedMessage.hostname = msgMatches[3];

    // Caso: quit
    } else if (std::regex_match(request, msgMatches, quitMsgRegex)) {
        parsedMessage.method = msgMatches[1];
        parsedMessage.ip = msgMatches[2];

    // Caso: update
    } else if (std::regex_match(request, msgMatches, updateMsgRegex)) {
        parsedMessage.method = msgMatches[1];
        parsedMessage.operation = std::string(msgMatches[2]).at(0);
        parsedMessage.hostname = msgMatches[3];
        parsedMessage.size = std::stoi(msgMatches[4]);
        parsedMessage.cuerpo = msgMatches[5];

    // Caso: add
    } else if (std::regex_match(request, msgMatches, addMsgRegex)) {
        parsedMessage.method = msgMatches[1];
        parsedMessage.filename = msgMatches[2];
        parsedMessage.size = std::stoi(msgMatches[3]);
        parsedMessage.cuerpo = msgMatches[4];

    } else {
        parsedMessage.method = "BAD REQUEST";
    }


    return parsedMessage;
}

std::string buildPG_Request(const std::string& method, const std::string& header, const std::string& body) {
    return method + " " + header + " " + body;
}

std::string handlePG_LIST(FileSystem& fs) {
    // Obtener directorio en HTML
    std::string directoryHTML = fs.list();

    // Construir respuesta
    const std::string HTMLbody = "<h1>Directorio de Figuras ASCII</h1>\n" + directoryHTML;
    int code = OK_CODE;

    return buildPG_Response(code, HTMLbody);
}

std::string handlePG_GET(const std::string& filename, FileSystem& fs) {
    // Información del Mensaje
    std::string answerContent;
    int code;

    // Obtener figura del File System
    std::string figure = fs.get(filename);

    if (figure != "") {
        // Armar respuesta con la figura
        answerContent = "\t<h1>Figura " + filename + "</h1>\n"
                        + "\t<pre>\n" + figure + "\n\t</pre>\n";
        int code = OK_CODE;

    // La figura no se encuentra en el File System
    } else {
        std::cerr << "Error 404: figura no encontrada -> \""
                        << filename.c_str() << "\"" << std::endl << std::endl;
        answerContent = "\t<h1>File Not Found 404</h1>\n"
                        "\t<p>No se encontró figura con el nombre: "
                        + filename + "."+ "</p>\n";
        code = FILE_NOT_FOUND_CODE;

    }

    return buildPG_Response(code, answerContent);
}

std::string handlePG_ADD(const std::string& filename, const std::string& body, FileSystem& fs) {
    // Información del Mensaje
    std::string answerContent;
    int code;

    // Obtener figura del File System
    int error = fs.add(filename, body);

    if (error == 0) {
        // Responder con 200 OK
        answerContent = "\t<h1>Figura \"" + filename + "\" agregada</h1>\n"
                        +  "\t<p>\"Se agregó con éxito " + filename + ".\n";
        code = OK_CODE;

    // Ocurrió un error al agregar la figura
    } else {
        std::cerr << "Error 500: no se logró agregar la figura -> \""
                        << filename << "\"" << std::endl << std::endl;
        code = INTERNAL_ERROR_CODE;
        answerContent = "\t<h1>Internal Error 500</h1>\n"
                        "\t<p>No se logró eliminar la figura: "
                        + filename + "."+ "</p>\n";
    }
    return buildPG_Response(code, answerContent);
}

std::string handlePG_DELETE(const std::string& filename, FileSystem& fs) {
    std::string answerContent;
    int code; // Código del mensaje

    if (filename != "") {
        // Intentar borrarlo
        if (fs.rm(filename) == 0) {
            // Se borró con éxito, se construye la respuesta
            answerContent = "\t<h1>Figura \"" + filename + "\" eliminada</h1>\n"
                            +  "\t<p>\"Se eliminó con éxito " + filename + ".\n";
            code = OK_CODE;

        } else {
            // Problemas al borrar el archivo
            printf("Error: Failed to delete file or file not found\n");
            answerContent =  "\t<h1>Failed to delete file 500</h1>\n"
                            "\t<p>\"No se logró eliminar la imagen " + filename + ".\n";
            code = INTERNAL_ERROR_CODE;
        }
        // LOG: no se reconoció la ruta, ocurrió algún error
        std::cerr << "Error: No se reconoce la ruta para DELETE."
                        << std::endl << filename << std::endl << std::endl;
        answerContent =  "\t<h1>Bad Delete Request 400</h1>\n"
                        "\t<p>\"No se logró eliminar la imagen." + filename + "\n";
        code = BAD_REQUEST_CODE;
    }
    return buildPG_Response(code, answerContent);
}

std::string handlePG_CONNECT(const int id, const std::string& hostname, FileSystem& fs) {
    return buildPG_Response(OK_CODE, "");
}

std::string handlePG_UPDATE(const std::string& request, bool success) {
    // Información del Mensaje
    std::string answerContent;
    int code;

    if (success) {
        // Armar respuesta con la figura
        answerContent = "";
        int code = OK_CODE;

    // La figura no se encuentra en el File System
    } else {
        std::cerr << "Error 500: Error al actualizar el tenedor -> \""
                        << request << "\"" << std::endl << std::endl;
        answerContent = "\t<h1>File Not Found 404</h1>\n"
                        "\t<p>No se procesó correctamente la solicitud:\n"
                        + request + "."+ "</p>\n";
        code = INTERNAL_ERROR_CODE;

    }

    return buildPG_Response(code, answerContent);
}

std::string buildPG_Response(int code, const std::string& HTMLBody) {
    std::string response = "";
    return "/BEGIN/" + std::to_string(code) + "/END/" + response;
}

std::pair<int, std::string> parsePG_Response(std::string& response) {
    std::regex responseRegex(R"(^/BEGIN/(\d+{3})/(\d+?)/END/[ ]?(.+?))");
    std::smatch responseMatches;
    if (std::regex_match(response, responseMatches, responseRegex)) {
        return std::make_pair(std::stoi(responseMatches[1]), responseMatches[3]);
    } else {
        throw std::runtime_error("ProtocoloGrupal: respuesta inválida -> " + response);
    }
}