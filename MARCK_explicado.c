// Esse é o código da MARCK Pastelaria, agora com algumas funcionalidades a mais.
// Como o programa cresceu bastante, vou separar as explicações por partes, mas mantendo
// a mesma ideia do código menor: explicar o que cada função faz e como as variáveis se relacionam.
//
// A principal diferença desta versão é que agora temos tipos/categorias, cadastro e exclusão
// de produtos, atualização de estoque, leitura e gravação de arquivos e registro de vendas.
//
// Também passamos a usar ponteiros em algumas funções. Não precisa pensar neles como algo
// misterioso: neste código eles aparecem principalmente para que uma função consiga alterar
// uma variável que foi criada em outra função. Vou explicar isso quando chegarmos lá.
#include <stdio.h>
// A biblioteca stdio.h contém as funções de entrada e saída que já conhecemos, como printf()
// e scanf(), mas também fornece o necessário para trabalhar com arquivos: FILE, fopen(),
// fprintf(), fgets(), fclose(), remove(), rename(), perror() etc.
//
// Já string.h contém funções para manipular textos (strings), como strlen(), strcmp() e strcpy().
// Em C, uma string é armazenada em um vetor de caracteres e termina com o caractere especial \0.
#include <string.h>

// O #define cria uma substituição feita pelo pré-processador antes da compilação.
// Portanto, MAX_PRODUTOS não é uma variável: sempre que o compilador encontrar esse nome,
// ele será substituído pelo número 15. Isso é útil para definir os limites dos nossos vetores.
//
// MAX_PRODUTOS é a capacidade máxima do catálogo, enquanto MAX_TIPOS é a capacidade máxima
// de categorias. Os valores atuais podem ser menores; não precisamos preencher tudo.
// ARQUIVO e ARQUIVO_VENDAS guardam os nomes dos arquivos que serão usados pelo programa.
//
// Repare também que ARQUIVO ".tmp" será interpretado como duas strings literais juntas:
// "marck_dados.txt" + ".tmp" resulta em "marck_dados.txt.tmp". O próprio C faz essa junção.
#define MAX_PRODUTOS 15
#define MAX_TIPOS 5
#define ARQUIVO "marck_dados.txt"
#define ARQUIVO_VENDAS "Registro de vendas.txt"

// Essas variáveis globais descrevem os dados gerais do programa. São globais porque foram
// declaradas fora de qualquer função, então podem ser acessadas pelas várias funções do código.
// quantidadeProdutos e quantidadeTipos dizem quantos elementos estão efetivamente cadastrados,
// não quantos cabem nos vetores. Por exemplo, podemos ter capacidade para 15 produtos, mas
// somente 9 cadastrados no momento.
//
// Os vetores produto, preco, estoque e compra continuam seguindo a mesma lógica do código menor:
// um mesmo índice representa o mesmo produto em todos eles.
// produto[i] guarda o nome; preco[i], o preço; estoque[i], a quantidade disponível;
// compra[i], a quantidade desse produto que está no carrinho do cliente.
//
// A diferença é que agora os produtos também pertencem a um tipo. tipo[] guarda os nomes das
// categorias, enquanto tipoPorProduto[i] informa a categoria do produto de índice i.
// Essa relação usa índices começando em 0: 0 = primeiro tipo, 1 = segundo tipo etc.
//
// produtosPorTipo[t] contará quantos produtos pertencem ao tipo t.
// indicePorTipo[t][k] guardará o índice global de um produto dentro da categoria t.
// Isso permite mostrar uma lista separada por categorias sem precisar reorganizar o vetor produto.
//
// Como antes, as opções exibidas ao usuário começam em 1, mas os índices dos vetores começam
// em 0. Por isso, em vários lugares aparecerá a transformação escolha-1 ou t-1.
int quantidadeProdutos = 9;
int quantidadeTipos = 3;
int produtosPorTipo[MAX_TIPOS];
char produto[MAX_PRODUTOS][100] = {
    "Pastel de carne", "Pastel de frango e catupiry",
    "Bolo gelado de chocolate", "Pastel de pizza",
    "Mousse de maracuja", "Coca-cola 2L", "Pudim",
    "Sprite 2L", "Guarana 2L"
};
char tipo[MAX_TIPOS][30] = {"Salgado", "Doce", "Bebida"};
int tipoPorProduto[MAX_PRODUTOS] = {0,0,1,0,1,2,1,2,2};
float preco[MAX_PRODUTOS] = {10,10,7.5,10,5,12,5,12,12};
int estoque[MAX_PRODUTOS] = {20,20,20,20,20,20,20,20,20};
int compra[MAX_PRODUTOS] = {0};
int indicePorTipo[MAX_TIPOS][MAX_PRODUTOS];

