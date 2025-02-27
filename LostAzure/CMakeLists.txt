cmake_minimum_required(VERSION 3.15)

project(LostAzure LANGUAGES C CXX ASM)

find_bsp(
	ID com.sysprogs.arm.stm32
	VERSION 2024.07
	MCU STM32H753ZI
	CONFIGURATION com.sysprogs.toolchainoptions.arm.syscallspecs=
	FRAMEWORKS com.sysprogs.arm.stm32.hal com.sysprogs.arm.stm32.ll com.sysprogs.arm.stm32.threadx
	HWREGISTER_LIST_FILE STM32H7xxxx/DeviceDefinitions/stm32h753xx.xml
	DISABLE_GNU_EXTENSIONS)

bsp_compile_definitions(USE_FULL_LL_DRIVER)
add_bsp_based_executable(
	NAME LostAzure
	SOURCES
		LostAzure.cpp
		app_azure_rtos_config.h
		app_threadx.h
		tx_initialize_low_level.S
		tx_user.h
		system_stm32h7xx.c
		stm32h7xx_hal_conf.h
		syscalls.c
		sysmem.c
		BoardManager.h
		BoardManager.cpp
		InterruptHandler.cpp
		InterruptHandler.h
		mainThread.cpp
		mainThread.h
		SystemTimer.cpp
		Room.cpp
		Room.h
		Maze.cpp
		Maze.h
		Section.cpp
		Section.h
		Renderer.cpp
		Renderer.h
	GENERATE_BIN
	GENERATE_MAP
	OUTPUT_RELOCATION_RECORDS)
target_compile_definitions(LostAzure PRIVATE USE_FULL_LL_DRIVER)
