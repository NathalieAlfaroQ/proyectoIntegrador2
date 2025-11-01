

#include "syscall.h"


void sum() {

	Exit( 2025 );

}

int main() {
	Fork(sum);
	Yield();

	Fork(sum);
	Yield();

	Exit( 2025 ); 
}
