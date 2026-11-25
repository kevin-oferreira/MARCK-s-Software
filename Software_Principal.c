#include <stdio.h>
#include <string.h>

#define MAX_PRODUTOS 100
#define MAX_TIPOS 25
#define ARQUIVO "marck_dados.txt"
#define ARQUIVO_VENDAS "Registro de vendas.txt"

int quantidadeProdutos = 9;
int quantidadeTipos = 3;
int produtosPorTipo[MAX_TIPOS];
char produto[MAX_PRODUTOS][100] = {
    "Pastel de carne", 
    "Pastel de frango e catupiry",
    "Bolo gelado de chocolate", 
    "Pastel de pizza",
    "Mousse de maracuja", 
    "Coca-cola 2L", 
    "Pudim",
    "Sprite 2L", 
    "Guarana 2L"
};
char tipo[MAX_TIPOS][30] = {"Salgado", "Doce", "Bebida"};
int tipoPorProduto[MAX_PRODUTOS] = {0,0,1,0,1,2,1,2,2};
float preco[MAX_PRODUTOS] = {10,10,7.5,10,5,12,5,12,12};
int estoque[MAX_PRODUTOS] = {20,20,20,20,20,20,20,20,20};
int compra[MAX_PRODUTOS] = {0};
int indicePorTipo[MAX_TIPOS][MAX_PRODUTOS];

/* Le uma linha inteira, descartando o excesso para nao afetar a proxima leitura. */
int lerLinha(char destino[], int capacidade){
    if(fgets(destino, capacidade, stdin) == NULL) return 0;
    size_t n = strlen(destino);
    if(n > 0 && destino[n-1] == '\n') destino[n-1] = '\0';
    else {
        int c;
        while((c = getchar()) != '\n' && c != EOF) {}
        return -1;
    }
    return 1;
}

/* Le um numero e rejeita letras ou caracteres extras. */
int lerInteiro(const char mensagem[], int *resultado){
    char linha[100], extra;
    printf("%s", mensagem);
    int status = lerLinha(linha, sizeof(linha));
    if(status != 1) return status;
    if(sscanf(linha, " %d %c", resultado, &extra) != 1) return -1;
    return 1;
}

int lerPreco(const char mensagem[], float *resultado){
    char linha[100], extra;
    printf("%s", mensagem);
    int status = lerLinha(linha, sizeof(linha));
    if(status != 1) return status;
    if(sscanf(linha, " %f %c", resultado, &extra) != 1) return -1;
    if(!(*resultado >= 0 && *resultado <= 1000000)) return -1;
    return 1;
}

void organizarItens(){
    for(int t=0; t<MAX_TIPOS; t++) produtosPorTipo[t] = 0;
    for(int t=0; t<quantidadeTipos; t++){
        for(int i=0; i<quantidadeProdutos; i++){
            if(tipoPorProduto[i] == t){
                indicePorTipo[t][produtosPorTipo[t]] = i;
                produtosPorTipo[t]++;
            }
        }
    }
}

/* Arquivo texto com formato proprio: cabecalho, tipos e produtos. */
int salvarDados(){
    FILE *f = fopen(ARQUIVO ".tmp", "w");
    if(f == NULL){ perror("Nao foi possivel salvar"); return 0; }
    int ok = fprintf(f, "MARCK1\n%d %d\n", quantidadeTipos, quantidadeProdutos) >= 0;
    for(int t=0; t<quantidadeTipos && ok; t++)
        ok = fprintf(f, "%s\n", tipo[t]) >= 0;
    for(int i=0; i<quantidadeProdutos && ok; i++)
        ok = fprintf(f, "%s\n%d %.9g %d\n", produto[i], tipoPorProduto[i], preco[i], estoque[i]) >= 0;
    if(fclose(f) != 0) ok = 0;
    if(!ok){ remove(ARQUIVO ".tmp"); puts("Erro ao escrever os dados."); return 0; }
    /* Renomeacao direta funciona no POSIX; no Windows, remove o arquivo antigo primeiro. */
    if(rename(ARQUIVO ".tmp", ARQUIVO) != 0){
        remove(ARQUIVO);
        if(rename(ARQUIVO ".tmp", ARQUIVO) != 0){
            perror("Erro ao concluir salvamento"); return 0;
        }
    }
    return 1;
}

