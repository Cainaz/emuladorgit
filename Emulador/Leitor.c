#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include "Leitor.h"

void inicializar(CHIP8 *chip)
{
    chip->opcode = 0;
    chip->delay_timer =0;
    chip->I=0;
    chip->sp=0;
    chip->sound_timer=0;
    chip->pc &= 0;
}

void carregarArquivo(const char *narq, CHIP8 *chip){

    FILE *fp;
    fp = fopen(narq, "rb");
    int resultado =0;

    if (fp == NULL)
    {
      puts ("Erro ao ler 0 arquivo \n");
      fclose (fp);
    }
    //carregando a leitura do jogo para a memoria na posi��o 0x200
    resultado = fread(chip->memory+0x200, 1,memsize-0x200,fp);

    printf("Numero de elementos lidos: %d\n", resultado);

    for(int i=0; i<memsize;i++)
    {
        printf("variavel em [%d]: %x\n",i,chip->memory[i]);
    }
    //p;
    fclose(fp);

}

void emular(CHIP8 *chip){

  // Fetch opcode
  chip->opcode = chip->memory[chip->pc] << 8 | chip->memory[chip->pc + 1];

  // Decode opcode
  switch(chip->opcode & 0xF000)
  {

    case 0x0000:
        switch(chip->opcode & 0x000F)
        {
            case 0x0000: // 0x00E0: Clears the screen
            // Execute opcode
            break;

            case 0x000E: // 0x00EE: Returns from subroutine
                chip->pc = chip->stack[(--chip->sp)&0xF] + 2;
            // Execute opcode
            break;

            default:
                printf ("Opcode desconhecido [0x0000]: 0x%X\n", chip->opcode);
        }
        break;

    case 0x1000:
        break;

    case 0x2000:
            chip->stack[chip->sp] = chip->pc;
            ++chip->sp;
            chip->pc = chip->opcode & 0x0FFF;
        break;

    case 0x3000:
        break;

    case 0x4000:
        break;

    case 0x5000:
        break;

    case 0x6000:
        break;

    case 0x7000:
        break;

    case 0x8000:
        break;

    case 0x9000:
        break;

    case 0xA000: // ANNN: Sets I to the address NNN

      chip->I = chip->opcode & 0x0FFF;
      chip->pc += 2;
    break;

    case 0xB000:
        break;

    case 0xC000:
        break;

    case 0xD000:
        break;

    case 0xE000:
        break;

    case 0xF000:
        break;

    default:
      printf ("Unknown opcode: 0x%X\n", chip->opcode);
  }


  // Update timers

  if(chip->delay_timer > 0)
    --chip->delay_timer;

  if(chip->sound_timer > 0)
  {
    if(chip->sound_timer == 1)
      printf("BEEP!\n");
    --chip->sound_timer;
  }

}




