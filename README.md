# tp-2018-2c-nuGetChar
0. Ejecutar `mkdir ~/Documentos` y luego `cd ~/Documentos`
1. Clonar el repo en __~/Documentos__. Para esto, ingresar a https://goo.gl/Pz7m9n (es más corto y rápido escribir esto) y una vez que el browser haga la conversión a la URL real, copiar y pegar la URL del repo para el git clone: `git clone https://github.com/sisoputnfrba/tp-2018-2c-nuGetChar`. El repo debería haberse clonado en __/home/utnso/Documentos/tp-2018-2c-nuGetChar__ que es equivalente a __~/Documentos/tp-2018-2c-nuGetChar__
2. Ejecutar `cd tp-2018-2c-nuGetChar`.
3. Ejecutar `sudo make scratch` para bajar las commons y compilar el proyecto (pass sudo: _utnso_).
4. Ejecutar `vi configs/ips.txt` y setear las IPs de los módulos que allí figuran, nada más (para salir de vi apretar ESC y poner `:wq`).
5. Ejecutar `make ips`.
7. Ejecutar `make prod`.
8. Dependiendo el test, ejecutar lo siguiente para copiar la config a los módulos:
    * `make test_minima`
    * `make test_algoritmos`
    * `make test_filesystem`
    * `make test_completa`
9. Ejecutar el módulo que corresponda en `xxx\Debug`.
    
> solo para el MDJ:   

9. Ejecutar `make fifa_examples` para descargar los examples del File System.
10. Ejecutar `make fs_entrega` para instalar el File System de la _entrega final_ en el MDJ.