int carregarDados(){
    FILE *f = fopen(ARQUIVO, "r");
    if(f == NULL) return 0;
    char assinatura[32];
    int nt, np;
    char tiposTemp[MAX_TIPOS][30];
    char produtosTemp[MAX_PRODUTOS][100];
    int categoriasTemp[MAX_PRODUTOS], estoquesTemp[MAX_PRODUTOS];
    float precosTemp[MAX_PRODUTOS];
    if(!fgets(assinatura, sizeof(assinatura), f) || strcmp(assinatura, "MARCK1\n") != 0 ||
       fscanf(f, "%d %d\n", &nt, &np) != 2 ||
       nt < 0 || nt > MAX_TIPOS || np < 0 || np > MAX_PRODUTOS || (nt == 0 && np != 0)) goto invalido;
    for(int t=0; t<nt; t++){
        if(!fgets(tiposTemp[t], sizeof(tiposTemp[t]), f)) goto invalido;
        size_t n = strlen(tiposTemp[t]);
        if(n == 0 || tiposTemp[t][n-1] != '\n') goto invalido;
        tiposTemp[t][n-1] = '\0';
        if(tiposTemp[t][0] == '\0') goto invalido;
    }
    for(int i=0; i<np; i++){
        if(!fgets(produtosTemp[i], sizeof(produtosTemp[i]), f)) goto invalido;
        size_t n = strlen(produtosTemp[i]);
        if(n == 0 || produtosTemp[i][n-1] != '\n') goto invalido;
        produtosTemp[i][n-1] = '\0';
        if(produtosTemp[i][0] == '\0') goto invalido;
        if(fscanf(f, "%d %f %d\n", &categoriasTemp[i], &precosTemp[i], &estoquesTemp[i]) != 3 ||
           categoriasTemp[i] < 0 || categoriasTemp[i] >= nt ||
           !(precosTemp[i] >= 0 && precosTemp[i] <= 1000000) || estoquesTemp[i] < 0) goto invalido;
    }
    fclose(f);
    quantidadeTipos = nt;
    quantidadeProdutos = np;
    for(int t=0; t<nt; t++) strcpy(tipo[t], tiposTemp[t]);
    for(int i=0; i<np; i++){
        strcpy(produto[i], produtosTemp[i]);
        tipoPorProduto[i] = categoriasTemp[i];
        preco[i] = precosTemp[i];
        estoque[i] = estoquesTemp[i];
        compra[i] = 0;
    }
    organizarItens();
    return 1;
invalido:
    fclose(f);
    puts("Arquivo de dados invalido. O catalogo inicial sera utilizado.");
    return 0;
}

void registraTipo(){
    if(quantidadeTipos >= MAX_TIPOS){ puts("Limite de tipos atingido."); return; }
    char nome[30];
    printf("Nome do novo tipo: ");
    if(lerLinha(nome, sizeof(nome)) != 1){ puts("Nome invalido ou muito longo."); return; }
    if(nome[0] == '\0'){ puts("Nome vazio."); return; }
    for(int i=0; i<quantidadeTipos; i++)
        if(strcmp(nome, tipo[i]) == 0){ puts("Esse tipo ja existe."); return; }
    strcpy(tipo[quantidadeTipos], nome);
    quantidadeTipos++;
    organizarItens();
    puts("Tipo registrado!");
    salvarDados();
}

