#ifndef MEMORY_H
#define MEMORY_H

// Различные файлы и макросы для разных платформ
#include <platform.h>

#if __linux__
// Если Linux, преобразователь адреса и длины в страницу и длину страниц
inline void AddrToPage(void **ppAddr, size_t *uipLen)
{
	// Преобразуем адрес
	void *pPage = (void *)(*(unsigned long *)ppAddr & ~(PAGE_SIZE - 1));
	
	// Преобразуем длину
	*uipLen = ((*(unsigned long *)ppAddr + *uipLen - 1) | (PAGE_SIZE - 1)) - (unsigned long)pPage + 1;
	
	// Записываем адрес
	*ppAddr = pPage;
}
#endif

// Полный доступ к памяти
inline bool SetFullMemAccess(void *pAddr, size_t uiLen)
{
#ifdef _WIN32
	// Если Windows
	// Старая протекция
	DWORD ulOldProtect;
	
	// Устанавливаем новую протекцию
	if (!VirtualProtect(pAddr, uiLen, PAGE_EXECUTE_READWRITE, &ulOldProtect))
#elif __linux__
	// Если Linux
	// Преобразуем адрес в страницу
	AddrToPage(&pAddr, &uiLen);
	
	// Устанавливаем новую протекцию
	if (mprotect(pAddr, uiLen, PROT_READ | PROT_WRITE | PROT_EXEC))
#endif
		// Не успешно
		return false;
	
	// Успешно
	return true;
}

#endif