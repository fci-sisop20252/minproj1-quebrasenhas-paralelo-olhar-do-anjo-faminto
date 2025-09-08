#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>
#include <time.h>
#include "hash_utils.h"

/**
 * PROCESSO COORDENADOR - Mini-Projeto 1: Quebra de Senhas Paralelo
 * 
 * Este programa coordena múltiplos workers para quebrar senhas MD5 em paralelo.
 * O MD5 JÁ ESTÁ IMPLEMENTADO - você deve focar na paralelização (fork/exec/wait).
 * 
 * Uso: ./coordinator <hash_md5> <tamanho> <charset> <num_workers>
 * 
 * Exemplo: ./coordinator "900150983cd24fb0d6963f7d28e17f72" 3 "abc" 4
 * 
 * SEU TRABALHO: Implementar os TODOs marcados abaixo
 */

#define MAX_WORKERS 16
#define RESULT_FILE "password_found.txt"

/**
 * Calcula o tamanho total do espaço de busca
 * 
 * @param charset_len Tamanho do conjunto de caracteres
 * @param password_len Comprimento da senha
 * @return Número total de combinações possíveis
 */
long long calculate_search_space(int charset_len, int password_len) {
    long long total = 1;
    for (int i = 0; i < password_len; i++) {
        total *= charset_len;
    }
    return total;
}

/**
 * Converte um índice numérico para uma senha
 * Usado para definir os limites de cada worker
 * 
 * @param index Índice numérico da senha
 * @param charset Conjunto de caracteres
 * @param charset_len Tamanho do conjunto
 * @param password_len Comprimento da senha
 * @param output Buffer para armazenar a senha gerada
 */
void index_to_password(long long index, const char *charset, int charset_len, 
                       int password_len, char *output) {
    for (int i = password_len - 1; i >= 0; i--) {
        output[i] = charset[index % charset_len];
        index /= charset_len;
    }
    output[password_len] = '\0';
}

/**
 * Função principal do coordenador
 */
