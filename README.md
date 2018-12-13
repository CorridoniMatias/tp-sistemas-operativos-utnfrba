# tp-2018-2c-nuGetChar
1. Clonar el repo en __Documents__.
2. Ejecutar `cd tp-2018-2c-nuGetChar`.
3. Ejecutar `sudo make scratch` para bajar las commons y compilar el proyecto (pass sudo: _utnso_).
4. Ejecutar `vi configs/ips.txt` y setear las IPs de los módulos que allí figuran, nada más (para salir de vi apretar ESC y poner `:wq`).
5. Ejecutar `make ips`.
6. Dependiendo el test ejecutar lo siguiente para copiar la config a los módulos.
    * `make test_minima`
    * `make test_algoritmos`
    * `make test_filesystem`
    * `make test_completa`
7. Ejecutar el módulo que corresponda en `xxx\Debug`.
    
> solo para el MDJ:   

7. Ejecutar `make fifa_examples` para descargar los examples del File System.
8. Ejecutar `make fs_entrega` para instalar el File System de la _entrega final_ en el MDJ.
