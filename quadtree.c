#include "quadtree.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifdef __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h> /* OpenGL functions */
#endif

float RED_FACTOR = 0.3;
float GREEN_FACTOR = 0.59;
float BLUE_FACTOR = 0.11;

unsigned int first = 1;
char desenhaBorda = 1;

QuadNode *newNode(int x, int y, int width, int height)
{
    QuadNode *n = malloc(sizeof(QuadNode));
    n->x = x;
    n->y = y;
    n->width = width;
    n->height = height;
    n->NW = n->NE = n->SW = n->SE = NULL;
    n->color[0] = n->color[1] = n->color[2] = 0;
    n->id = first++;
    return n;
}

// chamada recursiva para desenhar a quadtree

QuadNode *desenhaQuadtree(QuadNode *n, float minError, Img *pic)
{
    // printf("ID: %d \n", n->id);

    //transform img into grayscale

    RGBPixel(*pixels)[pic->width] = (RGBPixel(*)[pic->height])pic->img; 

    int i,j;

    if (n == NULL)
        return NULL;

    if (minError == 0)
    {
        return newNode(0, 0, n->width, n->height);
    }

    float meiaLargura = n->width / 2;
    float meiaAltura = n->height / 2;

    unsigned int totalPixels = n->width * n->height;

    // calcula a cor média da região

    int mediaR = 0;
    int mediaG = 0;
    int mediaB = 0;

    for (i = n->x; i < n->x + n->width; i++)
    {
        for (j = n->y; j < n->y + n->height; j++)
        {

            mediaR += pixels[i][j].r;
            mediaG += pixels[i][j].g;
            mediaB += pixels[i][j].b;
        }
    }

    mediaR = mediaR / totalPixels;
    mediaG = mediaG / totalPixels;
    mediaB = mediaB / totalPixels;

    // calcula o histograma da região

    unsigned int histogram[256];

    for (i = 0; i < 256; i++)
    {
        histogram[i] = 0;
    }

    for (i = n->x; i < n->x + n->width; i++)
    {
        for (j = n->y; j < n->y + n->height; j++)
        {

            unsigned int intensity = (pixels[i][j].r * RED_FACTOR) + (pixels[i][j].g * GREEN_FACTOR) + (pixels[i][j].b * BLUE_FACTOR);

            histogram[intensity] += 1;
        }
    }

    // calcula nível de erro da região

    // calcula intensidade média do quadrante utilizando o histograma

    unsigned int soma = 0;

    for (i = 0; i < 256; i++)
    {
        soma += histogram[i] * i;
    }

    int intensidadeMedia = soma / totalPixels;

    n->color[0] = mediaR;
    n->color[1] = mediaG;
    n->color[2] = mediaB;
    // Calculo do erro conforme fórmula da seção 3.3
    long double erro = 0;

    for (i = 0; i < n->width; i++)
    {
        for (j = 0; j < n->height; j++)
        {
            unsigned int intensity = (pixels[i][j].r * RED_FACTOR) + (pixels[i][j].g * GREEN_FACTOR) + (pixels[i][j].b * BLUE_FACTOR);
            erro += pow((intensity - intensidadeMedia), 2);
        }
    }

    long double totalPixelsDividido = 1.00000000 / totalPixels;

    long double totalPixelsVezesErro = totalPixelsDividido * erro;

    long double erroRegiao = sqrt(totalPixelsVezesErro);

    // printf("erro regiao: %Lf \n", erroRegiao);
    if (erro < minError)
    {
        // printf("Chegou ao erro minimo! %Lf  minErro %f \n", erro, minError);
        n->status = CHEIO;
        return n;
    }
    else
    {
        n->status = PARCIAL;
        n->NW = desenhaQuadtree(newNode(n->x, n->y, meiaLargura, meiaAltura), minError, pic);
        n->NE = desenhaQuadtree(newNode(n->x + meiaLargura, n->y, meiaLargura, meiaAltura), minError, pic);
        n->SW = desenhaQuadtree(newNode(n->x, n->y + meiaAltura, meiaLargura, meiaAltura), minError, pic);
        n->SE = desenhaQuadtree(newNode(n->x + meiaLargura, n->y + meiaAltura, meiaLargura, meiaAltura), minError, pic);
    }

    return n;
}

QuadNode *geraQuadtree(Img *pic, float minError)
{
    // Converte o vetor RGBPixel para uma MATRIZ que pode acessada por pixels[linha][coluna]
    RGBPixel(*pixels)[pic->width] = (RGBPixel(*)[pic->height])pic->img;

    // Veja como acessar os primeiros 10 pixels da imagem, por exemplo:
    int i, j;
    for (i = 0; i < 10; i++)
        printf("%02X %02X %02X\n", pixels[0][i].r, pixels[1][i].g, pixels[2][i].b);

    int width = pic->width;
    int height = pic->height;
    unsigned int totalPixels = width * height;

    //////////////////////////////////////////////////////////////////////////
    // Implemente aqui o algoritmo que gera a quadtree, retornando o nodo raiz
    //////////////////////////////////////////////////////////////////////////

    // Converter imagem para tons de cinza

    Img *grayImg = malloc(sizeof(Img));

    grayImg->width = pic->width;
    grayImg->height = pic->height;

    grayImg->img = malloc(sizeof(RGBPixel) * grayImg->width * grayImg->height);

    RGBPixel(*graypixels)[grayImg->width] = (RGBPixel(*)[grayImg->height])grayImg->img;


    for (i = 0; i < grayImg->width; i++)
    {
        for (j = 0; j < grayImg->height; j++)
        {
            unsigned int intensity = (pixels[i][j].r * RED_FACTOR) + (pixels[i][j].g * GREEN_FACTOR) + (pixels[i][j].b * BLUE_FACTOR);

            //printf("intensity: %d \n", intensity);
            graypixels[i][j].r = intensity;
            graypixels[i][j].g = intensity;
            graypixels[i][j].b = intensity;
        }
    }
    
    QuadNode *raiz = desenhaQuadtree(newNode(0, 0, width, height), minError, grayImg);

    return raiz;
}