int main(int argc, char *argv[]) {
    // TODO 1: Validar argumentos de entrada
    // Verificar se argc == 5 (programa + 4 argumentos)
    // Se não, imprimir mensagem de uso e sair com código 1
    
    // IMPLEMENTE AQUI: verificação de argc e mensagem de erro
    if (argc != 5) {
        fprintf(stderr, "Uso: %s <hash> <tamanho> <charset> <workers>\n", argv[0]);
        return 1;
    }

    // Parsing dos argumentos (após validação)
    const char *target_hash = argv[1];
    int password_len = atoi(argv[2]);
    const char *charset = argv[3];
    int num_workers = atoi(argv[4]);
    int charset_len = strlen(charset);
    
    // TODO: Adicionar validações dos parâmetros
    // - password_len deve estar entre 1 e 10
    if (password_len < 1 || password_len > 10) {
        fprintf(stderr, "Erro: Tamanho da senha deve estar entre 1 e 10.\n");
        return 1;
    }
    // - num_workers deve estar entre 1 e MAX_WORKERS
    if (num_workers < 1 || num_workers > MAX_WORKERS) {
        fprintf(stderr, "Erro: Número de workers deve estar entre 1 e %d.\n", MAX_WORKERS);
        return 1;
    }
    // - charset não pode ser vazio
    if (charset_len == 0) {
        fprintf(stderr, "Erro: Charset não pode ser vazio.\n");
        return 1;
    }
    
    printf("=== Mini-Projeto 1: Quebra de Senhas Paralelo ===\n");
    printf("Hash MD5 alvo: %s\n", target_hash);
    printf("Tamanho da senha: %d\n", password_len);
    printf("Charset: %s (tamanho: %d)\n", charset, charset_len);
    printf("Número de workers: %d\n", num_workers);
    
    // Calcular espaço de busca total
    long long total_space = calculate_search_space(charset_len, password_len);
    printf("Espaço de busca total: %lld combinações\n\n", total_space);
    
    // Remover arquivo de resultado anterior se existir
    unlink(RESULT_FILE);
    
    // Registrar tempo de início
    time_t start_time = time(NULL);
    
    // TODO 2: Dividir o espaço de busca entre os workers
    // Calcular quantas senhas cada worker deve verificar
    // DICA: Use divisão inteira e distribua o resto entre os primeiros workers
    
    // IMPLEMENTE AQUI:
    long long passwords_per_worker = total_space / num_workers;
    long long remaining = total_space % num_workers;
    
    // Arrays para armazenar PIDs dos workers
    pid_t workers[MAX_WORKERS];
    
    // TODO 3: Criar os processos workers usando fork()
    printf("Iniciando workers...\n");
    
    // IMPLEMENTE AQUI: Loop para criar workers
    for (int i = 0; i < num_workers; i++) {
        // TODO: Calcular intervalo de senhas para este worker
        long long inicio, fim;
        if (i < remaining) {
            inicio = i * (passwords_per_worker + 1); // i = 0, inicio = 0
            fim = inicio + passwords_per_worker; // i = 0, fim = 0 + passwords_per_worker
        } else {
            inicio = remaining * (passwords_per_worker + 1) + (i - remaining) * passwords_per_worker;
            fim = inicio + passwords_per_worker - 1;
        }

        // TODO: Converter indices para senhas de inicio e fim
        char password_inicio[password_len + 1];
        char password_fim[password_len + 1];
        index_to_password(inicio, charset, charset_len, password_len, password_inicio);
        index_to_password(fim, charset, charset_len, password_len, password_fim);

        // TODO 4: Usar fork() para criar processo filho
        // TODO 5: No processo pai: armazenar PID
        // TODO 6: No processo filho: usar execl() para executar worker
        // TODO 7: Tratar erros de fork() e execl()
        pid_t pid = fork();
        if (pid < 0) {
            perror("Erro ao criar worker");
            exit(1);
        } else if (pid == 0) {
            execl("./worker", "worker", "............" ,NULL);
            perror("Erro ao executar worker");
            exit(1);
        } else {
            workers[i] = pid;
            printf("ID: %d | PID: %d | Intervalo: %s a %s\n", i, pid, password_inicio, password_fim);
        }

    }
    
    printf("\nTodos os workers foram iniciados. Aguardando conclusão...\n");
    
    // TODO 8: Aguardar todos os workers terminarem usando wait()
    // IMPORTANTE: O pai deve aguardar TODOS os filhos para evitar zumbis
    
    // IMPLEMENTE AQUI:
    int contador = 0;
    // - Loop para aguardar cada worker terminar
    for (int i = 0; i < num_workers; i++) {
        int status;
        
        // - Usar wait() para capturar status de saída
        pid_t child_pid = wait(&status);
        // - Identificar qual worker terminou
        if (WIFEXITED(status)) {
            int exit_code = WEXITSTATUS(status);
            contador++;
            // - Verificar se terminou normalmente ou com erro
            if (exit_code == 0) {
                printf("Terminou normalmente.\n", child_pid);
            } else {
                printf("Terminou com erro (código %d).\n", child_pid, exit_code);
            }
        }
    }
    // - Contar quantos workers terminaram
    printf("Total de workers que terminaram: %d\n", contador);
    
    // Registrar tempo de fim
    time_t end_time = time(NULL);
    double elapsed_time = difftime(end_time, start_time);
    
    printf("\n=== Resultado ===\n");
    
    // TODO 9: Verificar se algum worker encontrou a senha
    
    // Ler o arquivo password_found.txt se existir
    int fd = open(RESULT_FILE, O_RDONLY); // Abrir arquivo RESULT_FILE para leitura
    if (fd >= 0) { 
        char buffer[256];
        ssize_t bytes_read = read(fd, buffer, sizeof(buffer) - 1); // Ler conteúdo do arquivo
        close(fd);

        if (bytes_read > 0) { // Se conseguiu ler
            buffer[bytes_read] = '\0'; // garante que o ultimo caractere seja uma terminacao de string

            // Fazer parse do formato "worker_id:password"
            int found_worker;
            char found_password[128];
            if (sscanf(buffer, "%d:%127s", &found_worker, found_password) == 2) {
                // Calcular o hash da senha encontrada
                char verify_hash[33];
                md5_string(found_password, verify_hash); //Verificar o hash usando md5_string()

                // - Exibir resultado encontrado
                printf("[Worker %d] Outro worker já encontrou!\n", worker_id);
                printf("Worker %d encontrou a senha: %s\n", found_worker, found_password);
                printf("Hash verificado: %s\n", verify_hash);
            } else {
                printf("Formato inesperado em %s: %s\n", RESULT_FILE, buffer);
            }
        }
    }


    
    // Estatísticas finais (opcional)
    // TODO: Calcular e exibir estatísticas de performance

    time_t end_time = time(NULL);
    double total_time = difftime(end_time, start_time);

    printf("[Worker %d] Finalizado. Total: %lld senhas em %.2f segundos", worker_id, passwords_checked, total_time);
    if (total_time > 0) {
        printf(" (%.0f senhas/s)", passwords_checked / total_time);
    }
    printf("\n");
    
    return 0;
}