void cadastrarProduto(){
    if(quantidadeProdutos >= MAX_PRODUTOS){ puts("Limite de produtos atingido."); return; }
    if(quantidadeTipos == 0){ puts("Cadastre um tipo primeiro."); return; }
    char nome[100];
    float valor;
    int qt, escolha;
    printf("Nome do produto: ");
    if(lerLinha(nome, sizeof(nome)) != 1){ puts("Nome invalido ou muito longo."); return; }
    if(nome[0] == '\0'){ puts("Nome vazio."); return; }
    for(int j=0; j<quantidadeProdutos; j++)
        if(strcmp(nome, produto[j]) == 0){ puts("Esse produto ja existe."); return; }
    if(lerPreco("Preco (use ponto decimal): ", &valor) != 1){ puts("Preco invalido."); return; }
    if(lerInteiro("Estoque inicial: ", &qt) != 1 || qt < 0){ puts("Estoque invalido."); return; }
    puts("\n===== TIPOS =====");
    for(int j=0; j<quantidadeTipos; j++) printf("%d - %s\n", j+1, tipo[j]);
    printf("%d - Cancelar\n", quantidadeTipos+1);
    if(lerInteiro("Escolha um tipo: ", &escolha) != 1 || escolha < 1 || escolha > quantidadeTipos+1){
        puts("Tipo invalido."); return;
    }
    if(escolha == quantidadeTipos+1){ puts("Cadastro cancelado."); return; }
    int i = quantidadeProdutos;
    strcpy(produto[i], nome);
    preco[i] = valor;
    estoque[i] = qt;
    compra[i] = 0;
    tipoPorProduto[i] = escolha-1;
    quantidadeProdutos++;
    organizarItens();
    puts("Produto registrado!");
    salvarDados();
}

/* Remove pelo indice global, de 0 a quantidadeProdutos-1. */
void deletaProduto(int indice){
    if(indice < 0 || indice >= quantidadeProdutos) return;
    for(int i=indice; i<quantidadeProdutos-1; i++){
        strcpy(produto[i], produto[i+1]);
        preco[i] = preco[i+1];
        estoque[i] = estoque[i+1];
        compra[i] = compra[i+1];
        tipoPorProduto[i] = tipoPorProduto[i+1];
    }
    quantidadeProdutos--;
    organizarItens();
}

void deletaTipo(int indice){
    if(indice < 0 || indice >= quantidadeTipos) return;
    for(int i=0; i<quantidadeProdutos; ){
        if(tipoPorProduto[i] == indice) deletaProduto(i);
        else i++;
    }
    for(int i=indice; i<quantidadeTipos-1; i++) strcpy(tipo[i], tipo[i+1]);
    quantidadeTipos--;
    for(int i=0; i<quantidadeProdutos; i++)
        if(tipoPorProduto[i] > indice) tipoPorProduto[i]--;
    organizarItens();
}

void excluirProdutoMenu(){
    if(quantidadeProdutos == 0){ puts("Nao ha produtos."); return; }
    puts("\n===== EXCLUIR PRODUTO =====");
    for(int i=0; i<quantidadeProdutos; i++)
        printf("%d - %s (%s)\n", i+1, produto[i], tipo[tipoPorProduto[i]]);
    printf("%d - Cancelar\n", quantidadeProdutos+1);
    int escolha;
    if(lerInteiro("Produto: ", &escolha) != 1 || escolha < 1 || escolha > quantidadeProdutos+1){
        puts("Opcao invalida."); return;
    }
    if(escolha == quantidadeProdutos+1) return;
    printf("Excluir '%s'? 1 - Sim / 2 - Nao\n", produto[escolha-1]);
    int confirma;
    if(lerInteiro("Confirmar: ", &confirma) != 1 || confirma != 1) return;
    deletaProduto(escolha-1);
    puts("Produto excluido!");
    salvarDados();
}

void excluirTipoMenu(){
    if(quantidadeTipos == 0){ puts("Nao ha tipos."); return; }
    puts("\n===== EXCLUIR TIPO =====");
    for(int t=0; t<quantidadeTipos; t++)
        printf("%d - %s (%d produtos)\n", t+1, tipo[t], produtosPorTipo[t]);
    printf("%d - Cancelar\n", quantidadeTipos+1);
    int escolha;
    if(lerInteiro("Tipo: ", &escolha) != 1 || escolha < 1 || escolha > quantidadeTipos+1){
        puts("Opcao invalida."); return;
    }
    if(escolha == quantidadeTipos+1) return;
    printf("ATENCAO: excluir '%s' tambem excluira %d produto(s).\n",
           tipo[escolha-1], produtosPorTipo[escolha-1]);
    int confirma;
    if(lerInteiro("1 - Confirmar / 2 - Cancelar: ", &confirma) != 1 || confirma != 1) return;
    deletaTipo(escolha-1);
    puts("Tipo e seus produtos excluidos!");
    salvarDados();
}

