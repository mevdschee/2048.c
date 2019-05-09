2048.c
======

![screenshot](screenshot.png)

Version de consola del juego "2048" pra GNU/Linux

### Cómo se juega

Puede mover los mosaicos en cuatro direcciones con las teclas de flecha: arriba, abajo, izquierda y derecha. Todos los números en el tablero se deslizarán en esa dirección hasta que golpeen la pared y si se tocan entre sí, dos números se combinarán en uno si tienen el mismo valor. Cada número solo se combinará una vez por jugada. Cada movimiento aparece un nuevo número 2 o 4. Si tienes un 2048 en el tablero, has ganado, pero pierdes una vez que el tablero está lleno y no puedes hacer un movimiento. 

### Requerimientos

- Compilador de C

Testeado en: GNU/Linux, FreeBSD, OpenBSD

### Instalación

```
wget https://raw.githubusercontent.com/mevdschee/2048.c/master/2048.c
gcc -o 2048 2048.c
./2048
```

### Correr el juego

El juego soporta diferentes esquemas de colores. Esto depende del soporte ANSI para 88 o 256 colores. Si no hay los suficientes colores soportados, el juego irá del blanco al negro (mucho más apto para jugar). Para el esquema de colores original, pone en consola:
```
./2048
```
Para el esquema de colores de blanco a negro (requiere 256 colores):

```
./2048 blackwhite
```

Para el esquema de colores de azul a rojo (requiere 256 colores):

```
./2048 bluered
```

### Contribuciones

Las contribuciones son siempre bienvenidas. Siempre correr los tests antes de commitear usando:

```
$ ./2048 test
All 13 tests executed successfully
```
