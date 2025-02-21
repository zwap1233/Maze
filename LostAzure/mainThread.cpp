#include "mainThread.h"

#include "InterruptHandler.h"
#include "BoardManager.h"

void mainThread(ULONG argument)
{
	(void) argument;
	
	initBoardManager();
	
	while (1)
	{
		updateBoards();
		tx_thread_sleep(10);
	}
}