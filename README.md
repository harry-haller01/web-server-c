# Servidor de directorios en C

Este proyecto es un servidor de directorios simple escrito en C que utiliza HTML y CSS para visualizar el contenido de un directorio especificado. El servidor permite navegar por los subdirectorios del directorio raíz y descargar archivos. Acepta hasta 10 clientes conectados al mismo tiempo.

## Características

- Visualización de directorios y archivos en un navegador web utilizando HTML y CSS
- Navegación por subdirectorios del directorio raíz
- Descarga de archivos
- Soporte para hasta 10 clientes conectados simultáneamente
- Configuración de puerto personalizado

## Requisitos

- Compilador de C (por ejemplo, GCC)
- Sistema operativo compatible con POSIX (Linux, macOS, etc.)

## Compilación

Para compilar el proyecto, ejecute el siguiente comando en la terminal:

```
gcc -o server server.c -lpthread
```

Esto generará un archivo ejecutable llamado `server`.

## Uso

Para ejecutar el servidor, use el siguiente comando en la terminal:

```
./server <puerto> <directorio_raiz>
```

Reemplace `<directorio_raiz>` con la ruta del directorio desea servir y `<puerto>` con el número de puerto en el que desea que el servidor escuche.

Por ejemplo:

```
./server 8800 /home/usuario/directorios
```

Una vez que el servidor esté en ejecución, abra un navegador web y navegue a `http://localhost:<puerto>` para ver el contenido del directorio raíz.

## Licencia

Este proyecto está licenciado bajo la Licencia MIT. Consulte el archivo `LICENSE` para obtener más información.