# php_unscrew
A program that decrypts files encrypted with php_screw

# Compilation
`g++ -lz -o unscrew unscrew.cpp`

# Usage
```
./unscrew [ENCRYPTED_FILE] [HEADER_LENGTH] [KEY_BYTES]
Exmaple: ./unscrew encrypted.php 8 3432c803c00001053e000d01
```

# Obtaining the Key
As seen in my_screw.h (source of php_screw), the key is an array of short, and when compiled into a shared object, becomes a part of the `data` segment. The values can be obtained using objdump or any other disassembler.

```
objdump -s -j .data php_screw.so

Contents of section .data:
 2540 3432c803 c0000105 3e000d01 00000000  42......>.......
 2550 00000000 00000000 00000000 00000000  ................
 2560 4c000000 eaf33101 00000000 00000000  L.....1.........
 2570 00000000 80130000 00000000 00000000  ................
 2580 00000000 00000000 00000000 00000000  ................
 2590 8a130000 00000000 00000000 00000000  ................
 25a0 00000000 00000000 00000000           ............    
 ```
 
 Store the hex values in pairs of 2 (C++ short) from the start of the dump till the first null byte. It would result in this `3432c803c00001053e000d01` hexstring, this would be our crypt key.

# Obtaining the Header Length

php_screw also adds a header to the start of the files, we need to determine the length of the header before we can decrypt the data. This can be done by inspecting the encrypted file or by seeing the `rodata` section of the shared object.

```
objdump -s -j .rodata php_screw.so

Contents of section .rodata:
 1340 656e6162 6c656400 7068705f 73637265  enabled.php_scre
 1350 77207375 70706f72 74007368 6f775f73  w support.show_s
 1360 6f757263 65006869 67686c69 6768745f  ource.highlight_
 1370 66696c65 00720009 54455354 31320900  file.r..TEST12..
 1380 7068705f 73637265 7700312e 352e3000  php_screw.1.5.0.
 1390 312e322e 3300                        1.2.3.          
```
We can see that between `007200` and `007068` lies our header (`\tTEST12\t`) which is 8 characters long.  
