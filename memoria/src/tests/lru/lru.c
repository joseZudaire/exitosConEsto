#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int cantMaxima = 3;
char stack[3] ={0}; 
char visit_pages[17] = {'1', '2', '3', '4', '5', '1', '2', '2', '3', '4', '5', '6', '3', '7','3','8','9'}; 
//char visit_pages[11] = {'3', '3', '3', '3', '3', '3', '3', '3', '3', '3', '3'}; 
//char visit_pages[11] = {'3', '5', '0', '4', '9', '7', '8', '2', '5', '1', '3'}; 
int cant_page_fault = 0;

void page_fault() {
    printf("\nPage fault\n");
    cant_page_fault++;
}

void print_stack()
{
    int m;
    
    for (m=0; m<cantMaxima; m++) {
        if (stack[m] == NULL) {
            printf("  N");
        } else {
            printf("  %c", stack[m]);
        }
    }
    printf("\n");
}


void change_stack(int index, int i)
{
    int temp, k;
    
    temp = stack[index];
    for (k=index; k<cantMaxima; k++) { 
        if (index != 0) {
            if (stack[k+1] == NULL) {
                break;
            }
        }
        if (k+1 < cantMaxima) {
            stack[k] = stack[k+1];
        }
    }
    
    if (index != 0) {
        stack[k] = temp; 
        stack[k] = visit_pages[i]; 
    } else {
        stack[(cantMaxima - 1)] = temp; 
        stack[(cantMaxima - 1)] = visit_pages[i]; 
        page_fault();
    }
//    printf("indice: %d\n", index);
}

int main()
{
    int i, j, k, w;
    int index; 
    int temp; 

    printf("Orden de llegada de las paginas: \n");
    for (w=0;w<strlen(visit_pages); w++) {
        printf(" %c ", visit_pages[w]);
    }
    printf("\n");

    
    for (i=0; i<cantMaxima; i++) {
        stack[i] = NULL;
    }
    printf("\n");
    printf("Parte inferior de la pila-> Parte superior de la pila: \n");
    printf("\n");
    print_stack(); 
    for (i=0; i<strlen(visit_pages); i++) {
        for (j=0; j<cantMaxima; j++) {
            if (stack[j] == NULL) {
                stack[j] = visit_pages[i];
                page_fault();
                break;
            }
            if (stack[j] == visit_pages[i]) {
                if (j+1 < 5 && stack[j+1] == NULL || j == (cantMaxima - 1)) { 
                    printf("\nLa pagina ya se encuentra en el stack\n");
                    break;
                }
                change_stack(j, i);
            } else if (j == (cantMaxima - 1) && stack[j] != visit_pages[i]) {
                change_stack(0, i); 
            }
        }
        printf("---> %c:\n", visit_pages[i]);
        print_stack();
        printf("\n");
    }
    printf("\n Ocurrieron %d fallos de pagina \n\n", cant_page_fault);
    return 0;
}