void atualizaEstoque(){
    if(quantidadeProdutos == 0){ puts("Nao ha produtos registrados."); return; }
    int escolha;
    do{
        puts("\n===== ATUALIZAR ESTOQUE =====");
        for(int i=0; i<quantidadeProdutos; i++)
            printf("%d - %s | Estoque: %d\n", i+1, produto[i], estoque[i]);
        printf("%d - Voltar\n", quantidadeProdutos+1);
        int status = lerInteiro("Escolha um produto: ", &escolha);
        if(status == 0) return;
        if(status != 1){ puts("Opcao invalida."); continue; }
        if(escolha == quantidadeProdutos+1) return;
        if(escolha < 1 || escolha > quantidadeProdutos){ puts("Opcao invalida."); continue; }
        int novo;
        status = lerInteiro("Novo estoque total: ", &novo);
        if(status == 0) return;
        if(status != 1 || novo < 0){ puts("Estoque invalido."); continue; }
        estoque[escolha-1] = novo;
        puts("Estoque atualizado!");
        salvarDados();
    }while(1);
}

void mostraTipo(){
    puts("\n===== TIPOS =====");
    for(int i=0; i<quantidadeTipos; i++) printf("%d - %s\n", i+1, tipo[i]);
    printf("%d - Finalizar pedido\n", quantidadeTipos+1);
}

void mostraProduto(int t){
    printf("\n===== %s =====\n", tipo[t-1]);
    for(int i=0; i<produtosPorTipo[t-1]; i++){
        int j = indicePorTipo[t-1][i];
        printf("%d - %s | R$%.2f | Carrinho: %d | Estoque: %d\n",
               i+1, produto[j], preco[j], compra[j], estoque[j]);
    }
    printf("%d - Voltar aos tipos\n", produtosPorTipo[t-1]+1);
}

void adicionarAoPedido(int t, int i){
    int j = indicePorTipo[t-1][i-1];
    int qt;
    int status = lerInteiro("Quantidade (negativa para remover do carrinho): ", &qt);
    if(status != 1){ puts("Quantidade invalida."); return; }
    long nova = (long)compra[j] + qt;
    if(nova >= 0 && nova <= estoque[j]){
        compra[j] = (int)nova;
        puts("Pedido atualizado!");
    }else puts("Quantidade fora do limite.");
}

void pedirProduto(int t){
    int escolha;
    int status = lerInteiro("Escolha um produto: ", &escolha);
    if(status != 1){ puts("Opcao invalida."); return; }
    if(escolha >= 1 && escolha <= produtosPorTipo[t-1]) adicionarAoPedido(t, escolha);
    else if(escolha != produtosPorTipo[t-1]+1) puts("Opcao invalida.");
}

/* Procura o maior ID registrado para continuar a numeracao apos reiniciar. */
unsigned long proximoIdNota(){
    FILE *f = fopen(ARQUIVO_VENDAS, "r");
    if(f == NULL) return 1;
    char linha[256];
    unsigned long maior = 0, id;
    while(fgets(linha, sizeof(linha), f) != NULL){
        if(sscanf(linha, "========== NOTA #%lu ==========", &id) == 1 && id > maior)
            maior = id;
    }
    fclose(f);
    return maior + 1;
}

/* Escreve o mesmo comprovante na tela e no arquivo de historico. */
void escreverNota(FILE *destino, unsigned long id, float total){
    fprintf(destino, "\n========== NOTA #%06lu ==========\n", id);
    fprintf(destino, "\n");
    for(int i=0; i<quantidadeProdutos; i++){
        if(compra[i] > 0){
            fprintf(destino, "%d x %s\n", compra[i], produto[i]);
            fprintf(destino, "    %d x R$%.2f = R$%.2f\n",
                    compra[i], preco[i], compra[i] * preco[i]);
        }
    }
    fprintf(destino, "\nTOTAL: R$%.2f\n", total);
    fprintf(destino, "==================================\n");
}

