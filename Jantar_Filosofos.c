/*Exercício 2 – Problema do Jantar dos Filósofos
Desenvolva uma implementação em C para o problema clássico do Jantar dos Filósofos, utilizando processos para representar os filósofos.

Considere cinco filósofos sentados ao redor de uma mesa, com um garfo entre cada par de filósofos. Cada filósofo alterna entre os estados de pensar e comer, sendo necessário adquirir os dois garfos adjacentes para conseguir comer.

A implementação deverá utilizar mecanismos de sincronização entre processos para controlar o acesso aos garfos, garantindo que:

Dois filósofos não utilizem simultaneamente o mesmo garfo.

A sincronização evite condições de corrida (race conditions).

Seja tratada a possibilidade de ocorrência de deadlock (impasse), evitando que todos os filósofos fiquem bloqueados indefinidamente.

Utilize memória compartilhada, mutexes e semáforos compartilhados para implementar o controle de acesso aos recursos.*/

//-> maior problema: Dois filósofos não podem utilizar o mesmo garfo da esquerda ao mesmo tempo

/*O que basicamente eu preciso fazer?
1) fork() - Criar cinco processos, um p/ cada filósofo
2) mmap() - Criar e compartilhar a memória entre processos
3) Semáforos - Controlar o acesso aos garfos
4) Mutex - Proteger regiões críticas e evitar condições de corrida

-> No máximo quatro filósofos tentem pegar os garfos
-> Pelo menos um filósofo sempre poderá conseguir os dois garfos

*/


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <pthread.h>

//Declarando as constantes
#define NUM_FILOSOFOS 5
#define TEMPO_PENSANDO 2
#define TEMPO_COMENDO
#define REFEICOES 3

//criando a struct de memória compartilhada
typedef struct{
    sem_t garfos[NUM_FILOSOFOS]; //representa cada garfo disponível
    sem_t limite;
    pthread_mutex_t mutex;
} MemCompartilhada;

//Criando a funções responsável por criar a memória compartilhada
MemCompartilhada *criaMemoriaCompartilhada(){

    MemCompartilhada *mem;

    mem = mmap(NULL, sizeof(MemCompartilhada), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    if(mem == MAP_FAILED){

        printf("Erro ao criar a memória compartilhada! \n");
        exit(EXIT_FAILURE);

    }

    return mem;

}


int main(){




    return 0;

}