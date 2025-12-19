-- Definir el TÍTULO del juego
gameTitle = "Dummy Game"

/*-- Definir la RESOLUCIÓN de la pantalla (COMPROBAR SI DEFINIMOS TAMAÑO DE PANTALLA)
screenWidth = 1280
screenHeight = 720*/

-- Definir los parametros de la CAMARA
camera = {
    id = 1,
    name = "CameraEntity"
    -- Datos del Transform
    transform = {
        pos = { x = 0.0f, y = 0.0f, z = 40.0f },
        rot = { x = 0.0f, y = 0.0f, z = 0.0f, w = 1.0f },
        scl = { x = 1.0f, y = 1.0f, z = 1.0f }
    }
    -- Datos del Camera component
    CCamera = {
        nearDistance = 1,
        farDistance = 10000
    }
}

-- Definir la LUZ
luz = {
    id = 2,
    name = "LightEntity"
    -- Datos del Transform
    transform = {
        pos = { x = 0.0f, y = 30.0f, z = 0.0f },
        rot = { x = 0.382f, y = 0.0f, z = 0.0f, w = 0.923f },
        scl = { x = 1.0f, y = 1.0f, z = 1.0f }
    }
    -- Datos del Light component
    CLight = {
        diffColor = { 1.0f, 1.0f, 1.0f },
        farDistance = 10000
    }
}

-- Definir un cubo dummy (TEMPORAL)
cube = {
    id = 3,
    name = "CubeEntity"
    -- Datos del Transform
    transform = {
        pos = { x = 0.0f, y = 30.0f, z = 0.0f },
        rot = { x = 0.0f, y = 0.0f, z = 0.0f, w = 1.0f },
        scl = { x = 0.2f, y = 0.2f, z = 0.2f }
    }
    -- Datos del Mesh component
    CMesh = {
        meshName = "ogrehead.mesh"
    }
}

