#include "lodepng.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>


typedef struct pix{
    unsigned char r, g, b, a;
    struct pix *left, *right, *up, *down, *parent;
    double colorr, colorg, colorb;
    int num, size;
}pix;

typedef struct lst_el{
    struct lst_el *next;
    pix *i;
} lst_el;

typedef struct list{
    lst_el *beg;
    int size;
}list;

void insert_beg(pix *i, list *list){
    lst_el *new_el = malloc(sizeof(lst_el));
    if (new_el == NULL) {
        perror("Ошибка выделения памяти");
        exit(1); // Или обработайте ошибку другим способом
    }
    new_el -> next = list -> beg;
    new_el -> i = i;
    list -> beg = new_el;
    return;
}

pix *Find_Set(pix *i){
    if(i -> parent == i) return i;
    else return i -> parent = Find_Set(i -> parent);
}

void Union(pix *el1, pix *el2){
    el1 = Find_Set(el1);
    el2 = Find_Set(el2);
    if(el1 != el2) {
        el2 -> parent = el1;
        el2 -> colorr = el1 -> colorr;
        el2 -> colorg = el1 -> colorg;
        el2 -> colorb = el1 -> colorb;
        //printf("%d", el1 -> size);
        el1 -> size++;
        el2 -> size++;
    }
    return;
}

int i, j;

//
char* load_png_file  (const char *filename, int *width, int *height) {
    unsigned char *image = NULL;
    int error = lodepng_decode32_file(&image, width, height, filename);
    if (error) {
        printf("error %u: %s\n", error, lodepng_error_text(error));
        return NULL;
    }

    return (image);
}

void write_png_file(const char *filename, const unsigned char *image, unsigned width, unsigned height) {
    unsigned char* png;
    size_t pngsize;
    int error = lodepng_encode32(&png, &pngsize, image, width, height);
    if (!error) {
        lodepng_save_file(png, pngsize, filename);
    }
    if (error)
        printf("error %u: %s\n", error, lodepng_error_text(error));
    free(png);
}

typedef struct pixel {
    char R;
    char G;
    char B;
    char alpha;
}pixel;

void before(unsigned char *pixels, int w, int h) {
    for(i = 3; i < w * h * 4; i+=4) {
        if(pixels[i] < 30) pixels[i] = 0;
        if(pixels[i] > 225) pixels[i] = 255;
    }
    return;
}

void components(int w, int h, unsigned char *image) {
    int n, E, x, y;
    n = w * h;

    srand(time(NULL));
    pix *pixels = malloc(sizeof(pix) * n);
    for(i = 0; i < h; i++) {
        int r = rand() % 256;
        int g = rand() % 256;
        int b = rand() % 256;
        for(j = 0; j < w; j++) {
            pix *temp = &pixels[i * w + j];
            temp -> r = image[(i * w + j) * 4];
            temp -> g = image[(i * w + j) * 4 + 1];
            temp -> b = image[(i * w + j) * 4 + 2];
            temp -> a = image[(i * w + j) * 4 + 3];
            //if(temp->a!=0)printf("%d\n", temp -> a);
            temp -> left = j > 0? &pixels[(i * w + j - 1)] : NULL;
            temp -> right = j < w-1 ? &pixels[(i * w + j + 1)] : NULL;
            temp -> up = i > 0? &pixels[((i - 1) * w + j)] : NULL;
            temp -> down = i < h-1? &pixels[((i + 1) * w + j)] : NULL;
            temp -> parent = temp;
            temp -> colorr = r;
            temp -> colorg = g;
            temp -> colorb = b;
            temp -> num = i * w + j;
            temp -> size = 1;
        }
    }

    int dif = 20;
    for(i = 0; i < h; i++){
        for(j = 0; j < w; j++){
            pix *temp = &pixels[i * w + j];
            if(temp->up)
                if(abs(temp->a - temp->up->a) <= dif)
                {Union(temp, temp -> up);}
            if(temp->down)
                if(abs(temp->a - temp->down->a) <= dif)
                {
                    Union(temp, temp -> down);
                }
            if (temp->left) {
                //if(temp -> r != 255) printf("%d %d\n", temp->r, temp->left->r);
                if(abs(temp->a - temp->left->a) <= dif)
                {
                    Union(temp, temp -> left);
                    //printf("A");
                }
            }
            if(temp->right)
                if(abs(temp->a - temp->right->a) <= dif)
                {Union(temp, temp -> right);}
        }
        //printf("%d %d\n", i, h);
    }

    //printf("KFJD");

    for(i = 0; i < n; i++) {
        pix *root = Find_Set(&pixels[i]);
        if (root->size < 5 ) {
            pixels[i].r = 0;
            pixels[i].g = 0;
            pixels[i].b = 0;
        }
        else {
            //printf("%d\n", root->size);
            pixels[i].r = root->colorr;
            pixels[i].g = root->colorg;
            pixels[i].b = root->colorb;
        }
        image[i * 4] = pixels[i].r;
        image[i * 4 + 1] = pixels[i].g;
        image[i * 4 + 2] = pixels[i].b;
        image[i * 4 + 3] = 255;
    }
    free(pixels);
    return;
}

int main() {

    int w = 0, h = 0;

    char *filename = "scull.png";
    char *picture = load_png_file(filename, &w, &h);

    if (picture == NULL) {
        printf("I can't read the picture %s. Error.\n", filename);
        return -1;
    }


    unsigned char *res = malloc(sizeof(char) * w * h * 4);
    for (i = 0; i < h * w * 4; i += 4) {
        pixel P;
        P.R = picture[i+0];
        P.G = picture[i+1];
        P.B = picture[i+2];
        P.alpha = picture[i+3];


        int avg = ((P.R + P.G + P.B + P.alpha) / 4) % 256;
        res[i] = 255;
        res[i + 1] = 255;
        res[i + 2] = 255;
        res[i + 3] = avg;

    }
    before(res, w, h);
    //char *new_image = "scull_res1.png";
    //write_png_file(new_image, res, w, h);
    components(w, h, res);
    free(picture);
    char *new_image = "scull_res.png";
    write_png_file(new_image, res, w, h);
    free(res);
    return 0;
}
