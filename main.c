#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/*Defino la estructura del chip*/

struct Chip
{
    uint8_t memoriaRAM[4096]; // 4KB de RAM
    uint8_t V[16]; // Registros de propositos generales
    uint16_t PC ; // Contador de programa
    uint16_t RI; //Registro indice
    uint16_t stack [16]; // La pila
    uint8_t SP ; // Stack pointer
    uint8_t delay ; // Temporizador 1
    uint8_t sound ; // temporizador 2
    uint64_t pantalla [32];

    

};

// esta funcion es para inicializar los registros en 0 y el contador de programa en 512 ya que historicamente se hizo asi.
void inicializar_chip8(struct Chip* c)
{
    memset(c, 0, sizeof(struct Chip));
    c->PC = 512;
}

void emular_ciclo(struct Chip* c)
{
    //FETCH (buscar)
    //Leemos el byte actual y el siguiente, y los unimos en 16 bits
    uint16_t instruccion = (c->memoriaRAM[c->PC] << 8) | c ->memoriaRAM[c->PC + 1];
    //Incrementamos el PC en 2 para que en el proximo ciclo lea la siguiente instruccion
    c->PC += 2;

    //2.DECODE (desarmadero)
    uint8_t X = (instruccion & 0x0F00) >> 8; //El segundo digito
    uint8_t Y = (instruccion & 0x00F0) >> 4; //El tercer digito
    uint8_t N = instruccion & 0x000F; //El cuarto digito
    uint8_t NN = instruccion & 0x00FF; //Los ultimos dos digitos
    uint16_t NNN = instruccion & 0X0FFF; ////Los ultimos tres digitos

    // & EXECUTE (decodificar y ejecutar)
    // ..aca entra la parte de las mascaras bit a bit, en CHIP 8 el primer digito te dice que hacer pero los otros tres digitos te dicen con que hacerlo.
    switch(instruccion & 0xF000)
    {
        case 0xA000:
            //Comando A: Cambiar el registro indice (RI)
            //Necesitamos extraer los ultimos 3 digitos (NNN)
            c->RI = NNN;
            break;

        case 0x1000:
            //Comando 1: Saltar a una direccion (PC = NNN)
            c->PC = NNN;
            break;
        
        case 0x6000:
            //Comando 6XNN: Establece Vx en NN
            c->V[X] = NN;
            break;
        
        case 0x7000:
            //Comando 7XNN: Suma NN a Vx
            c->V[X] =+ NN;
            break;
        
        case 0x2000:
            //Comando 2NNN: Llama a una subrutina.
            //1. Guardar donde estabamos, el stack guarda la direccion actual del PC
            c->stack[c->SP] = c->PC;
            //2. Mover el puntero al siguiente renglon vacio del stack
            c->SP += 1; //Tambien podria usar c->SP++
            //3. JUMP, el procesador viaja a la subrutina
            c->PC = NNN;
            break;

        case 0x0000:
            c->PC = c->stack[c->SP];
            c->PC = NNN;
            break;

        default:
        printf("Instruccion no reconocida: 0x%X\n", instruccion);
        break;
    }
}

int main()
{
    //1. Instanciamos el chip en la memoria
    struct Chip mi_chip;

    //2. Lo inicializamos (le pasamos la direccion con '&')
    inicializar_chip8(&mi_chip);
    
    //3. Hacemos arrancar todo
    while (1)
    {
        emular_ciclo(&mi_chip);
    }
    return 0;
}