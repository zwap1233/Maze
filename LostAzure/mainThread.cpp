#include "mainThread.h"

#include "InterruptHandler.h"
#include "BoardManager.h"
#include "Board.h"

void mainThread(ULONG argument)
{
	(void) argument;
	
	initBoardManager();
	
	while (1)
	{
		updateTestPattern();
		tx_thread_sleep(1);
	}
}