// COMENTE a linha abaixo quando seu algoritmo ja estiver funcionando
// Caso contrario, ele ira gerar uma arvore de teste com 3 nodos
// #define DEMO
#ifdef DEMO

/************************************************************/
/* Teste: criando uma raiz e dois nodos a mais              */
/************************************************************/

QuadNode *raiz = newNode(0, 0, width, height);
raiz->status = PARCIAL;
raiz->color[0] = 0;
raiz->color[1] = 0;
raiz->color[2] = 255;

int meiaLargura = width / 2;
int meiaAltura = height / 2;

QuadNode *nw = newNode(meiaLargura, 0, meiaLargura, meiaAltura);
nw->status = PARCIAL;
nw->color[0] = 0;
nw->color[1] = 0;
nw->color[2] = 255;

// Aponta da raiz para o nodo nw
raiz->NW = nw;

QuadNode *nw2 = newNode(meiaLargura + meiaLargura / 2, 0, meiaLargura / 2, meiaAltura / 2);
nw2->status = CHEIO;
nw2->color[0] = 255;
nw2->color[1] = 0;
nw2->color[2] = 0;

// Aponta do nodo nw para o nodo nw2
nw->NW = nw2;

#endif
// Finalmente, retorna a raiz da árvore
// return raiz
// Limpa a memória ocupada pela árvore
void clearTree(QuadNode *n)
{
    if (n == NULL)
        return;
    if (n->status == PARCIAL)
    {
        clearTree(n->NE);
        clearTree(n->NW);
        clearTree(n->SE);
        clearTree(n->SW);
    }
    // printf("Liberando... %d - %.2f %.2f %.2f %.2f\n", n->status, n->x, n->y, n->width, n->height);
    free(n);
}

// Ativa/desativa o desenho das bordas de cada região
void toggleBorder()
{
    desenhaBorda = !desenhaBorda;
    printf("Desenhando borda: %s\n", desenhaBorda ? "SIM" : "NÃO");
}

// Desenha toda a quadtree
void drawTree(QuadNode *raiz)
{
    if (raiz != NULL)
        drawNode(raiz);
}

// Grava a árvore no formato do Graphviz
void writeTree(QuadNode *raiz)
{
    FILE *fp = fopen("quad.dot", "w");
    fprintf(fp, "digraph quadtree {\n");
    if (raiz != NULL)
        writeNode(fp, raiz);
    fprintf(fp, "}\n");
    fclose(fp);
    printf("\nFim!\n");
}

void writeNode(FILE *fp, QuadNode *n)
{
    if (n == NULL)
        return;

    if (n->NE != NULL)
        fprintf(fp, "%d -> %d;\n", n->id, n->NE->id);
    if (n->NW != NULL)
        fprintf(fp, "%d -> %d;\n", n->id, n->NW->id);
    if (n->SE != NULL)
        fprintf(fp, "%d -> %d;\n", n->id, n->SE->id);
    if (n->SW != NULL)
        fprintf(fp, "%d -> %d;\n", n->id, n->SW->id);
    writeNode(fp, n->NE);
    writeNode(fp, n->NW);
    writeNode(fp, n->SE);
    writeNode(fp, n->SW);
}

// Desenha todos os nodos da quadtree, recursivamente
void drawNode(QuadNode *n)
{
    if (n == NULL)
        return;

    glLineWidth(0.1);

    if (n->status == CHEIO)
    {
        glBegin(GL_QUADS);
        glColor3ubv(n->color);
        glVertex2f(n->x, n->y);
        glVertex2f(n->x + n->width - 1, n->y);
        glVertex2f(n->x + n->width - 1, n->y + n->height - 1);
        glVertex2f(n->x, n->y + n->height - 1);
        glEnd();
    }

    else if (n->status == PARCIAL)
    {
        if (desenhaBorda)
        {
            glBegin(GL_LINE_LOOP);
            glColor3ubv(n->color);
            glVertex2f(n->x, n->y);
            glVertex2f(n->x + n->width - 1, n->y);
            glVertex2f(n->x + n->width - 1, n->y + n->height - 1);
            glVertex2f(n->x, n->y + n->height - 1);
            glEnd();
        }
        drawNode(n->NE);
        drawNode(n->NW);
        drawNode(n->SE);
        drawNode(n->SW);
    }
    // Nodos vazios não precisam ser desenhados... nem armazenados!
}
