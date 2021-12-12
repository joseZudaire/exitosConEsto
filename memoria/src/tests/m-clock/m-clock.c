#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "m-clock.h"

const int cantMaxima = 5;
t_page stack[5] = {0};
//Paginas que llegan
char visit_pages[11] = {'3', '5', '0', '5', '1', '0', '8', '2', '5', '1', '3'};
int cant_page_fault = 0;
int change_page_sem = 1;


void page_fault() {
    printf("\nPage fault\n");
    cant_page_fault++;
    change_page_sem = 0;
}


// imprimir stack
void print_stack()
{
    int m;
    for (m=0; m<cantMaxima; m++) {
        if (stack[m].pid == NULL) {
            printf("  N  ");
        } else {
            printf("  %c", stack[m].pid);
            if (stack[m].use_bit == 1) printf("*");
            else printf("-");
            if (stack[m].modif_bit == 1) printf("*");
            else printf("-");
        }
    }
    printf("\n\n");
}

// Mover el valor de la pila
void change_stack(int index, int i)
{
    int k;
    // CARGa si el stack tiene una posicion vacia    
    t_page temp = stack[index];
    for (k=index; k<cantMaxima; k++) { 
        if (index != 0) {
            if (stack[k+1].pid == NULL) {
                break;
            }
            if (k+1 < cantMaxima) {
               stack[k] = stack[k+1];
            }
            if (stack[k].use_bit == 1 && stack[k].modif_bit) {
                page_fault();
                stack[k].pid=i;
                stack[k].use_bit=1;
                stack[k].modif_bit=0;
                break;
            }
        }
    }


    change_page_sem = 0;

}

void create_stack() {
    // Asignación de stack
    int i;
    for (i=0; i<cantMaxima; i++) {
        t_page page;// = malloc(sizeof(t_page));
        page.pid = NULL;
        page.use_bit = 0;
        page.modif_bit = 0;
        stack[i] = page;

    }
}

int notIncluded(char* page) {
    int i;
    for (i=0; i<cantMaxima; i++) {
        if (page == stack[i].pid) {
            stack[i].use_bit = 1;
            stack[i].modif_bit = 1;
            return 0;
        }
    }
    return 1;
}

int main()
{
    int i, j, k, w;
    int index;
    j = 0;
    printf("Orden de llegada de las paginas: \n");
    for (w=0;w<strlen(visit_pages); w++) {
        printf(" %c ", visit_pages[w]);
    }
    printf("\n");

    create_stack();

    printf("Parte inferior de la pila-> Parte superior de la pila: \n");
    print_stack();

    for (i=0; i<strlen(visit_pages); i++) {
        for (j=0; j<cantMaxima; j++) {
//        while (change_page_sem == 1) {
            if (stack[j].pid == NULL && notIncluded(visit_pages[i])) {
                stack[j].pid = visit_pages[i];
                page_fault();
                break;
            }
            if (stack[j].pid == visit_pages[i]) {
                if (j+1 < cantMaxima && stack[j+1].pid == NULL || j == (cantMaxima - 1)) {
                    printf("\nLa pagina ya se encuentra en el stack\n");
                    change_page_sem = 0;
                    break;
                }
                change_stack(j, i);
                
            } else if (j == (cantMaxima - 1) && stack[j].pid != visit_pages[i]) {
                change_stack(0, i);
            }
            
            printf("");
//            printf("\n indice: %d\n",j);

//            j++;
//            if (j==cantMaxima) j=0;
        }

        printf("---> %c:\n", visit_pages[i]);
        print_stack();
        change_page_sem = 1;
    }

    return 0;
}