// Encabezado
#ifndef LECTOR_FIGURA_H
#define LECTOR_FIGURA_H

// Bibliotecas
#include <string>
#include <vector>

// Métodos
std::string leer_figura(const std::string &nombreFigura);
std::vector<std::string> listar_figuras();
std::string agregar_figura(const std::string &nombre_figura, const std::string &contenido);

#endif // LECTOR_FIGURA_H 