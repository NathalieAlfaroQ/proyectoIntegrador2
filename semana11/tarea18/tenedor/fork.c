

#include "syscall.h"

void nada(int);
int id;

int main() {
   Fork( rutina );
   Write( "basura", 6, 1 );
}


void rutina( int param ) {
   Write( "hola, nuevo fork2\n", 4, 1 );
}