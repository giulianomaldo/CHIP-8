#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>

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
            c->V[X] += NN;
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
        //Como hay 0x0000 es una familia de isntrucciones entonces tengo que anidar un switch para diferenciarlas
            switch(instruccion & 0x0FF)
            {
                case 0x00E0:
                    //Comando 0x00E0: Limpiar Pantalla
                    memset(c->pantalla, 0, sizeof(c->pantalla));
                    break;
                
                case 0x00EE:
                    //Comando 00EE: Retornar
                    c->SP -= 1; //Retrocedemos al renglon anterior
                    c->PC = c->stack[c->SP]; //Recuperamos la direccion de retorno
                    break;
                
                default:
                    printf("Instruccion 0x0000 no reconocida: 0x%X\n", instruccion);
                    break;
            }
            break; //Cierre del case principal
    

        default:
        printf("Instruccion no reconocida: 0x%X\n", instruccion);
        break;
    }
}

int cargar_rom(struct Chip* c, const char* nombre_archivo) {
    //abrimos el archivo en binario
    FILE* archivo = fopen("roms/pong.ch8", "rb");
    if (archivo == NULL) {
        printf("Error: No se pudo abrir el arhcivo %s\n", nombre_archivo);
        return 0;
    }

    //medimos el arhcivo
    fseek(archivo, 0, SEEK_END); //vamos al final del archivo
    long tamano_archivo = ftell(archivo); // preguntamos en que byte estamos 
    rewinf(archivo); //volvemos el cursor al principio para empezar a leer

    // control de calidad
    if (tamano_archivo > (4096 - 512)) {
        printf("Error: el ROM es demasiado grande para la memoria.");
        fclose(archivo);
        return 0;
    }
    // leer el archivo y volcarlo en la RAM con la funcion fread
    fread(c->memoriaRAM + 512, sizeof(uint8_t), tamano_archivo, archivo);

    // cerramos el archivo y salimos 
    fclose(archivo);
    return 1;
}


int main() {
    // 1. Instanciamos el chip
    struct Chip mi_chip;

    // 2. Lo inicializamos
    inicializar_chip8(&mi_chip);
    
    // 3. Preparamos el reloj usando <time.h>
    // Convertimos los "ticks" del sistema a milisegundos reales
    double ultimo_tiempo = ((double)clock() / CLOCKS_PER_SEC) * 1000.0;
    
    // 4. El Motor Principal (Game Loop)
    while (1) {
        // A. Ejecutar un ciclo de CPU
        emular_ciclo(&mi_chip);
        
        // B. Mirar el reloj actual
        double tiempo_actual = ((double)clock() / CLOCKS_PER_SEC) * 1000.0;
        double tiempo_pasado = tiempo_actual - ultimo_tiempo;
        
        // C. Si pasaron 16.66ms (60Hz), actualizar temporizadores
        if (tiempo_pasado >= 16.66) {
            if (mi_chip.delay > 0) mi_chip.delay--;
            if (mi_chip.sound > 0) mi_chip.sound--;
            
            ultimo_tiempo = tiempo_actual; 
        }
    }
    return 0;
}