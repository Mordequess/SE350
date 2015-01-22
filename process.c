#include "process.h"
#include "scheduler.h"

void null_process () {
	while (1) {
		release_processor ( ) ;
	}
}