int imprimirNota(){
    float total = 0;
    int itens = 0;
    for(int i=0; i<quantidadeProdutos; i++){
        itens += compra[i];
        total += compra[i] * preco[i];
    }
    if(itens == 0){ puts("Carrinho vazio. Nenhuma venda registrada."); return 0; }

    unsigned long id = proximoIdNota();
    FILE *historico = fopen(ARQUIVO_VENDAS, "a");
    if(historico == NULL){
        perror("Nao foi possivel abrir o registro de vendas");
        puts("Venda nao finalizada. Carrinho e estoque preservados.");
        return 0;
    }
    escreverNota(historico, id, total);
    int erroEscrita = ferror(historico);
    if(fclose(historico) != 0) erroEscrita = 1;
    if(erroEscrita){
        puts("Erro ao registrar a venda. Verifique o historico antes de tentar novamente.");
        return 0;
    }
    escreverNota(stdout, id, total);
    for(int i=0; i<quantidadeProdutos; i++){
        estoque[i] -= compra[i];
        compra[i] = 0;
    }
    if(!salvarDados()) puts("ATENCAO: venda registrada, mas nao foi possivel salvar o estoque.");
    else printf("Venda #%06lu registrada em '%s'.\n", id, ARQUIVO_VENDAS);
    return 1;
}

void iniciarPedido(){
    if(quantidadeProdutos == 0 || quantidadeTipos == 0){ puts("Nao ha produtos disponiveis."); return; }
    int opcaoTipo;
    while(1){
        mostraTipo();
        int status = lerInteiro("Escolha uma categoria: ", &opcaoTipo);
        if(status == 0){ puts("Entrada encerrada. Pedido cancelado."); break; }
        if(status != 1){ puts("Opcao invalida."); continue; }
        if(opcaoTipo == quantidadeTipos+1){
            int confirma;
            if(lerInteiro("1 - Finalizar venda / 2 - Cancelar pedido: ", &confirma) != 1) continue;
            if(confirma == 1) imprimirNota();
            else puts("Pedido cancelado.");
            break;
        }
        if(opcaoTipo >= 1 && opcaoTipo <= quantidadeTipos){
            mostraProduto(opcaoTipo);
            pedirProduto(opcaoTipo);
        }else puts("Opcao invalida.");
    }
    for(int i=0; i<quantidadeProdutos; i++) compra[i] = 0;
}

void logo(){
    printf(" __  __     _     ____    ____  _  __\n");
    printf("|  \\/  |   / \\   |  _ \\  / ___|| |/ /\n");
    printf("| |\\/| |  / _ \\  | |_) || |    | ' / \n");
    printf("| |  | | / ___ \\ |  _ < | |___ |   \\ \n");
    printf("|_|  |_|/_/   \\_\\|_| \\_\\ \\____||_|\\_\\\n\n");
}

void editarMenu(){
    while(1){
        printf("\n===== EDITAR MENU =====\nProdutos: %d/%d\nTipos: %d/%d\n",
               quantidadeProdutos, MAX_PRODUTOS, quantidadeTipos, MAX_TIPOS);
        puts("1 - Registrar produto");
        puts("2 - Registrar tipo");
        puts("3 - Deletar produto");
        puts("4 - Deletar tipo");
        puts("5 - Atualizar estoque");
        puts("6 - Voltar");
        int escolha;
        int status = lerInteiro("Escolha: ", &escolha);
        if(status == 0) return;
        if(status != 1){ puts("Opcao invalida."); continue; }
        switch(escolha){
            case 1: cadastrarProduto(); break;
            case 2: registraTipo(); break;
            case 3: excluirProdutoMenu(); break;
            case 4: excluirTipoMenu(); break;
            case 5: atualizaEstoque(); break;
            case 6: puts("Voltando ao menu principal..."); return;
            default: puts("Opcao invalida.");
        }
    }
}

void decisaoInicial(){
    while(1){
        puts("\nOla, seja bem-vindo a MARCK's Softwares!");
        puts("1 - Fazer pedido");
        puts("2 - Editar menu");
        puts("3 - Salvar e sair");
        int escolha;
        int status = lerInteiro("Escolha: ", &escolha);
        if(status == 0){ salvarDados(); return; }
        if(status != 1){ puts("Opcao invalida."); continue; }
        switch(escolha){
            case 1: iniciarPedido(); break;
            case 2: editarMenu(); break;
            case 3: salvarDados(); puts("Saindo..."); return;
            default: puts("Opcao invalida.");
        }
    }
}

int main(void){
    if(carregarDados()) puts("Dados carregados do arquivo.");
    organizarItens();
    logo();
    decisaoInicial();
    return 0;
}
