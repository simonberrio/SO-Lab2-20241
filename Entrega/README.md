# Laboratorio 2 - Sistemas Operativos: Mini Shell "wish"

**Autores:**  
- **Simón Berrio**  
- **Leon Mateo Vélez González**

**Repositorio:** [SO-Lab2-20241](https://github.com/simonberrio/SO-Lab2-20241)

---

## 📂 Estructura del repositorio

```bash
SO-Lab2-20241/
├── Entrega/                     # Carpeta de entrega final
│   ├── wish                     # Ejecutable final del shell
│   ├── wish.c                   # Código fuente de la última versión (v8)
│   └── README.md                # Documento de entrega
│
├── versions/                    # Carpeta de desarrollo y versiones intermedias
│   ├── wish.v1.c                # Versión 1: Lectura básica
│   ├── wish.v2.c                # Versión 2: Ejecución de comandos
│   ├── wish.v3.c                # Versión 3: Comando `exit`
│   ├── wish.v4.c                # Versión 4: Comando `cd`
│   ├── wish.v5.c                # Versión 5: Comando `path`
│   ├── wish.v6.c                # Versión 6: Redirección de salida (`>`)
│   ├── wish.v7.c                # Versión 7: Ejecución paralela (`&`)
│   └── wish.v8.c                # Versión 8: Ejecución por archivo (batch mode)
│
├── README.md                    # Readme general del repositorio## Descripción general

El objetivo del laboratorio fue implementar paso a paso un shell minimalista en lenguaje C, denominado wish, capaz de ejecutar comandos del sistema, manejar rutas, redirección, ejecución paralela y lectura desde archivos.

Cada versión construye sobre la anterior, incorporando progresivamente nuevas funcionalidades hasta lograr un shell funcional con comportamiento similar al de bash.

---

## Evolución por versiones

| Versión | Descripción | Funcionalidades principales |
|----------|--------------|-----------------------------|
| v1 | Shell base | Muestra el prompt wish>, lee e imprime la línea ingresada. |
| v2 | Ejecución de comandos externos | Implementa fork(), execvp() y waitpid(). |
| v3 | Comando interno exit | Permite salir correctamente del shell. |
| v4 | Comando interno cd | Cambia el directorio de trabajo con chdir(). |
| v5 | Comando interno path | Configura los directorios donde buscar ejecutables. |
| v6 | Redirección de salida (>) | Usa dup2() para redirigir stdout hacia archivos. |
| v7 | Ejecución paralela (&) | Soporte de múltiples comandos ejecutados simultáneamente. |
| v8 | Modo batch | Lee y ejecuta comandos desde un archivo de texto. |

Las versiones se encuentran dentro de la carpeta versions/.  
La carpeta Entrega/ contiene la versión final y funcional, lista para evaluación.

---

## Compilación

Desde la carpeta Entrega donde esta wish.c, sin embargo tambien esta la opcion de ejecutarlo desde la carpeta versions

---

## Conclusiones

Durante el desarrollo de este laboratorio se comprendió de forma práctica el funcionamiento interno de un shell en Unix, desde la lectura de comandos hasta la creación y sincronización de procesos.

La implementación de funcionalidades como redirección y ejecución paralela permitió afianzar el manejo de descriptores de archivos, procesos hijos y control de flujo mediante llamadas al sistema. Asimismo, se reforzó el uso de herramientas de control de versiones (Git) para mantener un proceso de desarrollo ordenado y modular.

El resultado final es un programa estable, funcional y cercano en comportamiento a un shell real, cumpliendo con los objetivos de aprendizaje propuestos.

---
