#include <stdio.h>
#include <stdlib.h>

int main(){
    int ** matrice;
    int r,c;
    int righe = 10;
    int colonne = 6;

    /* ALLOCAZIONE MATRICE */
    matrice = (int **) malloc(sizeof(int *)*righe);
    for (r = 0; r < righe; r++)
            matrice[r] = (int *) malloc(sizeof(int)*colonne);
    

    for (r = 0; r<righe; r++){
        for (c = 0; c < colonne; c++)
        {
            matrice[r][c] = 0;
        }
    }

    for (r = 0; r<righe; r++){
        printf("\n");
        for (c = 0; c < colonne; c++)
        {
           printf("%d ", matrice[r][c]);
        }
    }
}