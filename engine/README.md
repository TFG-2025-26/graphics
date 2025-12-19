# Estructura del Motor

## Glosario
	-Módulos & Departamentos
	-Carpetas
	-EC vs ECS
	-Reparto tareas & Comunicación
 	-Guía Estilo GitHub Proyects
	-Compilación Automatizada

## Módulos & Departamentos
	- Input: SDL_Sensor para el giroscopio
		Integrantes: Dorjee
	- Renderizado: Ogre
		Integrantes: Nacho & Diego
	- Físicas: Bullet
		Integrantes: Manu & Jose
	- Sonido: FMOD
		Integrantes: Luisja
	- Carga de Datos: LUA
		Integrantes: Nacho
	- Utils
		Integrantes: Jiale 

## Carpetas
	La carpeta Bin será donde se generan todos los archivos finales durante el proceso de compialación.

	La corpeta projects almcenará cada módulo del motor.

	La carpeta src será donde se almacenen los archivos de encabezado e implementación.

	La carpeta assets tendrá todos los assets del juego.

## EC vs ECS
	Usamos EC porque el uso de ECS seria mucho más complejo de implementar, las entities encapsulan datos y comportamientos, cada componente se autogestiona a sí mismos y el uso des sistemas complica más la gestión 
	de instanciar las entidades.

## Reparto de Tareas y Comunicación del Grupo
	Se utilizará Github Projects al ser más convenientes pra nosotros, con todo el proyecto y motor centralizado en un solo sitio. 
	Se repartirán las tareas por módulos en grupos de 2,3 y 4 dependiendo de la dificultad de la tarea.

	Cada semana se tendrá una reunión donde cada miembro dirá que ha tocado y cómo ha avanzado para dar feedback al resto del grupo y dar una 
	idea general del progreso del motor. Esto nos ayudará en caso de posibles atascos si una tarea es demasiado extensa para reajustar la planificación del sprint actual.

	En estas reuniones se decidirán también la toma de decisiones conjuntamente.

## GitHub  Proyects
	CREACIÓN Y GESTIÓN DE HISTORIAS
		Crear una Historia de Usuario:
			Dentro del proyecto, selecciona New Issue o Add item.
  
		Formato estándar historias:
			Cada historia debe seguir la estructura: "Como [tipo de usuario] quiero [acción] para [beneficio]."
  
  			Ejemplo: <<Como desarrollador quiero una API documentada para reducir el tiempo de integración.>>

  		Criterios de Aceptación:
   			Se deben incluir criterios de aceptación claros y verificables utilizando el formato Given-When-Then (Dado-Cuando-Entonces), esto es cómo debe funcionar el Proyecto una vez esté terminada la implementación de la historia:
			Given: Contexto inicial
			When: Acción realizada
			Then: Resultado esperado

   			Ejemplo: 
	MILESTONES
		Los hitos representan entregables clave dentro del proyecto y deben estar alineados con objetivos específicos.
		
  		Convenciones de Nombres:
   			vX.Y - [Descripción]
    			Ejemplo: v1.0 - Lanzamiento Inicial

		Feature - [Nombre]
			Ejemplo: Feature - Autenticación de Usuarios

 		 Criterios de Finalización:
			Un hito se considera completo cuando:
			Todas las historias asignadas han sido cerradas.
			Se han realizado revisiones de código necesarias.
			Ha sido aprobado en la reunión de equipo correspondiente.

	TAGS
		Las etiquetas permiten una rápida categorización de las historias y tareas dentro del proyecto.

 		Tipos de Etiquetas
			Prioridad: Alta, Media, Baja
			Estado: En progreso, Pendiente, Revisión, Completado
			Departamento: Backend, Frontend, QA, UI/UX, DevOps
			Tipo de tarea: Bug, Mejora, Nueva Funcionalidad, Investigación

 	FLUJO DE TRABAJO
  		1. Creación -> Se configura nombre siguiendo este guía de estilo, se asocian miembros del Departamento involucrado, tiempo estimado, tags de Departamento, Prioridad, Estado y Tipo de Tarea.
    		2. Progresión -> La tarea se sitúa en la columna adecuada del Proyects en cada momento. Si es necesario hacer cambios en la Descripción de la tarea, en la estimación de tiempo. o mimebros implicados se configura.
      		3. Cierre -> Se determna el tiempo tomada para realizar la tarea. Esta se cierra una vez haya sido completada y revisada mediante un Pull Request.
	
## Compilación Automatizada

### Ejecutar .bat de módulo
	1. Clonar repo
	2. Abrir Developer Commmand Prompt
	3. cd dir .bat del módulo a compilar (Ej: ./2425-Grupo04-FluxEngine\dependencies\Ogre)
	3. build .bat (si pide F = archivo D = Directorio poner D)

# Ejecutar .bat padre
	1. Clonar repo
	2. Abrir Developer Commmand Prompt
	3. Usar comando cd dir .bat padre situado en ./2425-Grupo04-FluxEngine
	4. Usar comando build .bat (si pide F = archivo D = Directorio poner D)
