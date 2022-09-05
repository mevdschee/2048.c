2048.c
======

[English](README.md)

![screenshot](screenshot.png)

Version de consola del juego "2048" para GNU/Linux

### Cómo se juega

Se pueden mover las teselas en las cuatro direcciones con las teclas de flecha: arriba, abajo, izquierda y derecha. Todos los números en el tablero se deslizarán en esa dirección hasta que choquen con la pared y, si se tocan entre sí, dos números se combinarán en uno si tienen el mismo valor. Cada número solo se combinará una vez por jugada. Cada movimiento aparece un nuevo número 2 o 4. Habrás ganado si consigues un 2048 en el tablero, pero perderás si el tablero está lleno y no puedes hacer más movimientos.

### Requisitos

- Compilador de C

Testeado en: GNU/Linux, FreeBSD, OpenBSD

### Instalación

```
wget https://raw.githubusercontent.com/mevdschee/2048.c/master/2048.c
gcc -o 2048 2048.c
./2048
```

### Ejecutar el juego

El juego soporta diferentes esquemas de colores. Esto depende del soporte ANSI para 88 o 256 colores. Si no hay los suficientes colores soportados, el juego recurrirá al blanco y negro (que sigue siendo apto para jugar). Para el esquema de colores original, ejecute en la consola:

```
./2048
```
Para el esquema de colores en blanco y negro (requiere 256 colores):

```
./2048 blackwhite
```

Para el esquema de colores azul y rojo (requiere 256 colores):

```
./2048 bluered
```

### Contribuciones

Las contribuciones son siempre bienvenidas. Ejecute siempre las pruebas antes de hacer commit usando:

```
$ ./2048 test
All 13 tests executed successfully
```
