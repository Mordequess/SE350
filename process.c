#include "process.h"
#include "scheduler.h"
#include "rtx.h"

void null_process() {
	while (1) {
		release_processor() ;
	}
}
