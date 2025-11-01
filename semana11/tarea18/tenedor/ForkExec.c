
#include "syscall.h"

void UseMemory(){
	Exec("../test/Memory");
}

int main() {
	int i=0;

	for (i = 0; i < 5; i++) {
		Fork( UseMemory );
		Yield();
	}
	Exit( 0 );
}