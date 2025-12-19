#include "ComponentArguments.h"

#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"
#include "FluxError.h"
#include "PhysicsUtils.h"

flux_utils::Vector2 flux_script::ComponentArguments::getValueToVector2(const std::string& key) 
{
    auto it = _args.find(key);
    if (it == _args.cend()) {
        writeFluxError("No se pudo encontar el tipo de argumento Vector2 en el script. Se devolvera por defecto (0,0)");
        return flux_utils::Vector2(0, 0);
    }

    const std::string& val = it->second;

    float x = 0.f, y = 0.f;
    {
        size_t pos = val.find('|');
        if (pos == std::string::npos) {
            writeFluxError("Formato incorrecto para un argumento de tipo Vector3 en el script. Se devolvera por defecto (0,0)");
            return flux_utils::Vector2(0, 0);
        }

        std::string sx = val.substr(0, pos);
        std::string sy = val.substr(pos + 1);

        try {
            x = std::stof(sx);
            y = std::stof(sy);
        }
        catch (...) {
            writeFluxError("Formato incorrecto para uno de los componentes de un Vector2 en el script. Se devolvera por defecto (0,0)");
            return flux_utils::Vector2(0, 0);
        }
    }

    return flux_utils::Vector2(x, y);
}

flux_utils::Vector3 flux_script::ComponentArguments::getValueToVector3(const std::string& key)
{
    auto it = _args.find(key);
    if (it == _args.end()) {
        //Simplemente se informa de que ha ocurrido un error y se devulve por defecto un vector (0,0,0)
        writeFluxError("No se pudo encontar el tipo de argumento Vector3 en el script. Se devolvera por defecto (0,0,0)");
        return flux_utils::Vector3(0, 0, 0); 
    }

    // Ejemplo: la string es "0|1|0"
    const std::string& val = it->second;

    // Dividir por '|' 
    // (hay muchas formas: std::stringstream, manual, etc.)
    float x = 0.f, y = 0.f, z = 0.f;
    {
        size_t pos1 = val.find('|');
        if (pos1 == std::string::npos) {
            // error (no hay ni una barra vertical)
            writeFluxError("Formato incorrecto para un argumento de tipo Vector3 en el script. Se devolvera por defecto (0,0,0)");
            return flux_utils::Vector3(0, 0, 0);
        }
        size_t pos2 = val.find('|', pos1 + 1);
        if (pos2 == std::string::npos) {
            // error
            writeFluxError("Formato incorrecto para un argumento de tipo Vector3 en el script. Se devolvera por defecto (0,0,0)");
            return flux_utils::Vector3(0, 0, 0);
        }
        // Extraer substrings
        std::string sx = val.substr(0, pos1);
        std::string sy = val.substr(pos1 + 1, pos2 - (pos1 + 1));
        std::string sz = val.substr(pos2 + 1);

        try {
            x = std::stof(sx);
            y = std::stof(sy);
            z = std::stof(sz);
        }
        catch (...) {
            writeFluxError("Formato incorrecto para uno de los componentes de un Vector3 en el script. Se devolvera por defecto (0,0,0)");
            return flux_utils::Vector3(0, 0, 0);
        }
    }

    return flux_utils::Vector3(x, y, z);
}

flux_utils::Vector4 flux_script::ComponentArguments::getValueToVector4(const std::string& key)
{
    auto it = _args.find(key);
    if (it == _args.end()) {
        // Escribir error y devolver (0,0,0,0)
        writeFluxError("No se pudo encontar el tipo de argumento Vector4 en el script. Se devolvera por defecto (0,0,0,0)");
        return flux_utils::Vector4(0, 0, 0, 0);
    }

    // Ejemplo: la string es "0|1|0"
    const std::string& val = it->second;

    // Dividir por '|' 
    // (hay muchas formas: std::stringstream, manual, etc.)
    float x = 0.f, y = 0.f, z = 0.f, w = 0.f;
    {
        size_t pos1 = val.find('|');
        if (pos1 == std::string::npos) {
            // error (no hay ni una barra vertical)
            writeFluxError("Formato incorrecto para un argumento de tipo Vector4 en el script. Se devolvera por defecto (0,0,0,0)");
            return flux_utils::Vector4(0, 0, 0, 0);
        }
        size_t pos2 = val.find('|', pos1 + 1);
        if (pos2 == std::string::npos) {
            // error
            writeFluxError("Formato incorrecto para un argumento de tipo Vector4 en el script. Se devolvera por defecto (0,0,0,0)");
            return flux_utils::Vector4(0, 0, 0, 0);
        }
        size_t pos3 = val.find('|', pos2 + 1);
        if (pos3 == std::string::npos) {
            writeFluxError("Formato incorrecto para un argumento de tipo Vector4 en el script. Se devolvera por defecto (0,0,0,0)");
            return flux_utils::Vector4(0, 0, 0, 0);
        }
        // Extraer substrings
        std::string sx = val.substr(0, pos1);
        std::string sy = val.substr(pos1 + 1, pos2 - (pos1 + 1));
        std::string sz = val.substr(pos2 + 1, pos3 - (pos2 + 1));
        std::string sw = val.substr(pos3 + 1);

        try {
            x = std::stof(sx);
            y = std::stof(sy);
            z = std::stof(sz);
            w = std::stof(sw);
        }
        catch (...) {
            writeFluxError("Formato incorrecto para uno de los componentes de un Vector3 en el script. Se devolvera por defecto (0,0,0,0)");
            return flux_utils::Vector4(0, 0, 0, 0);
        }
    }

    return flux_utils::Vector4(x, y, z, w);
}

