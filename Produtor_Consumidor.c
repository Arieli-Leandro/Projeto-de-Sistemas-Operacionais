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
    if(mem == MAP_FAILED){

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

// -> O produtor é responsável por fabricar elementos e colocá-los no buffer compartilhado
/* O que o produtor precisa fazer?
1) Verificar se tem espaço
2) Solicitar acesso ao buffer
3) Inserir o produto
4) Atualizar a posição do buffer (tem que ser circular)
5) Liberar o buffer
6) Avisar que existe um produto no buffer

-> O produtor não pode ficar segurando o mutex enquanto espera o buffer liberar uma posição no vetor

*/

//criando a função responsável por criar o produtor
void produtor(ArmMemoriaCompartilhada *mem, int id){

    //estou produzindo exatamente a msm quantidade de posições do meu vetor
    for(int i = 0; i< ITENS_POR_PRODUTOR; i++){

        //aguarda até liberar espaço
        sem_wait(&mem->buffer_vazio);

        //bloqueando a região crítica de memória para que o produtor ou o consumidor não alterem a msm região ao mesmo tempo
        pthread_mutex_lock(&mem->mutex);

        //produzindo um elemento qlqr
        int produto = id * 100 + i + 1;

        //insere o produto dentro do buffer
        mem->buffer[mem->in] = produto;

        printf("O produtor %d produziu: %d na posição: %d \n", id, produto, mem->in);
        fflush(stdout);

        //atualiza a posição de inserção -> a posição está num buffer circular
        mem->in = (mem->in + 1) % TAM_BUFFER;

        //desbloqueando a região crítica de memória
        pthread_mutex_unlock(&mem->mutex);

        //informando ao semáforo que existe um novo produto
        sem_post(&mem->buffer_cheio);

        //Só pra ficar melhor pra visualizar
        sleep(1);

    }

    return;

}

//O consumidor é responsável por consumir oq acabamos de criar
/*O que o consumidor precisar fazer? (basicamente é o contrário do produtor)
1) Verificar se existe produto disponivel
2) solicitar acesso ao buffer
3) Retirar um produto
4) Atualizar a posição
5) Liberar o buffer
6) Avisar q existe um espaço q acabou de liberar

-> Aqui é aonde tem a inversão dos papéis do semáforo

*/


//criando a função do consumidor
void consumidor(ArmMemoriaCompartilhada *mem, int id){

    for(int i = 0; i< ITENS_POR_PRODUTOR; i++){

        //se não tem produto disponível, aguarda até ter
        sem_wait(&mem->buffer_cheio);

        //bloqueia e entra na região crítica de memória
        pthread_mutex_lock(&mem->mutex);

        //consome o produto do buffer
        int produto = mem->buffer[mem->out];

        printf("Consumidor %d consumiu: %d da posição %d \n", id, produto, mem->out);
        fflush(stdout);

        //atualiza a posição de retirada no buffer circular
        mem->out = (mem->out + 1) % TAM_BUFFER;

        //saindo e desbloqueando a região crítica de memória
        pthread_mutex_unlock(&mem->mutex);

        //informa o semáforo q existe uma posição vazia
        sem_post(&mem->buffer_vazio);

        //Só pra ficar melhor pra ver
        sleep(1);

    }

    return;

}

int main(){

    //criando a memória compartilhada
    ArmMemoriaCompartilhada *mem = criaMemoriaCompartilhada();

    //inicializando as posições do buffer
    mem->in = 0;
    mem->out = 0;

    //inicializando os mecanismos de sincronização
    inicializaSemaforo(mem);
    inicializaMutex(mem);

    //criando os processos do produtor
    for(int i = 0; i < NUM_PRODUTORES; i++){

        pid_t pid = fork();

        //processo q deu erro
        if(pid < 0){

            printf("Erro ao criar o processo do produtor! \n");
            exit(EXIT_FAILURE);

        }else if(pid == 0){

            //processo filho
            produtor(mem, i + 1);
            exit(EXIT_SUCCESS);

        }

    }

    // criando os processos do consumidor
    for (int i = 0; i < NUM_CONSUMIDORES; i++) {

        pid_t pid = fork();

        if (pid < 0) {
            
            printf("Erro ao criar o processo do produtor! \n");
            exit(EXIT_FAILURE);

        }else if(pid == 0){

            consumidor(mem, i + 1);
            exit(EXIT_SUCCESS);

        }
        
    }

    //aguardando os processos filhos terminares
    for(int i = 0; i < NUM_PRODUTORES + NUM_CONSUMIDORES; i++){

        wait(NULL);

    }

    //liberando os recursos
    pthread_mutex_destroy(&mem->mutex);
    sem_destroy(&mem->buffer_vazio);
    sem_destroy(&mem->buffer_cheio);

    munmap(mem, sizeof(ArmMemoriaCompartilhada));

    return 0;

}