/* Le uma linha inteira, descartando o excesso para nao afetar a proxima leitura. */
// Esta função lê uma linha inteira digitada pelo usuário e a guarda no vetor destino.
// Ela recebe também capacidade, que informa quantos caracteres cabem nesse vetor, contando
// inclusive o caractere final \0 que marca o fim da string.
//
// Aqui usamos fgets() em vez de scanf("%s"), porque fgets() consegue ler espaços. Assim,
// o usuário pode digitar "Pastel de frango e catupiry" como um único nome, por exemplo.
// fgets(destino, capacidade, stdin) lê no máximo capacidade-1 caracteres da entrada padrão,
// que é o teclado (stdin), e acrescenta \0 ao final do texto lido.
//
// A função devolve três estados: 1 quando a linha foi lida corretamente; 0 quando a leitura
// terminou por EOF/erro; e -1 quando a linha ultrapassou o espaço disponível ou não veio
// completa com a quebra de linha esperada. Essa distinção será aproveitada pelas outras funções.
int lerLinha(char destino[], int capacidade){
    // Se fgets() não conseguir ler nada, devolvemos 0 para sinalizar fim/erro de entrada.
    if(fgets(destino, capacidade, stdin) == NULL) return 0;
    size_t n = strlen(destino);
    // strlen() conta os caracteres antes do terminador \0. Se a última posição for \n,
    // trocamos essa quebra de linha por \0 para deixar somente o texto digitado.
    if(n > 0 && destino[n-1] == '\n') destino[n-1] = '\0';
    else {
        int c;
        // A linha não coube inteira no vetor. Consumimos o restante até a quebra de linha
        // ou o fim da entrada, para que esses caracteres não sejam confundidos com a próxima resposta.
        while((c = getchar()) != '\n' && c != EOF) {}
        return -1;
    }
    return 1;
}

/* Le um numero e rejeita letras ou caracteres extras. */
// Esta função lê um número inteiro de maneira um pouco mais cuidadosa do que um scanf() direto.
// mensagem é o texto que será mostrado antes da leitura. resultado é um ponteiro para um int.
//
// O asterisco em int *resultado indica que resultado não guarda diretamente um número inteiro:
// ele guarda o endereço de uma variável int que foi criada em outra parte do programa.
// Quando chamarmos a função, passaremos algo como &escolha. O operador & obtém o endereço
// da variável escolha; assim, a função consegue escrever o número lido diretamente nela.
//
// Dentro da função, *resultado acessa o valor que está naquele endereço. Portanto, em
// sscanf(..., resultado, ...), o número lido é colocado na variável original do chamador.
//
// Primeiro, usamos lerLinha() para obter o texto completo. Depois, sscanf() tenta interpretar
// esse texto como inteiro. O formato " %d %c" tem um detalhe importante: %d lê o inteiro e
// %c tenta ler algum caractere não branco que tenha sobrado. Se houver letras ou símbolos extras,
// sscanf() fará duas conversões, e a condição != 1 rejeitará a entrada. Assim, "12abc" não passa.
//
// O retorno mantém os estados de lerLinha(): 1 = inteiro válido; 0 = fim/erro de entrada;
// -1 = entrada inválida. Os espaços no formato fazem a leitura ignorar espaços em branco.
int lerInteiro(const char mensagem[], int *resultado){
    char linha[100], extra;
    printf("%s", mensagem);
    int status = lerLinha(linha, sizeof(linha));
    if(status != 1) return status;
    // Se houver um caractere extra depois do número, sscanf() retorna 2 em vez de 1.
    if(sscanf(linha, " %d %c", resultado, &extra) != 1) return -1;
    return 1;
}

// Esta função é praticamente a mesma ideia de lerInteiro(), mas agora recebe um float,
// ou seja, um número que pode ter parte decimal. Por isso usamos %f no sscanf().
//
// O ponteiro float *resultado funciona como o ponteiro da função anterior: passamos o endereço
// de uma variável float e *resultado permite acessar e modificar o valor guardado nela.
//
// Além de verificar se a entrada contém somente um número, também verificamos se ele está
// entre 0 e 1.000.000. A expressão entre parênteses define esse intervalo; o ! na frente
// inverte o resultado lógico. Portanto, se o preço não estiver dentro do intervalo, retorna -1.
//
// O programa pede ponto decimal, por exemplo 7.5, porque a leitura numérica segue a convenção
// da configuração usada pelo programa/ambiente; não estamos convertendo vírgula em ponto aqui.
int lerPreco(const char mensagem[], float *resultado){
    char linha[100], extra;
    printf("%s", mensagem);
    int status = lerLinha(linha, sizeof(linha));
    if(status != 1) return status;
    if(sscanf(linha, " %f %c", resultado, &extra) != 1) return -1;
    if(!(*resultado >= 0 && *resultado <= 1000000)) return -1;
    return 1;
}

