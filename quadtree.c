#include "quadtree.h"
#include <stdio.h>
#include <stdlib.h>

#ifdef __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>     /* OpenGL functions */
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

QuadNode *geraQuadtree(Img* pic, float minError)
{
    // Converte o vetor RGBPixel para uma MATRIZ que pode acessada por pixels[linha][coluna]
    RGBPixel (*pixels)[pic->width] = (RGBPixel(*)[pic->height]) pic->img;

    // Veja como acessar os primeiros 10 pixels da imagem, por exemplo:
    int i,j;
    for(i=0; i<10; i++)
        printf("%02X %02X %02X\n",pixels[0][i].r,pixels[1][i].g,pixels[2][i].b);

    int width = pic->width;
    int height = pic->height;

    //////////////////////////////////////////////////////////////////////////
    // Implemente aqui o algoritmo que gera a quadtree, retornando o nodo raiz
    //////////////////////////////////////////////////////////////////////////
    
    GrayscaleImg *p = malloc(sizeof(GrayscaleImg) * (width * height));
    
    for (i=0; i<height; i++)
    {
        for(j=0; j<width; j++)
        {
            
            printf("\nBLUE %d \n RED %d \n GREEN %d\n", pic->img->b, pic->img->r, pic->img->g);
            char intensity = (char)(pic->img->b * BLUE_FACTOR) + (pic->img->r * RED_FACTOR) + (pic->img->g * GREEN_FACTOR);

            printf("\nGRAY: %d\n", intensity);

            p[j].grayPix = &intensity;

        }
    }
// COMENTE a linha abaixo quando seu algoritmo ja estiver funcionando
// Caso contrario, ele ira gerar uma arvore de teste com 3 nodos

#define DEMO
#ifdef DEMO

    /************************************************************/
    /* Teste: criando uma raiz e dois nodos a mais              */
    /************************************************************/

    QuadNode *raiz = newNode(0,0,width,height);
    raiz->status = PARCIAL;
    raiz->color[0] = 0;
    raiz->color[1] = 0;
    raiz->color[2] = 255;

    int meiaLargura = width/2;
    int meiaAltura = height/2;

    QuadNode *nw = newNode(meiaLargura, 0, meiaLargura, meiaAltura);
    nw->status = PARCIAL;
    nw->color[0] = 0;
    nw->color[1] = 0;
    nw->color[2] = 255;

    // Aponta da raiz para o nodo nw
    raiz->NW = nw;

    QuadNode *nw2 = newNode(meiaLargura+meiaLargura/2, 0, meiaLargura/2, meiaAltura/2);
    nw2->status = CHEIO;
    nw2->color[0] = 255;
    nw2->color[1] = 0;
    nw2->color[2] = 0;

    // Aponta do nodo nw para o nodo nw2
    nw->NW = nw2;

#endif
    // Finalmente, retorna a raiz da árvore
    return raiz;
}

// Limpa a memória ocupada pela árvore
void clearTree(QuadNode *n)
{
    if(n == NULL) return;
    if(n->status == PARCIAL)
    {
        clearTree(n->NE);
        clearTree(n->NW);
        clearTree(n->SE);
        clearTree(n->SW);
    }
    //printf("Liberando... %d - %.2f %.2f %.2f %.2f\n", n->status, n->x, n->y, n->width, n->height);
    free(n);
}

// Ativa/desativa o desenho das bordas de cada região
void toggleBorder() {
    desenhaBorda = !desenhaBorda;
    printf("Desenhando borda: %s\n", desenhaBorda ? "SIM" : "NÃO");
}

// Desenha toda a quadtree
void drawTree(QuadNode *raiz) {
    if(raiz != NULL)
        drawNode(raiz);
}

// Grava a árvore no formato do Graphviz
void writeTree(QuadNode *raiz) {
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
    if(n == NULL) return;

    if(n->NE != NULL) fprintf(fp, "%d -> %d;\n", n->id, n->NE->id);
    if(n->NW != NULL) fprintf(fp, "%d -> %d;\n", n->id, n->NW->id);
    if(n->SE != NULL) fprintf(fp, "%d -> %d;\n", n->id, n->SE->id);
    if(n->SW != NULL) fprintf(fp, "%d -> %d;\n", n->id, n->SW->id);
    writeNode(fp, n->NE);
    writeNode(fp, n->NW);
    writeNode(fp, n->SE);
    writeNode(fp, n->SW);
}

// Desenha todos os nodos da quadtree, recursivamente
void drawNode(QuadNode *n)
{
    if(n == NULL) return;

    glLineWidth(0.1);

    if(n->status == CHEIO) {
        glBegin(GL_QUADS);
        glColor3ubv(n->color);
        glVertex2f(n->x, n->y);
        glVertex2f(n->x+n->width-1, n->y);
        glVertex2f(n->x+n->width-1, n->y+n->height-1);
        glVertex2f(n->x, n->y+n->height-1);
        glEnd();
    }

    else if(n->status == PARCIAL)
    {
        if(desenhaBorda) {
            glBegin(GL_LINE_LOOP);
            glColor3ubv(n->color);
            glVertex2f(n->x, n->y);
            glVertex2f(n->x+n->width-1, n->y);
            glVertex2f(n->x+n->width-1, n->y+n->height-1);
            glVertex2f(n->x, n->y+n->height-1);
            glEnd();
        }
        drawNode(n->NE);
        drawNode(n->NW);
        drawNode(n->SE);
        drawNode(n->SW);
    }
    // Nodos vazios não precisam ser desenhados... nem armazenados!
}

