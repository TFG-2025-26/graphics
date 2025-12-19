#pragma once

#ifndef FLUX_ERROR_H_
#define FLUX_ERROR_H_


// Archivo de cabecera que define dos macros para la gestión de errores de Flux Engine. Se debe incluir en todos los archivos en los
// que se quiera lanzar errores.
// 
//Si se está en modo debug, se imprime el mensaje de error por consola
#ifdef _DEBUG
#include <iostream>
#define writeFluxError(msg) std::cerr << "Error de Flux Engine: " << msg << "\n"
#else
#define writeFluxError(msg)
#endif
//Función que devuelve el valor de la función y la excepción junto a su mensaje
#define throwFluxError(value, msg) writeFluxError(msg); return value

#endif // FLUX_ERROR_H_