// Esta função constrói a relação entre os produtos e suas categorias.
// Lembre que os produtos continuam guardados em um único vetor global, produto[].
// Não criamos um vetor separado para cada categoria; em vez disso, montamos uma espécie de
// índice auxiliar para descobrir quais posições de produto[] pertencem a cada tipo.
//
// Primeiro zeramos produtosPorTipo[] para apagar as contagens anteriores. O laço vai até
// MAX_TIPOS, e não somente até quantidadeTipos, porque queremos deixar todas as posições
// da tabela de contagem em um estado conhecido.
//
// Depois, para cada tipo t que realmente existe, percorremos todos os produtos i.
// Se tipoPorProduto[i] == t, significa que o produto i pertence à categoria t.
// Guardamos então i em indicePorTipo[t][produtosPorTipo[t]] e aumentamos a contagem.
//
// Exemplo: se os produtos 0, 2 e 5 forem da categoria 1, teremos algo como
// indicePorTipo[1][0] = 0, indicePorTipo[1][1] = 2 e indicePorTipo[1][2] = 5.
// Assim, quando quisermos mostrar somente essa categoria, consultamos esses índices.
// Sempre que adicionamos ou removemos um produto/tipo, chamamos esta função novamente para
// reconstruir a tabela e manter as informações consistentes.
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
// Esta função salva o catálogo (tipos, produtos, preços e estoques) em um arquivo de texto.
// Ela retorna 1 quando consegue concluir o salvamento e 0 quando ocorre algum problema.
//
// FILE *f é um ponteiro para uma estrutura FILE. Essa estrutura representa o arquivo aberto
// e mantém as informações necessárias para ler ou escrever nele. Não precisamos conhecer
// seus detalhes internos: usamos o ponteiro nas funções da biblioteca padrão.
//
// fopen(nome, modo) abre um arquivo. O modo "w" significa escrita: se o arquivo não existir,
// ele será criado; se já existir, seu conteúdo será apagado e escrito novamente.
// Aqui usamos primeiro um arquivo temporário (.tmp), em vez de sobrescrever diretamente o
// arquivo principal. Isso reduz o risco de deixar o arquivo principal pela metade se ocorrer
// um erro durante a gravação.
//
// Se fopen() falhar, ele retorna NULL. Nesse caso, perror() mostra uma mensagem junto da
// descrição do erro fornecida pelo sistema, e a função retorna 0.
//
// fprintf() funciona como printf(), mas escreve no arquivo indicado pelo primeiro argumento.
// A primeira linha identifica o formato do arquivo (MARCK1); a segunda registra quantos tipos
// e produtos existem. Em seguida gravamos os nomes dos tipos e, para cada produto, seu nome,
// o índice da categoria, o preço e o estoque.
//
// O valor de retorno de fprintf() é negativo quando há erro. Por isso guardamos em ok se a
// escrita funcionou. O trecho "&& ok" dos laços impede que continuemos tentando gravar depois
// que um erro já foi detectado. O formato %.9g escreve o preço usando até 9 algarismos
// significativos, evitando uma quantidade desnecessária de casas decimais.
//
// fclose() fecha o arquivo e também tenta concluir a gravação dos dados que ficaram em buffer.
// Por isso também verificamos seu retorno: um erro pode aparecer justamente no fechamento.
//
// Se algo falhar, removemos o arquivo temporário com remove() e informamos o erro.
// Se tudo der certo, rename() tenta trocar o arquivo temporário pelo arquivo definitivo.
// Em sistemas POSIX, a renomeação pode substituir diretamente o destino. No Windows, isso
// frequentemente falha quando o destino já existe; por isso o código tenta remover o arquivo
// antigo e renomear novamente. Depois de concluído, o arquivo passa a se chamar marck_dados.txt.
int salvarDados(){
    // O sufixo .tmp indica que esta gravação acontece primeiro em um arquivo temporário.
    FILE *f = fopen(ARQUIVO ".tmp", "w");
    if(f == NULL){ perror("Nao foi possivel salvar"); return 0; }
    // O cabeçalho permite reconhecer o formato e saber quantos registros deverão ser lidos.
    int ok = fprintf(f, "MARCK1\n%d %d\n", quantidadeTipos, quantidadeProdutos) >= 0;
    for(int t=0; t<quantidadeTipos && ok; t++)
        ok = fprintf(f, "%s\n", tipo[t]) >= 0;
    for(int i=0; i<quantidadeProdutos && ok; i++)
        ok = fprintf(f, "%s\n%d %.9g %d\n", produto[i], tipoPorProduto[i], preco[i], estoque[i]) >= 0;
    // Fechar o arquivo também é parte do salvamento: pode haver erro ao descarregar o buffer.
    if(fclose(f) != 0) ok = 0;
    if(!ok){ remove(ARQUIVO ".tmp"); puts("Erro ao escrever os dados."); return 0; }
    /* Renomeacao direta funciona no POSIX; no Windows, remove o arquivo antigo primeiro. */
    // Só tentamos colocar o arquivo temporário no lugar do definitivo depois de gravar tudo.
    if(rename(ARQUIVO ".tmp", ARQUIVO) != 0){
        remove(ARQUIVO);
        if(rename(ARQUIVO ".tmp", ARQUIVO) != 0){
            perror("Erro ao concluir salvamento"); return 0;
        }
    }
    return 1;
}

