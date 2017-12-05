#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_native_dialog.h>
#include "Leitor.h"
#define FPS 180.0
#define LARGURA_TELA 640 +15
#define ALTURA_TELA 320 +15
//definindo biblioteca grafica
ALLEGRO_DISPLAY *janela = NULL;
ALLEGRO_EVENT_QUEUE *fila_eventos = NULL;
ALLEGRO_TIMER *timer = NULL;
ALLEGRO_BITMAP *quadrado = NULL;
int sair = 0;

//definindo chip fontset
unsigned char chip8_fontset[80] =
{
  0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
  0x20, 0x60, 0x20, 0x20, 0x70, // 1
  0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
  0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
  0x90, 0x90, 0xF0, 0x10, 0x10, // 4
  0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
  0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
  0xF0, 0x10, 0x20, 0x40, 0x40, // 7
  0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
  0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
  0xF0, 0x90, 0xF0, 0x90, 0x90, // A
  0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
  0xF0, 0x80, 0x80, 0x80, 0xF0, // C
  0xE0, 0x90, 0x90, 0x90, 0xE0, // D
  0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
  0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

void inicializar(CHIP8 *chip)
{
    chip->opcode = 0;
    chip->delay_timer =0;
    chip->I=0;
    chip->sp=0;
    chip->sound_timer=0;
    chip->pc &= 0;
    chip->drawflag = 0;
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
    unsigned short * keys;
    int y, x, i =0;
    unsigned short vx, vy =0;
    unsigned short height, pixel=0;

  // Fetch opcode
  chip->opcode = chip->memory[chip->pc] << 8 | chip->memory[chip->pc + 1];

  // Decode opcode
  switch(chip->opcode & 0xF000)
  {

    case 0x0000:
        switch(chip->opcode & 0x000F)
        {
            case 0x0000: //limpa a tela
                memset(chip->graphics, 0, sizeof(chip->graphics));
                chip->pc += 2;
                chip->drawflag =1;
            break;

            case 0x000E: //retorna de uma subrotina
                chip->sp = chip->sp - 1;
                chip->pc = chip->stack[chip->sp];
                chip->pc += 2;
            break;

            default:
                printf ("Opcode desconhecido [0x0000]: 0x%X\n", chip->opcode);
        }
    break;

    case 0x1000: //Jump para o endere�o NNN
            chip->pc = chip->opcode & 0x0FFF;
    break;

    case 0x2000: //chama uma subrotina em NNN
            chip->stack[chip->sp] = chip->pc;
            ++chip->sp;
            chip->pc = chip->opcode & 0x0FFF;
    break;

    case 0x3000: //se Vx for igual a NN pular a proxima instru�ao
            if(chip->V[(chip->opcode & 0x0F00) >> 8] == (chip->opcode & 0x00FF))
                chip->pc += 4;
            else
                chip->pc += 2;
    break;

    case 0x4000: // se Vx for diferente de NN pular a proxima instru�ao
            if(chip->V[(chip->opcode & 0x0F00) >> 8] != (chip->opcode & 0x00FF))
                chip->pc += 4;
            else
                chip->pc += 2;
    break;

    case 0x5000: //se Vx for igual a Vy pular a proxima instrucao
            if(chip->V[(chip->opcode & 0x0F00) >> 8 ] == chip->V[(chip->opcode & 0x00F0) >> 4])
                chip->pc += 4;
            else
                chip->pc += 2;
    break;

    case 0x6000: //seta Vx para NN
            chip->V[(chip->opcode & 0x0F00) >> 8] = (chip->opcode & 0x00FF);
            chip->pc +=2;
    break;

    case 0x7000: //adiciona NN em Vx
            chip->V[(chip->opcode & 0x0F00) >> 8] += (chip->opcode & 0x00FF);
            chip->pc += 2;
    break;

    case 0x8000:
            switch(chip->opcode & 0x000F)
            {
                case 0x0000: //seta Vx para Vy
                    chip->V[(chip->opcode & 0x0F00) >> 8] = chip->V[(chip->opcode & 0x00F0) >> 4];
                    chip->pc += 2;
                break;

                case 0x0001:  //seta Vx para Vx or com Vy
                    chip->V[(chip->opcode & 0x0F00) >> 8] = chip->V[(chip->opcode & 0x0F00) >> 8] | chip->V[(chip->opcode & 0x00F0) >> 4];
                    chip->pc += 2;
                break;

                case 0x0002: //seta Vx para Vx and com Vy
                    chip->V[(chip->opcode & 0x0F00) >> 8] = chip->V[(chip->opcode & 0x0F00) >> 8] & chip->V[(chip->opcode & 0x00F0) >> 4];
                    chip->pc += 2;
                break;

                case 0x0003: //seta Vx para Vx xor com Vy
                    chip->V[(chip->opcode & 0x0F00) >> 8] = chip->V[(chip->opcode & 0x0F00) >> 8] ^ chip->V[(chip->opcode & 0x00F0) >> 4];
                    chip->pc += 2;
                break;

                case 0x0004: //adiciona Vy em Vx. VF � setado em 1 quando h� um carry, e para 0 se n�o houver
                    if(chip->V[(chip->opcode & 0x00F0) >> 4] > (chip->V[(chip->opcode & 0x0F00) >> 8]))
                        chip->V[0xF] = 1;
                    else
                        chip->V[0xF] = 0;

                    chip->V[(chip->opcode & 0x0F00) >> 8] = chip->V[(chip->opcode & 0x0F00) >> 8] + chip->V[(chip->opcode & 0x00F0) >> 4];

                    chip->pc += 2;
                break;

                case 0x0005: //Subtrai Vy de Vx. VF � setado em 0 se houver um "borrow" ou em 1 se n�o houver
                    if(chip->V[(chip->opcode & 0x00F0) >> 4] > chip->V[(chip->opcode & 0x0F00) >> 8])
                        chip->V[0xF] = 0;
                    else
                        chip->V[0xF] = 1;

                    chip->V[(chip->opcode & 0x0F00) >> 8] -= chip->V[(chip->opcode & 0x00F0) >> 4];

                    chip->pc += 2;
                break;

                case 0x0006: //Shifts Vx para direita em 1. Vf � shiftado para direita antes de Vx antes da shifitada
                    chip->V[0xF] = chip->V[(chip->opcode & 0x0F00) >> 8] & 0x01;
                    chip->V[(chip->opcode & 0x0F00) >> 8] = chip->V[(chip->opcode & 0x0F00) >> 8] >> 1;
                    chip->pc +=2;
                break;

                case 0x0007: //Seta Vx para Vy - Vx. VF � setado para 0 quando exist um "borrow" ou para 1 caso exista
                    if(chip->V[(chip->opcode & 0x0F00) >> 8] > chip->V[(chip->opcode & 0x00F0) >> 4])
                        chip->V[0xF] = 0;
                    else
                        chip->V[0xF] = 1;

                    chip->V[(chip->opcode & 0x0F00) >> 8] = chip->V[(chip->opcode & 0x00F0) >> 4] - chip->V[(chip->opcode & 0x0F00) >> 8];
                    chip->pc += 2;
                break;

                case 0x000E: //Shifts Vx 1 para esquerda. VF � setado com o bit mais significativo de Vx antes do shift
                    chip->V[0xF] = chip->V[(chip->opcode & 0x0F00) >> 8] >> 7;
                    chip->V[(chip->opcode & 0x0F00) >> 8] = chip->V[(chip->opcode & 0x0F00) >> 8] << 1;
                    chip->pc += 2;
                break;

                default: printf("Opcode desconhecido: 0x%x\n",chip->opcode);

            }
        break;

    case 0x9000: // se Vx n�o for igual a Vy, pula a pr�xima instru��o
        if(chip->V[(chip->opcode & 0x0F00) >> 8] != chip->V[(chip->opcode & 0x00F0) >> 4])
            chip->pc +=4;
        else
            chip->pc +=2;
    break;

    case 0xA000: // Seta I para NNN

      chip->I = chip->opcode & 0x0FFF;
      chip->pc += 2;
    break;

    case 0xB000: // Jump para o endere�o NNN mais V0
        chip->pc = chip->V[0] + (chip->opcode & 0x0FFF);
    break;

    case 0xC000: //Seta Vx sem um numero aleat�rio and com NN
        chip->V[(chip->opcode & 0x0F00) >> 8] = (rand() % 0xFF) & (chip->opcode & 0x00FF);
        chip->pc += 2;
    break;

    case 0xD000:
        vx = chip->V[(chip->opcode & 0x0F00) >> 8];
        vy = chip->V[(chip->opcode & 0x00F0) >> 4];
        height = chip->opcode & 0x000F;
        chip->V[0xF] = 0;

        for(y = 0; y < height; y++){
            pixel = chip->memory[chip->I + y];
            for(int x = 0; x < 8; x++){
                if((pixel & (0x80 >> x))!= 0){
                    if(chip->graphics[(vx+x+((vy+y)*64))] == 1)
                    {
                            chip->V[0xF] = 1;
                    }
                    chip->graphics[vx+x+((vy+y)*64)] ^= 1;
                }
            }
        }
        chip->drawflag = true;
        chip->pc += 2;
    break;

    case 0xE000: //keys***
        switch(chip->opcode & 0x00FF)
            {
                case 0x009E:
                    if(chip->key[chip->V[(chip->opcode & 0x0F00) >> 8]] != 0)
                            chip->pc += 4;
                        else
                            chip->pc += 2;

                break;

                case 0x00A1:
                    if(chip->key[chip->V[(chip->opcode & 0x0F00) >> 8]] == 0)
                            chip->pc += 4;
                        else
                            chip->pc += 2;

                break;

                default:
                    printf ("Opcode desconhecido: 0x%X\n", chip->opcode);
}
    break;

    case 0xF000:
        switch(chip->opcode & 0x00FF)
        {
            case 0x0007:
                chip->V[(chip->opcode & 0x0F00) >> 8] = chip->delay_timer;
                chip->pc += 2;
            break;

            case 0x000A://keys***
            {
                bool teclaPressionada = 0;
                for(i = 0; i < 16; i++)
                            if(chip->key[i] !=0){
                                chip->V[(chip->opcode & 0x0F00) >> 8] = i;
                                teclaPressionada = true;
                            }
                if(chip->key[i] != 1)
                    return;

                chip->pc += 2;
            }
            break;

            case 0x0015: //setar delay timer em Vx
                chip->delay_timer = chip->V[(chip->opcode & 0x0F00) >> 8];
                chip->pc += 2;
            break;

            case 0x0018: //setar  sound timer em Vx
                chip->sound_timer = chip->V[(chip->opcode & 0x0F00) >> 8];
                chip->pc += 2;
            break;

            case 0x001E: //setar Vx em I
                if(chip->I + chip->V[(chip->opcode & 0x0F00) >> 8] > 0xFFF)
                    chip->V[0xF] = 1;
                else
                    chip->V[0xF] = 0;

                chip->I = chip->I + chip->V[(chip->opcode & 0x0F00) >> 8];
                chip->pc += 2;
            break;

            case 0x0029: //setar I na localiza��o do sprite no caractere em Vx
                chip->I = chip->V[(chip->opcode & 0x0F00) >> 8] * 0x5;
                chip->pc += 2;
            break;

            case 0x0033: //Salva o a representa�ao decimal codificada em binario de Vx, com os 3 digitos mais significativos do endere�o em I
                chip->memory[chip->I] = chip->V[(chip->opcode & 0x0F00) >> 8] / 100;
                chip->memory[chip->I+1] = (chip->V[(chip->opcode & 0x0F00) >> 8] / 10) % 10;
                chip->memory[chip->I+2] = (chip->V[(chip->opcode & 0x0F00) >> 8] % 100) % 10;
                chip->pc += 2;
            break;

            case 0x0055: //Salva V0 em Vx na no endere�o de memoria de I
                for(int i = 0; i <= chip->V[(chip->opcode & 0x0F00) >> 8]; i++)
                    chip->memory[chip->I + i] = chip->V[i];

                chip->pc += 2;
            break;

            case 0x0065: //Preenche V0 com os valores de Vx da memoria come�anco em I
                for(i = 0; i <= ((chip->opcode & 0x0F00) >> 8); i++)
                    chip->V[i] = chip->memory[chip->I + i];
                chip->pc += 2;
            break;

            default: printf("Opcode desconhecido: %X\n", chip->opcode);
        }
    break;

    default:
      printf ("Opcode desconhecido: 0x%X\n", chip->opcode);
  }


  // atualizar timers

  if(chip->delay_timer > 0)
    --chip->delay_timer;

  if(chip->sound_timer > 0)
  {
    if(chip->sound_timer == 1)
      printf("BEEP!\n");
    --chip->sound_timer;
  }

}
/*int desenhar(CHIP8 * chip)
{
        ALLEGRO_EVENT evento;
        al_wait_for_event(fila_eventos, &evento);

        if(evento.type == ALLEGRO_EVENT_TIMER){
            chip->drawflag=1;
        }
        else if(evento.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
            sair = 1;
        }

        if(chip->drawflag && al_is_event_queue_empty(fila_eventos)) {

            al_clear_to_color(al_map_rgb(0,0,0));
            printf("desenhando\n");
            int eixox=0,eixoy=0;

            for(int l=0; l < 2048;l++){
                unsigned char test = chip->graphics[l];

                if(eixox==64){
                    eixox=0;
                    eixoy+=1;
                }
                if(test == 1){
                    al_set_target_bitmap(al_get_backbuffer(janela));
                    al_draw_bitmap(quadrado,eixox*10, eixoy*10, 0);
                    al_set_target_bitmap(quadrado);
                    al_clear_to_color(al_map_rgb(255, 255, 255));

                }
                else{
                    al_set_target_bitmap(al_get_backbuffer(janela));
                    al_draw_bitmap(quadrado,eixox*10, eixoy*10, 0);
                    al_set_target_bitmap(quadrado);
                    al_clear_to_color(al_map_rgb(0, 0, 0));
                }
                eixox+=1;
            }

        al_flip_display();

        chip->drawflag = 0;
        }
        if(sair==1)
            return 1;
}
*/
void error_msg(char *text){
	al_show_native_message_box(janela,"ERRO",
		"Ocorreu o seguinte erro e o programa sera finalizado:",
		text,NULL,ALLEGRO_MESSAGEBOX_ERROR);
}
int abrir_tela()
{
    if (!al_init()){
        error_msg("Falha ao inicializar a Allegro");
        return 0;
    }

    //cria o timer com o intervalo de tempo que ele ira disparar
    timer = al_create_timer(1.0 / FPS);
    if(!timer) {
        error_msg("Falha ao criar temporizador");
        return 0;
    }

    janela = al_create_display(LARGURA_TELA, ALTURA_TELA);
    if(!janela) {
        error_msg("Falha ao criar janela");
        al_destroy_timer(timer);
        return 0;
    }

    al_set_window_title(janela, "CHIP8");

    quadrado = al_create_bitmap(10, 10);
    if (!quadrado){
        error_msg("Falha ao criar bitmap");
        al_destroy_timer(timer);
        al_destroy_display(janela);
        return 0;
    }
    if (!al_install_keyboard()){
        error_msg("Falha ao inicializar o teclado");
        return 0;
    }

    /*al_set_target_bitmap(quadrado);
    al_clear_to_color(al_map_rgb(255, 0, 0));
    al_set_target_bitmap(al_get_backbuffer(janela));*/


    fila_eventos = al_create_event_queue();
    if(!fila_eventos) {
        error_msg("Falha ao criar fila de eventos");
        al_destroy_timer(timer);
        al_destroy_display(janela);
        al_destroy_bitmap(quadrado);
        return 0;
    }

    al_register_event_source(fila_eventos, al_get_display_event_source(janela));
    al_register_event_source(fila_eventos, al_get_timer_event_source(timer));
    al_register_event_source(fila_eventos, al_get_keyboard_event_source());
    al_clear_to_color(al_map_rgb(0,0,0));
    al_flip_display();
    al_start_timer(timer);

    return 1;
}
int tratar_teclas(CHIP8 * chip)
{

       ALLEGRO_EVENT evento;
        //espera ate que algum evento esteja na fila

        al_wait_for_event(fila_eventos, &evento);

        if(evento.type == ALLEGRO_EVENT_TIMER)
            chip->drawflag = true;

        else if (evento.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
            return 1;
        //se o evento for pressionar uma tecla
        else if (evento.type == ALLEGRO_EVENT_KEY_DOWN)
        {
            //verifica qual tecla foi pressionada
            switch(evento.keyboard.keycode)
            {
                //1
                case ALLEGRO_KEY_1:
                    chip->key[0x1] = 1;
                    break;
                //2
                case ALLEGRO_KEY_2:
                    chip->key[0x2] = 1;
                    break;
                //3
                case ALLEGRO_KEY_3:
                    chip->key[0x3] = 1;
                    break;
                //C
                case ALLEGRO_KEY_4:
                    chip->key[0xC] = 1;
                    break;
                //4
                case ALLEGRO_KEY_Q:
                    chip->key[0x4] = 1; printf("\n tecla Q");
                    break;
                //5
                case ALLEGRO_KEY_W:
                    chip->key[0x5] = 1; printf("\n tecla W");
                    break;
                //6
                case ALLEGRO_KEY_E:
                    chip->key[0x6] = 1; printf("\n tecla E");
                    break;
                //D
                case ALLEGRO_KEY_R:
                    chip->key[0xD] = 1;
                    break;
                //7
                case ALLEGRO_KEY_A:
                    chip->key[0x7] = 1;
                    break;
                //8
                case ALLEGRO_KEY_S:
                    chip->key[0x8] = 1;
                    break;
                //9
                case ALLEGRO_KEY_D:
                    chip->key[0x9] = 1;
                    break;
                //E
                case ALLEGRO_KEY_F:
                    chip->key[0xE] = 1;
                    break;
                //A
                case ALLEGRO_KEY_Y:
                    chip->key[0xA] = 1;
                    break;
                //0
                case ALLEGRO_KEY_X:
                    chip->key[0x0] = 1;
                    break;
                //B
                case ALLEGRO_KEY_C:
                    chip->key[0xB] = 1;
                    break;
                //F
                case ALLEGRO_KEY_V:
                    chip->key[0xF] = 1;
                    break;

                case ALLEGRO_KEY_ESCAPE:
                    return 1;
                    break;

            }

        }
        else if (evento.type == ALLEGRO_EVENT_KEY_UP)
        {
            //verifica qual tecla foi solta
            switch(evento.keyboard.keycode)
            {
                //1
                case ALLEGRO_KEY_1:
                    chip->key[0x1] = 0; printf("\n tecla QQQQQQ");
                    break;
                //2
                case ALLEGRO_KEY_2:
                    chip->key[0x2] = 0;
                    break;
                //3
                case ALLEGRO_KEY_3:
                    chip->key[0x3] = 0;
                    break;
                //C
                case ALLEGRO_KEY_4:
                    chip->key[0xC] = 0;
                    break;
                //4
                case ALLEGRO_KEY_Q:
                    chip->key[0x4] = 0;
                    break;
                //5
                case ALLEGRO_KEY_W:
                    chip->key[0x5] = 0;
                    break;
                //6
                case ALLEGRO_KEY_E:
                    chip->key[0x6] = 0;
                    break;
                //D
                case ALLEGRO_KEY_R:
                    chip->key[0xD] = 0;
                    break;
                //7
                case ALLEGRO_KEY_A:
                    chip->key[0x7] = 0;
                    break;
                //8
                case ALLEGRO_KEY_S:
                    chip->key[0x8] = 0;
                    break;
                //9
                case ALLEGRO_KEY_D:
                    chip->key[0x9] = 0;
                    break;
                //E
                case ALLEGRO_KEY_F:
                    chip->key[0xE] = 0;
                    break;
                //A
                case ALLEGRO_KEY_Y:
                    chip->key[0xA] = 0;
                    break;
                //0
                case ALLEGRO_KEY_X:
                    chip->key[0x0] = 0;
                    break;
                //B
                case ALLEGRO_KEY_C:
                    chip->key[0xB] = 0;
                    break;
                //F
                case ALLEGRO_KEY_V:
                    chip->key[0xF] = 0;
                    break;
            }
        }
            if((chip->drawflag == true) && (al_is_event_queue_empty(fila_eventos))) {
                chip->drawflag = false;

            al_clear_to_color(al_map_rgb(0,0,0));
            //printf("desenhando\n");
            int eixox=0,eixoy=0;

            for(int l=0; l < 2048;l++){
                unsigned char test = chip->graphics[l];

                if(eixox==64){
                    eixox=0;
                    eixoy+=1;
                }
                if(test == 1){
                    al_set_target_bitmap(al_get_backbuffer(janela));
                    al_draw_bitmap(quadrado,eixox*10, eixoy*10, 0);
                    al_set_target_bitmap(quadrado);
                    al_clear_to_color(al_map_rgb(255, 255, 255));

                }
                else{
                    al_set_target_bitmap(al_get_backbuffer(janela));
                    al_draw_bitmap(quadrado,eixox*10, eixoy*10, 0);
                    al_set_target_bitmap(quadrado);
                    al_clear_to_color(al_map_rgb(0, 0, 0));
                }
                eixox+=1;
            }

        al_flip_display();
        }

        return 0;
    }

