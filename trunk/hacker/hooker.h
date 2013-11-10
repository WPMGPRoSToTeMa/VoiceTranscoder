#ifndef HOOKER_H
#define HOOKER_H

#include <platform.h>	// Различные файлы и макросы для разных платформ
#include <memory.h>		// SetFullMemAccess

#define JMP_NEAR		0xE9	// Байтовый опкод jmp near
#define JMP_NEAR_SIZE	5		// Размер опкода jmp near (1 + 4, 0xE9 + ADDR)

// Для удобства
#define R_CAST					reinterpret_cast

#ifdef _WIN32
#define METHOD_CALL				__fastcall
#define METHOD_ARGS				void *pThis, int iTrash
#define METHOD_ARGS_TYPE		void *, int
#define METHOD_ARGS_CALL		pThis, iTrash
#elif __linux__
#define METHOD_CALL
#define METHOD_ARGS				void *pThis
#define METHOD_ARGS_TYPE		void *
#define METHOD_ARGS_CALL		pThis
#endif

// Перехват функций
template <typename T>
struct HOOKER
{
	T *m_pfnOrig;	// Адрес оригинальной функции
	
	unsigned char m_ucOrigBytes[JMP_NEAR_SIZE];		// Оригинальные первые 5 байт функции
	unsigned char m_ucPatchedBytes[JMP_NEAR_SIZE];	// Пропатченные первые 5 байт функции
	
	inline HOOKER(void *pfnOrig, T *pfnHook);		// Создание
	~HOOKER(void) { Unhook(); /* Сбрасываем перехват */ };	// Удаление
	inline void Hook(void) { if (SetFullMemAccess(m_pfnOrig, JMP_NEAR_SIZE)) memcpy(m_pfnOrig, m_ucPatchedBytes, JMP_NEAR_SIZE); };	// Перехватываем функцию (копируем патченные байты)
	inline void Unhook(void) { if (SetFullMemAccess(m_pfnOrig, JMP_NEAR_SIZE)) memcpy(m_pfnOrig, m_ucOrigBytes, JMP_NEAR_SIZE); };	// Сбрасываем перехват функции (копируем оригинальные байты)
};

// Создание
template <typename T>
HOOKER<T>::HOOKER(void *pfnOrig, T *pfnHook)
{
	// Записываем оригинальный адрес функции
	m_pfnOrig = (T *)pfnOrig;
	
	// Копируем оригинальные первые 5 байт функции
	memcpy(m_ucOrigBytes, pfnOrig, JMP_NEAR_SIZE);
	
	// Записываем первым байтом опкод jmp near
	m_ucPatchedBytes[0] = JMP_NEAR;
	
	// Записываем в остальные 4 байта адрес
	*(long *)&m_ucPatchedBytes[1] = (unsigned char *)pfnHook - (unsigned char *)pfnOrig - JMP_NEAR_SIZE;
	
	// Перехватываем
	Hook();
}

// Перехват виртуальных функций
struct VIRTHOOKER
{
	void *m_pAddr;		// Адрес оригинальной функции
	void *m_pHookAddr;	// Адрес функции в которую идёт перехват
	void **m_ppVTable;	// Таблица виртуальных функций (VTable)
	
	inline VIRTHOOKER(void **ppVTable, int iOffset, void *pHookAddr);	// Создание
	~VIRTHOOKER(void) { Unhook(); /* Сбрасываем перехват */ };	// Удаление
	inline void Hook(void) { if (SetFullMemAccess(m_ppVTable, sizeof(void *))) *m_ppVTable = m_pHookAddr; };	// Перехватываем функцию (записываем в VTable адрес функции в которую идёт перехват)
	inline void Unhook(void) { if (SetFullMemAccess(m_ppVTable, sizeof(void *))) *m_ppVTable = m_pAddr; };		// Сбрасываем перехват (записываем в VTable адрес оригинальной функции)
};

// Создание
VIRTHOOKER::VIRTHOOKER(void **ppVTable, int iOffset, void *pHookAddr)
{
	// Записываем адрес функции в которую идёт перехват
	m_pHookAddr = pHookAddr;
	
	// Записываем VTable
	m_ppVTable = &ppVTable[iOffset];
	
	// Записываем адрес оригинальной функции
	m_pAddr = *m_ppVTable;
	
	// Перехватываем
	Hook();
}

#endif