bool flux_script::ComponentArguments::getValueToBool(const std::string& key)
{
	auto it = _args.find(key);
	if (it != _args.cend()) {
		std::string val = it->second;
		for (char& c : val) {
			c = static_cast<char>(tolower(c));
		}
		return (val == "true" || val == "1");
	}
	else {
        writeFluxError("Formato incorrecto para un argumento de tipo booleano en el script. Se devolvera por defecto false");
        return false;
	}
}

int flux_script::ComponentArguments::getValueToInt(const std::string& key)
{
	auto it = _args.find(key);

    if (it != _args.cend()) return std::stoi(it->second);
    else {
        writeFluxError("Formato incorrecto para un argumento de tipo int en el script. Se devolvera por defecto 0");
        return 0;
    }
}

float flux_script::ComponentArguments::getValueToFloat(const std::string& key)
{
	auto it = _args.find(key);

	if (it != _args.cend()) return std::stof(it->second);
    else {
        writeFluxError("Formato incorrecto para un argumento de tipo float en el script. Se devolvera por defecto 0.0f");
        return 0.0f;
    }
}

std::string flux_script::ComponentArguments::getValueToString(const std::string& key)
{
	auto it = _args.find(key);
	if (it != _args.cend()) return it->second;
    else {
        writeFluxError("Formato incorrecto para un argumento de tipo string en el script. Se devolvera por defecto una cadena vacia");
        return ""; // Cadena vac�a por defecto
    } 
}

flux_utils::shapeType flux_script::ComponentArguments::getValueToShapeType(const std::string& key) {

    auto it = _args.find(key);

    if(it != _args.cend())
    {
        if (it->second == "BOX") return flux_utils::shapeType::BOX;
        if (it->second == "SPHERE") return flux_utils::shapeType::SPHERE;
        if (it->second == "CAPSULE") return flux_utils::shapeType::CAPSULE;
        if (it->second == "CYLINDER") return flux_utils::shapeType::CYLINDER;
    }
    else {
        writeFluxError("Formato incorrecto para un argumento de tipo shapeType en el script. Se devolvera por defecto BOX collider");
        return flux_utils::shapeType::BOX;
    }
}

flux_utils::rigidBodyType flux_script::ComponentArguments::getValueToRBType(const std::string& key) {

    auto it = _args.find(key);
    if (it != _args.cend())
    {
        if (it->second == "DYNAMIC") return flux_utils::rigidBodyType::DYNAMIC;
        if (it->second == "KINEMATIC") return flux_utils::rigidBodyType::KINEMATIC;
        if (it->second == "STATIC") return flux_utils::rigidBodyType::STATIC;
    }

    else {
        writeFluxError("Formato incorrecto para un argumento de tipo RigidBodyType en el script. Se devolvera por defecto DYNAMIC");
        return flux_utils::rigidBodyType::DYNAMIC;
    }
}

std::list<std::string> flux_script::ComponentArguments::getValueToListString(const std::string& key) {
    auto it = _args.find(key);
    if (it == _args.end()) {
        //Simplemente se informa de que ha ocurrido un error y se devulve por defecto un vector (0,0,0)
        writeFluxError("No se pudo encontar el tipo de argumento list<string> en el script. Se devolvera por defecto una lista vacia");
        return std::list<std::string>();
    }

    // Ejemplo: la string es "0|1|0"
    const std::string& val = it->second;

    std::list<std::string> lista;

    size_t pos1 = val.find('|');
    if (pos1 == std::string::npos) {
        // error (no hay ni una barra vertical)
        lista.push_back(val);
        return lista;
    }
    lista.push_back(val.substr(0, pos1));
    size_t pos2 = val.find('|',pos1+1);
    while (pos2 != std::string::npos) {
        lista.push_back(val.substr(pos1 + 1, pos2 - (pos1 + 1)));
        pos1 = pos2;
        pos2 = val.find('|',pos1 +1);
    }
    lista.push_back(val.substr(pos1 + 1, val.length() - (pos1 + 1)));
    
    return lista;
}

FLUX_API std::list<bool> flux_script::ComponentArguments::getValueToListBool(const std::string& key) {
    auto it = _args.find(key);
    if (it == _args.end()) {
        //Simplemente se informa de que ha ocurrido un error y se devulve por defecto un vector (0,0,0)
        writeFluxError("No se pudo encontar el tipo de argumento list<bool> en el script. Se devolvera por defecto una lista vacia");
        return std::list<bool>();
    }

    // Ejemplo: la string es "0|1|0"
    const std::string& val = it->second;

    std::list<bool> lista;

    size_t pos1 = val.find('|');
    if (pos1 == std::string::npos) {
        // error (no hay ni una barra vertical)
        lista.push_back(val == "true" || val == "1");
        return lista;
    }
    std::string aux = val.substr(0, pos1);
    lista.push_back(aux == "true" || aux == "1");
    size_t pos2 = val.find('|', pos1 + 1);
    while (pos2 != std::string::npos) {
        aux = val.substr(pos1 + 1, pos2 - (pos1 + 1));
        lista.push_back(aux == "true" || aux == "1");
        pos1 = pos2;
        pos2 = val.find('|', pos1 + 1);
    }
    aux = val.substr(pos1 + 1, val.length() - (pos1 + 1));
    lista.push_back(aux == "true" || aux == "1");

    return lista;
}

void flux_script::ComponentArguments::setArg(const std::string& key, const std::string& value)
{
	auto it = _args.find(key);

	if (it == _args.cend()) _args.insert({ key, value });
	else it->second = value;
}