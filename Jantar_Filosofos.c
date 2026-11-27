#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <pthread.h>
#include <time.h>

//Declarando as constantes
#define NUM_FILOSOFOS 5
#define TEMPO_PENSANDO 2
#define TEMPO_COMENDO 2
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

//criando a função responsável por inicializar os semáforos
void inicializaSemaforo(MemCompartilhada * mem){

    //inicializando os 5 garfos
    for(int i = 0; i < NUM_FILOSOFOS; i++){

        if(sem_init(&mem->garfos[i], 1, 1) == -1){

            printf("Erro ao inicializar o garfo! \n");
            exit(EXIT_FAILURE);

        }

    }

    //inicializa o limite de qunatos filósofos podem tentar pegar os garfos
    if(sem_init(&mem->limite, 1, 4) == -1){

        printf("Erro ao inicializar o limite! \n");
        exit(EXIT_FAILURE);

    }

}

//criando a função responsável por inicializar o mutex
void inicializaMutex(MemCompartilhada * mem){

    pthread_mutexattr_t atributo_mutex;

    //inicializando os atributos do mutex
    if(pthread_mutexattr_init(&atributo_mutex) != 0){

        printf("Erro ao inicializar os atributos do mutex \n");
        exit(EXIT_FAILURE);

    }

    //compartilhando o mutex
    if(pthread_mutexattr_setpshared(&atributo_mutex, PTHREAD_PROCESS_SHARED) != 0){

        printf("Erro ao compartilhar o mutex! \n");
        exit(EXIT_FAILURE);

    }

    //inicializando o mutex
    if(pthread_mutex_init(&mem->mutex, &atributo_mutex) != 0){

        printf("Erro ao inicializar o mutex! \n");
        exit(EXIT_FAILURE);

    }

    //liberando o mutex e os atributos depois de usar
    pthread_mutexattr_destroy(&atributo_mutex);

    return;

}



//-> Configuração dos garfos
// garfo esquerdo -> recebe o msm id que o filósofo
//garfo direito -> recebe um id somando com 1, e precisa voltar ao mesmo id operando em buffer circular


//criando a função q faz o filósofo pensar
void pensar(int id, MemCompartilhada *mem){
    
    //vetor de coisas que o filósofo pode estar pensando
    char *pensamento_fisolofo[] = {"Matemática", "Ciências", "Férias", "Natureza", "Existencialismo", "Arte", "Monster Gelado"};

    int quantidade = sizeof(pensamento_fisolofo) / sizeof(pensamento_fisolofo[0]);


    //protegendo a região de memória para que as mensagens não fiquem se misturando

    pthread_mutex_lock(&mem->mutex);

    printf("O filósofo %d  está pensando em... '%s' \n", id, pensamento_fisolofo[rand() % quantidade]);
    sleep(TEMPO_PENSANDO);

    pthread_mutex_unlock(&mem->mutex);

    return;

}

//criandoa função de comer
void comer(int id, MemCompartilhada *mem){

    int esquerdo = id;
    int direito = (id + 1) % NUM_FILOSOFOS;

    //aguarda um filósofo liberar um garfo para poder comer
    sem_wait(&mem->limite);

    //filosofo pegando o garfo esquerdo
    sem_wait(&mem->garfos[esquerdo]);

    //filosofo pegando o garfo direito
    sem_wait(&mem->garfos[direito]);

    //bloqueando a região de memória
    pthread_mutex_lock(&mem->mutex);

    printf("O Filósofo %d está comendo... \n", id);

    //desbloqueando a região de memória
    pthread_mutex_unlock(&mem->mutex);

    sleep(TEMPO_COMENDO);

    //liberando os garfos ocupados
    sem_post(&mem->garfos[esquerdo]);
    sem_post(&mem->garfos[direito]);

    //liberando para o próximo filósofo poder comer
    sem_post(&mem->limite);

    pthread_mutex_lock(&mem->mutex);

    printf("O Filósofo %d terminou de comer! \n", id);

    pthread_mutex_unlock(&mem->mutex);

    return;

}

//criando a função responsável por "criar" o filósofo
void filosofo(int id, MemCompartilhada *mem){

    for(int i = 0; i < REFEICOES; i++){

        pensar(id, mem);
        comer(id, mem);

    }

    pthread_mutex_lock(&mem->mutex);

    printf("O Filósofo %d terminou todas as refeições! \n", id);

    pthread_mutex_unlock(&mem->mutex);

    return;

}



int main(){

    srand(time(NULL));

    MemCompartilhada *mem = criaMemoriaCompartilhada();

    //inicializando as funções do mutex e do semáforo
    inicializaSemaforo(mem);
    inicializaMutex(mem);

    //criando os 5 filósofos
    for(int i = 0; i < NUM_FILOSOFOS; i++){

        pid_t pid = fork();

        if(pid < 0){

            //processo que deu erro
            printf("Erro ao criar o processo! \n");
            exit(EXIT_FAILURE);

        }else if(pid == 0){

            //processo filho
            
            //gerando uma sequência aleatória de pid
            srand(time(NULL) ^getpid());

            //chamando a função do filósofo
            filosofo(i, mem);

            _exit(EXIT_SUCCESS);

        }


    }

    //fazendo o processo principal aguardar tds os filhos terminarem
    for(int i = 0; i < NUM_FILOSOFOS; i++){

        wait(NULL);

    }

    printf("Todos os filosofos terminaram suas refeições! \n");

    //liberando os semáforos
    for(int i = 0; i < NUM_FILOSOFOS; i++){

        sem_destroy(&mem->garfos[i]);

    }

    //liberando o semáforo limite
    sem_destroy(&mem->limite);

    //liberando o mutex
    pthread_mutex_destroy(&mem->mutex);

    //liberando a memória compartilhada
    munmap(mem, sizeof(MemCompartilhada));


    return 0;

}