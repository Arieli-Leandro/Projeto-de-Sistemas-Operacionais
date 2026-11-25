/*Objetivo: Implementar, na linguagem C, soluções para dois problemas clássicos de programação concorrente, 
utilizando processos em vez de threads.

Exercício 1 – Problema do Produtor-Consumidor
Desenvolva uma implementação em C para o problema clássico do Produtor-Consumidor, 
utilizando processos para representar os produtores e consumidores.

A implementação deverá contemplar um buffer compartilhado de capacidade limitada, 
no qual os processos produtores inserem elementos e os processos consumidores retiram esses elementos.

A sincronização deverá garantir que:

Um produtor não insira elementos quando o buffer estiver cheio.

Um consumidor não retire elementos quando o buffer estiver vazio.

O acesso ao buffer compartilhado seja realizado de maneira mutuamente exclusiva, 
evitando condições de corrida (race conditions).

Utilize mecanismos de comunicação e sincronização entre processos, como memória compartilhada, 
mutexes e semáforos compartilhados. */

//O que vamos usar?
/* Mutex: Garante que somnete um processo por vez acesse
a região crítica do buffer

Semáforo (empty): Controla quantas posições vazias existem no buffer

Semáforo (full): Controla quantos produtos estão disponíveis para consumo

-> O mutex e os semáforos também precisarão ser compartilhados entre os processos

*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <pthread.h> //pra poder usar o mutex

#define TAM_BUFFER 5
#define NUM_PRODUTORES 2
#define NUM_CONSUMIDORES 2
#define ITENS_POR_PRODUTOR 5

//declarando a struct que vai ser responsável por armazenar o estado do buffer eos mecanismos de sincronização
typedef struct{
    int buffer[TAM_BUFFER]; // "depósito" de produtp
    int in; //onde o próximo vai ser colocado
    int out; // aonde o próximo vai ser retirador
    sem_t buffer_vazio;
    sem_t buffer_cheio;
    pthread_mutex_t mutex; //mecanismo p/ proteger a região crítica
} ArmMemoriaCompartilhada;

//Função responsável por criar o ponteiro para a struct para criar a memória compartilhada
ArmMemoriaCompartilhada * criaMemoriaCompartilhada(){

    ArmMemoriaCompartilhada *mem;

    mem = mmap(NULL, sizeof(ArmMemoriaCompartilhada), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    //verificando se foi criada a memória compartilhada ou se deu erro
    if(mem == MAP_FAILEP){

        printf("Erro ao criar a memória compartilhada! \n");
        exit(1);

    };

    //se nn entrar no if -> retorno a região da memória
    return mem;

}

//criando a função responsável por inicializar os semáforos 
void inicializaSemaforo(ArmMemoriaCompartilhada *mem){

    //inicializando o semáforo de posiçções vazias
    //o tamanho do buffer eu defini como uma constante de 5
    if(sem_init(&mem->buffer_vazio, 1, TAM_BUFFER) == -1){

        printf("Erro ao inicializar os semáforos vazios! \n");
        exit(EXIT_FAILURE);

    }

    //inicializa o semáforo de produtos disponíveis
    if(sem_init(&mem->buffer_cheio, 1, 0) == -1){

        printf("Erro ao inicializar o buffer normal! \n");
        exit(EXIT_FAILURE);

    }

    return;

}

//criando a função que é responsável por inicializar o mutex
void inicializaMutex(ArmMemoriaCompartilhada *mem){

    pthread_mutexattr_t atributos_mutex;

    //inicializando os atributos do mutex
    if(pthread_mutexattr_init(&atributos_mutex) != 0){

        printf("Erro ao inicializar os atributos do mutex! \n");
        exit(EXIT_FAILURE);

    }

    //compartilhando o mutex entre os processos
    if(pthread_mutexattr_setpshared(&atributos_mutex, PTHREAD_PROCESS_SHARED) != 0){

        printf("Erro ao compartilhar o mutex entre os processos! \n");
        exit(EXIT_FAILURE);

    }

    //inicializando o mutex cm os atributos configurads
    if(pthread_mutex_init(&mem->mutex, &atributos_mutex) != 0){
        
        printf("Erro ao inicializar o mutex! \n");
        exit(EXIT_FAILURE);

    }

    //liberando o mutex depois do uso
    pthread_mutexattr_destroy(&atributos_mutex);

    return;

}

int main(){





    return 0;

}