// Esta função faz o caminho inverso de salvarDados(): abre o arquivo e recupera os dados.
// O modo "r" significa leitura. Se o arquivo não existir, fopen() retorna NULL e a função
// devolve 0. Nesse caso, o programa continua usando os valores iniciais escritos no código.
//
// Antes de alterar os vetores globais, criamos vários vetores temporários (com o sufixo Temp).
// Isso é importante: se o arquivo estiver incompleto ou corrompido, não queremos misturar
// metade dos dados antigos com metade dos dados que conseguimos ler. Primeiro validamos tudo;
// somente depois copiamos os dados temporários para as variáveis globais.
//
// A assinatura MARCK1 identifica o formato esperado. Se futuramente mudarmos a organização
// do arquivo, poderemos usar outra assinatura para distinguir as versões.
// strcmp() compara duas strings: retorna 0 quando elas são iguais. Por isso a condição
// strcmp(assinatura, "MARCK1\n") != 0 significa que a assinatura não corresponde ao esperado.
//
// fscanf() lê dados formatados de um arquivo, assim como scanf() lê do teclado. Aqui esperamos
// exatamente dois inteiros: quantidade de tipos (nt) e quantidade de produtos (np). O retorno
// precisa ser 2, pois isso indica que as duas conversões foram feitas.
//
// Depois verificamos se as quantidades respeitam os limites dos vetores. Também não permitimos
// um arquivo com zero tipos e produtos cadastrados, porque todo produto precisa apontar para
// alguma categoria. Se qualquer validação falhar, usamos goto invalido para ir ao tratamento
// de erro que está no fim da função.
//
// Os nomes são lidos com fgets() para preservar os espaços. Depois calculamos o comprimento
// com strlen(), verificamos se a linha terminou com \n e substituímos essa quebra por \0.
// Também rejeitamos nomes vazios. A mesma lógica é usada para os nomes dos produtos.
//
// Para cada produto, fscanf() lê três valores: categoria, preço e estoque. A categoria precisa
// apontar para um tipo que exista (de 0 até nt-1); o preço precisa estar no intervalo aceito;
// e o estoque não pode ser negativo.
//
// Somente depois de todas essas leituras darem certo fechamos o arquivo e copiamos os dados
// temporários para as variáveis globais com strcpy() e atribuições simples.
// strcpy(destino, origem) copia o texto de uma string para outra. Como os vetores temporários
// têm os mesmos limites que os vetores definitivos e a leitura foi limitada por fgets(),
// os nomes cabem nos destinos.
//
// compra[i] recebe 0 porque o carrinho não deve ser restaurado de uma execução anterior:
// o arquivo salva o catálogo e o estoque, não um pedido que estava em andamento.
// Por fim, organizarItens() reconstrói os índices por categoria.
//
// A etiqueta invalido: é o destino do goto. Ela fecha o arquivo, avisa que os dados não são
// válidos e retorna 0. Como os dados globais ainda não tinham sido sobrescritos, o catálogo
// inicial pode continuar sendo utilizado.
int carregarDados(){
    FILE *f = fopen(ARQUIVO, "r");
    if(f == NULL) return 0;
    char assinatura[32];
    int nt, np;
    // Estes vetores temporários recebem o conteúdo do arquivo antes de tocar nos dados globais.
    char tiposTemp[MAX_TIPOS][30];
    char produtosTemp[MAX_PRODUTOS][100];
    int categoriasTemp[MAX_PRODUTOS], estoquesTemp[MAX_PRODUTOS];
    float precosTemp[MAX_PRODUTOS];
    // Se qualquer parte desta validação falhar, pulamos para o rótulo invalido no fim da função.
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

// Esta função cadastra uma nova categoria/tipo de produto.
// Primeiro verifica se já atingimos MAX_TIPOS. Se atingimos, não há espaço no vetor tipo[].
// Depois lemos o nome usando lerLinha(), que aceita espaços e protege o tamanho do vetor.
//
// nome[0] == \0 verifica se a string está vazia. Em C, uma string vazia tem o caractere
// terminador logo na primeira posição.
//
// O for percorre os tipos existentes e strcmp() verifica se o nome novo já está cadastrado.
// Lembre: strcmp() retorna 0 quando os textos são iguais. Se o nome ainda não existe,
// copiamos o texto para tipo[quantidadeTipos] e depois aumentamos a quantidade.
//
// organizarItens() atualiza as estruturas auxiliares. salvarDados() registra a alteração
// no arquivo para que a categoria continue existindo quando o programa for aberto novamente.
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

// Esta função cadastra um produto novo. Além do nome, agora precisamos registrar preço,
// estoque inicial e categoria. Por isso ela usa lerLinha(), lerPreco() e lerInteiro().
//
// Primeiro verificamos se há espaço no catálogo e se existe ao menos uma categoria para
// receber o produto. Depois lemos o nome, rejeitamos nome vazio e percorremos os produtos
// existentes para impedir cadastros duplicados. strcmp() faz a comparação dos nomes.
//
// lerPreco() recebe &valor, isto é, o endereço da variável valor. A função consegue então
// colocar o preço lido diretamente nessa variável. O mesmo acontece com lerInteiro() e qt.
//
// Em seguida mostramos os tipos disponíveis. A numeração mostrada começa em 1, enquanto
// tipo[] começa no índice 0. A última opção serve para cancelar o cadastro.
//
// Se o usuário escolher um tipo válido, usamos i = quantidadeProdutos como posição livre
// seguinte do vetor. Preenchemos todos os atributos desse novo produto no mesmo índice:
// nome, preço, estoque, carrinho e tipo. escolha-1 converte a opção humana para o índice C.
//
// Só depois de preencher os dados aumentamos quantidadeProdutos. Então reconstruímos os
// índices por categoria e salvamos o catálogo no arquivo.
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
// Esta função remove um produto a partir de seu índice global, isto é, a posição dele nos
// vetores produto[], preco[], estoque[], compra[] e tipoPorProduto[].
//
// Primeiro conferimos se o índice está dentro dos limites atuais. Depois deslocamos para a
// esquerda todos os elementos que vêm depois dele. Por exemplo, se removermos o índice 1,
// o produto do índice 2 passa para 1, o do índice 3 passa para 2 e assim por diante.
//
// É essencial deslocar todos os vetores relacionados, não somente produto[]. Caso contrário,
// o nome de um produto poderia ficar associado ao preço, estoque ou tipo de outro produto.
//
// Ao final, diminuímos quantidadeProdutos. A última posição passa a ficar fora da parte válida
// do catálogo e pode ser ignorada. Não é necessário apagar fisicamente o texto que sobrou nela.
// organizarItens() atualiza os índices auxiliares depois dessa mudança.
void deletaProduto(int indice){
    if(indice < 0 || indice >= quantidadeProdutos) return;
    // A posição removida é preenchida pelo elemento seguinte, mantendo os vetores alinhados.
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

// Esta função remove uma categoria. Como um produto não pode permanecer apontando para um tipo
// que deixou de existir, primeiro removemos todos os produtos pertencentes à categoria escolhida.
//
// O laço não incrementa i automaticamente. Quando encontra um produto da categoria removida,
// chama deletaProduto(i). Como os elementos seguintes são deslocados para a posição i, precisamos
// examinar essa mesma posição novamente. Por isso o i++ fica somente no else, quando não removemos.
//
// Depois deslocamos os nomes dos tipos para preencher a posição que foi removida e diminuímos
// quantidadeTipos. Mas ainda falta corrigir tipoPorProduto[]: se um produto apontava para um tipo
// posterior ao removido, seu índice precisa diminuir em 1, porque os tipos também foram deslocados.
//
// Por exemplo, se o tipo de índice 1 for removido, o antigo tipo 2 passa a ser o tipo 1.
// Portanto, todo produto cujo tipoPorProduto[i] era maior que 1 precisa ter esse valor reduzido.
// No final, organizamos novamente a tabela de índices por categoria.
void deletaTipo(int indice){
    if(indice < 0 || indice >= quantidadeTipos) return;
    // Não colocamos i++ no cabeçalho porque, ao excluir, outro produto ocupa a mesma posição.
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

// Esta função é a interface de exclusão de produtos. Ela conversa com o usuário, mas quem
// efetivamente desloca os vetores e remove o produto é deletaProduto().
//
// Primeiro mostramos cada produto junto de seu tipo. O índice exibido é i+1, como no código
// menor: o usuário escolhe começando em 1, mas os vetores começam em 0.
// A opção quantidadeProdutos+1 cancela a operação. Depois validamos a escolha e pedimos
// confirmação antes de chamar deletaProduto(escolha-1).
//
// A separação entre excluirProdutoMenu() e deletaProduto() é útil porque cada função tem
// uma responsabilidade: uma lida com o menu; a outra modifica a estrutura de dados.
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

// Esta função é semelhante ao menu de exclusão de produtos, mas agora trabalha com tipos.
// A listagem mostra também produtosPorTipo[t], que informa quantos produtos serão afetados.
//
// A exclusão de um tipo é mais drástica: deletaTipo() também remove todos os produtos que
// pertencem a ele. Por isso o programa avisa quantos produtos serão apagados e pede confirmação.
// Depois de confirmar, convertemos escolha para índice com escolha-1, chamamos deletaTipo() e
// salvamos o catálogo atualizado.
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

// Esta função permite substituir a quantidade total disponível de um produto.
// O do-while(1) cria um menu que se repete indefinidamente até acontecer um return.
// O return encerra a função e volta para quem a chamou; continue encerra somente a repetição
// atual e começa uma nova volta do laço.
//
// Para cada produto mostramos seu estoque atual. A última opção permite voltar ao menu anterior.
// lerInteiro() devolve um status, que usamos para diferenciar fim da entrada (0) de erro de
// digitação (-1). Assim, se a entrada for inválida, mostramos a mensagem e continuamos o menu.
//
// Depois de escolher um produto, lemos o novo estoque total e impedimos valores negativos.
// escolha-1 transforma a opção exibida em índice do vetor estoque[]. Em seguida, salvamos
// os dados para que a atualização não seja perdida ao fechar o programa.
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

// Esta função mostra a lista de categorias disponíveis para o cliente.
// O laço imprime os tipos cadastrados, numerando-os de 1 até quantidadeTipos.
// A opção seguinte, quantidadeTipos+1, significa finalizar o pedido.
void mostraTipo(){
    puts("\n===== TIPOS =====");
    for(int i=0; i<quantidadeTipos; i++) printf("%d - %s\n", i+1, tipo[i]);
    printf("%d - Finalizar pedido\n", quantidadeTipos+1);
}

// Esta função mostra os produtos de uma categoria específica.
// O parâmetro t é a opção escolhida pelo usuário, portanto começa em 1. Para acessar os
// vetores de categorias, precisamos usar t-1, que é o índice correspondente em C.
//
// O laço não percorre todos os produtos globais: ele percorre somente produtosPorTipo[t-1],
// isto é, a quantidade de produtos daquela categoria. A variável j recebe o índice global
// do produto consultado na matriz auxiliar indicePorTipo. Depois usamos j para encontrar
// nome, preço, quantidade no carrinho e estoque nos vetores originais.
//
// Perceba a diferença entre i e j: i é a posição do produto dentro da categoria que está sendo
// mostrada; j é a posição real desse produto nos vetores produto[], preco[] etc.
// A opção final permite voltar à lista de categorias.
void mostraProduto(int t){
    printf("\n===== %s =====\n", tipo[t-1]);
    for(int i=0; i<produtosPorTipo[t-1]; i++){
        int j = indicePorTipo[t-1][i];
        printf("%d - %s | R$%.2f | Carrinho: %d | Estoque: %d\n",
               i+1, produto[j], preco[j], compra[j], estoque[j]);
    }
    printf("%d - Voltar aos tipos\n", produtosPorTipo[t-1]+1);
}

// Esta função altera a quantidade de um produto no carrinho.
// Ela recebe t, a categoria selecionada pelo usuário, e i, a posição do produto dentro daquela
// categoria. Ambos são recebidos na contagem que começa em 1.
//
// Para localizar o produto nos vetores globais, usamos indicePorTipo[t-1][i-1]. Assim, primeiro
// convertemos a categoria para índice (t-1), depois convertemos a opção dentro da categoria (i-1).
// O resultado j é o índice global do produto.
//
// A quantidade qt pode ser positiva para adicionar itens ou negativa para remover itens do
// carrinho. A expressão compra[j] + qt calcula a nova quantidade sem alterar imediatamente
// o valor antigo. Usamos long para fazer a soma em um tipo inteiro de maior capacidade e reduzir
// o risco de overflow durante a operação.
//
// Se nova estiver entre 0 e o estoque disponível, atualizamos compra[j]. Caso contrário,
// não modificamos o carrinho e informamos que a quantidade está fora do limite.
void adicionarAoPedido(int t, int i){
    int j = indicePorTipo[t-1][i-1];
    int qt;
    int status = lerInteiro("Quantidade (negativa para remover do carrinho): ", &qt);
    if(status != 1){ puts("Quantidade invalida."); return; }
    // A conversão para long acontece antes da soma, para calcular a nova quantidade em tipo maior.
    long nova = (long)compra[j] + qt;
    if(nova >= 0 && nova <= estoque[j]){
        compra[j] = (int)nova;
        puts("Pedido atualizado!");
    }else puts("Quantidade fora do limite.");
}

// Esta função lê qual produto o cliente escolheu dentro de uma categoria.
// A validação precisa considerar produtosPorTipo[t-1], porque cada categoria pode ter uma
// quantidade diferente de produtos. Se a escolha estiver entre 1 e essa quantidade, chamamos
// adicionarAoPedido(). A opção seguinte significa voltar à lista de categorias, então não
// fazemos nada nesse caso. Qualquer outro número é inválido.
void pedirProduto(int t){
    int escolha;
    int status = lerInteiro("Escolha um produto: ", &escolha);
    if(status != 1){ puts("Opcao invalida."); return; }
    if(escolha >= 1 && escolha <= produtosPorTipo[t-1]) adicionarAoPedido(t, escolha);
    else if(escolha != produtosPorTipo[t-1]+1) puts("Opcao invalida.");
}

/* Procura o maior ID registrado para continuar a numeracao apos reiniciar. */
// Esta função procura o maior número de nota que já foi registrado no histórico e devolve o
// próximo número. Assim, a numeração não volta para 1 toda vez que o programa é reiniciado.
//
// unsigned long é um tipo inteiro sem sinal, adequado aqui porque o identificador não deve
// ser negativo e pode crescer ao longo do tempo.
//
// Abrimos o histórico no modo "r", somente para leitura. Se o arquivo ainda não existir,
// não há notas anteriores, então o próximo ID será 1.
//
// fgets() percorre o arquivo linha por linha. Para cada linha, sscanf() tenta encontrar o
// padrão do cabeçalho de uma nota e extrair o número que aparece depois de #.
// O formato aceita o número mesmo que ele tenha zeros à esquerda. Se encontrarmos um ID maior
// que o maior guardado até então, atualizamos maior. No final, devolvemos maior+1.
unsigned long proximoIdNota(){
    // Aqui só consultamos o histórico; não queremos alterar seu conteúdo.
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
// Esta função monta o comprovante da venda. O detalhe interessante é que ela não escolhe
// sozinha onde escrever: recebe um FILE *destino.
//
// Isso permite reaproveitar a mesma função para dois destinos diferentes. Quando passarmos
// historico, a nota será escrita no arquivo; quando passarmos stdout, será escrita no terminal.
// stdout é a saída padrão do programa, normalmente a tela. Portanto, não precisamos duplicar
// todo o código de montagem da nota.
//
// fprintf() recebe o destino como primeiro argumento e o restante funciona como printf().
// O especificador %06lu imprime o ID como unsigned long, usando no mínimo 6 algarismos e
// completando com zeros à esquerda. Por exemplo, o ID 7 aparece como 000007.
//
// O laço percorre o catálogo e só imprime os produtos cuja quantidade no carrinho é maior que 0.
// A linha seguinte mostra quantidade, preço unitário e subtotal. No fim, mostramos o total.
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

// Esta função é responsável por concluir uma venda: calcula o total, registra a nota no
// histórico, mostra o comprovante e desconta os produtos do estoque.
//
// Primeiro percorremos os produtos. itens soma as quantidades do carrinho; total soma
// quantidade * preço de cada produto. Se itens for 0, o carrinho está vazio e não há venda.
//
// Depois pedimos o próximo ID e abrimos o arquivo de vendas no modo "a" (append/acrescentar).
// Diferentemente do modo "w", o modo "a" não apaga o conteúdo anterior: cada nova nota é
// colocada ao final do arquivo, preservando o histórico das vendas passadas.
//
// Se fopen() falhar, não devemos descontar o estoque nem considerar a venda concluída.
// perror() informa o erro do sistema. Se a abertura funcionar, escreverNota() grava o
// comprovante no histórico. ferror() verifica se houve erro durante a escrita, e fclose()
// também é verificado porque pode detectar falhas ao concluir a gravação.
//
// Somente depois de confirmar que o registro foi escrito, chamamos escreverNota(stdout, ...)
// para exibir a mesma nota na tela. Então descontamos compra[i] de estoque[i] e zeramos
// compra[i], pois o pedido foi concluído.
//
// Por último, salvarDados() grava o estoque atualizado no arquivo do catálogo. Repare que
// o histórico de vendas e o catálogo são arquivos diferentes: um guarda as notas; o outro
// guarda os dados atuais dos produtos e estoques.
int imprimirNota(){
    float total = 0;
    int itens = 0;
    for(int i=0; i<quantidadeProdutos; i++){
        itens += compra[i];
        total += compra[i] * preco[i];
    }
    if(itens == 0){ puts("Carrinho vazio. Nenhuma venda registrada."); return 0; }

    unsigned long id = proximoIdNota();
    // O modo a acrescenta a nova nota ao final do arquivo, sem apagar as notas anteriores.
    FILE *historico = fopen(ARQUIVO_VENDAS, "a");
    if(historico == NULL){
        perror("Nao foi possivel abrir o registro de vendas");
        puts("Venda nao finalizada. Carrinho e estoque preservados.");
        return 0;
    }
    // Primeiro registramos no histórico. Só depois de verificar a escrita é que alteramos o estoque.
    escreverNota(historico, id, total);
    int erroEscrita = ferror(historico);
    if(fclose(historico) != 0) erroEscrita = 1;
    if(erroEscrita){
        puts("Erro ao registrar a venda. Verifique o historico antes de tentar novamente.");
        return 0;
    }
    // stdout representa a saída padrão (normalmente o terminal), então a mesma nota aparece na tela.
    escreverNota(stdout, id, total);
    for(int i=0; i<quantidadeProdutos; i++){
        estoque[i] -= compra[i];
        compra[i] = 0;
    }
    if(!salvarDados()) puts("ATENCAO: venda registrada, mas nao foi possivel salvar o estoque.");
    else printf("Venda #%06lu registrada em '%s'.\n", id, ARQUIVO_VENDAS);
    return 1;
}

// Esta função organiza o fluxo inteiro de um pedido. Primeiro verifica se há categorias e
// produtos disponíveis. Depois entra em um while(1), que mantém o cliente navegando entre
// categorias até ele escolher finalizar o pedido ou ocorrer o encerramento da entrada.
//
// Em cada repetição mostramos as categorias e lemos a escolha. Se lerInteiro() retornar 0,
// a entrada terminou e cancelamos o pedido. Se retornar algo diferente de 1, a entrada foi
// inválida e usamos continue para voltar ao começo do laço.
//
// Quando o usuário escolhe quantidadeTipos+1, perguntamos se deseja finalizar a venda ou
// cancelar. Se confirmar, chamamos imprimirNota(); caso contrário, apenas informamos o
// cancelamento. Em seguida, break encerra o while.
//
// Se a escolha for uma categoria existente, mostramos os produtos dela e pedimos um produto.
// Ao sair do laço, o for final limpa o carrinho inteiro.
// ATENÇÃO: do jeito que o código está, essa limpeza acontece também quando imprimirNota()
// falha. Portanto, apesar de a mensagem dentro de imprimirNota() dizer que o carrinho foi
// preservado, este for zera as quantidades logo depois que a função retorna. É um detalhe
// importante para corrigirmos depois, caso a intenção seja realmente preservar o carrinho
// quando não for possível registrar a venda.
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
    // Limpa todos os itens do carrinho ao terminar o fluxo de pedido; veja a observação acima
    // sobre o efeito disso quando o registro da nota falha.
    for(int i=0; i<quantidadeProdutos; i++) compra[i] = 0;
}

// Esta função apenas imprime o desenho da MARCK no terminal. Não altera nenhuma variável
// nem participa da lógica de cadastro, pedido ou salvamento.
//
// Como printf() interpreta a barra invertida como início de sequências especiais, precisamos
// escrever \\ para que apareça uma única barra invertida no terminal. Já \n representa
// uma quebra de linha.
void logo(){
    printf(" __  __     _     ____    ____  _  __\n");
    printf("|  \\/  |   / \\   |  _ \\  / ___|| |/ /\n");
    printf("| |\\/| |  / _ \\  | |_) || |    | ' / \n");
    printf("| |  | | / ___ \\ |  _ < | |___ |   \\ \n");
    printf("|_|  |_|/_/   \\_\\|_| \\_\\ \\____||_|\\_\\\n\n");
}

// Este é o menu responsável pelas alterações no catálogo e no estoque.
// O while(1) faz o menu reaparecer depois de cada operação, até o usuário escolher voltar.
//
// printf() mostra quantos produtos e tipos estão cadastrados e qual é o limite de cada vetor.
// Depois lemos a escolha. Se a entrada acabou, return encerra a função; se for inválida,
// continue reinicia o laço e mostra o menu novamente.
//
// switch(escolha) seleciona qual bloco executar com base no valor de escolha. Cada case chama
// uma função específica. break impede que a execução continue para o próximo case.
// No case 6 usamos return porque queremos sair do menu de edição e voltar à função chamadora.
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

// Este é o menu principal do programa. Ele oferece fazer um pedido, editar o catálogo ou sair.
// Assim como o menu de edição, usa while(1) para continuar funcionando até uma opção encerrar
// a função. Cada case encaminha o trabalho para a função responsável por aquela atividade.
//
// Ao sair, salvarDados() é chamado para garantir que as alterações feitas no catálogo e no
// estoque sejam gravadas. O registro de vendas é atualizado separadamente quando uma venda
// é concluída.
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

// A função main() é o ponto de entrada do programa: é por ela que a execução começa.
// Mantivemos aqui somente a organização geral, deixando o trabalho específico para as outras
// funções. Primeiro tentamos carregar os dados salvos anteriormente. Se carregarDados() retornar
// 1, mostramos que o arquivo foi lido; se retornar 0, permanecem os dados iniciais declarados
// no começo do código (ou o catálogo inicial, caso o arquivo esteja inválido).
//
// Depois organizamos os índices por categoria, imprimimos a logo, chamamos o menu principal
// e, quando ele termina, devolvemos 0 ao sistema operacional para indicar encerramento normal.
int main(void){
    if(carregarDados()) puts("Dados carregados do arquivo.");
    organizarItens();
    logo();
    decisaoInicial();
    return 0;
}

// Agora podemos observar o int main() e entender como ele organiza a execução
// do programa inteiro.
//
// Primeiro executamos:
//
// carregarDados();
//
// Essa função procura o arquivo onde os dados do catálogo foram salvos anteriormente.
// Se encontrar um arquivo válido, recupera os produtos, tipos, preços e estoques registrados.
// Caso contrário, o programa continua utilizando os dados iniciais declarados no começo do código.
//
// Depois executamos:
//
// organizarItens();
//
// Essa função organiza os índices dos produtos dentro de seus respectivos tipos.
// Assim, conseguimos mostrar os produtos separados por categoria sem precisar mudar
// a posição original deles nos vetores.
//
// Em seguida executamos:
//
// logo();
//
// Essa função imprime a logo da MARCK's Softwares no terminal.
//
// Finalmente executamos:
//
// decisaoInicial();
//
// Essa função apresenta o menu principal e permite que o usuário escolha entre
// realizar um pedido, editar o catálogo ou salvar os dados e sair do programa.
//
// Podemos enxergar uma versão simplificada da estrutura geral do programa assim:
//
// main()
// |
// +-- carregarDados()
// |
// +-- organizarItens()
// |
// +-- logo()
// |
// +-- decisaoInicial()
//       |
//       +-- iniciarPedido()
//       |     |
//       |     +-- mostraTipo()
//       |     |
//       |     +-- mostraProduto()
//       |     |
//       |     +-- pedirProduto()
//       |     |     |
//       |     |     +-- adicionarAoPedido()
//       |     |
//       |     +-- imprimirNota()
//       |           |
//       |           +-- proximoIdNota()
//       |           |
//       |           +-- escreverNota()
//       |
//       +-- editarMenu()
//             |
//             +-- cadastrarProduto()
//             |
//             +-- registraTipo()
//             |
//             +-- excluirProdutoMenu()
//             |     |
//             |     +-- deletaProduto()
//             |
//             +-- excluirTipoMenu()
//             |     |
//             |     +-- deletaTipo()
//             |
//             +-- atualizaEstoque()
//
// Essa árvore mostra as principais relações entre as funções, mas não representa
// absolutamente todas as chamadas do programa. Por exemplo, várias dessas funções
// também utilizam lerInteiro(), lerLinha() e salvarDados() para validar entradas
// e registrar as alterações nos arquivos.
//
// Isso permite dividir um problema grande em vários problemas menores.
// Cada função possui uma responsabilidade específica e pode chamar outras funções
// quando necessário. Assim, não precisamos escrever todo o funcionamento do programa
// dentro de int main(). Basta organizar a sequência geral e deixar que cada função
// execute a tarefa que lhe foi atribuída.
//
// Uma diferença importante em relação ao código anterior é que agora o programa não
// trabalha apenas com uma lista única de produtos. Temos também tipos (categorias),
// e cada produto está associado a um deles através do vetor tipoPorProduto[].
//
// A matriz indicePorTipo[][] permite relacionar a posição de um produto dentro de seu
// tipo com o índice original desse mesmo produto nos vetores produto[], preco[],
// estoque[] e compra[]. Dessa maneira, conseguimos apresentar os produtos separados
// por categoria sem precisar reorganizar os próprios vetores.
//
// Além disso, agora os dados podem ser salvos em arquivos e recuperados quando o programa
// for iniciado novamente, e as vendas concluídas são registradas em um histórico próprio.
