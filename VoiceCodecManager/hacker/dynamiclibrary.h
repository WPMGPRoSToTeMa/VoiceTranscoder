#ifndef DYNAMICLIBRARY_H
#define DYNAMICLIBRARY_H

// Различные файлы и макросы для разных платформ
#include <platform.h>

// Поиск функций
struct DYNLIB
{
	void *m_pHandle;	// Указатель на библиотеку
	void *m_pBase;		// Базовый адрес библиотеки
	size_t m_uiSize;	// Размер библиотеки
	
	// Создание
	inline DYNLIB(void *pAddr);
#if __linux__
	// Уничтожение (Только для Linux)
	~DYNLIB() { dlclose(m_pHandle); /* Закрываем библиотеку */ }
#endif
	// Поиск адреса функции по сигнатуре и маске
	inline void *FindAddr(const char *ccpSig, const char *ccpMask, size_t uiLen); // Сигнатура, маска (какие байты сравнивать, FF сравнивать, 00 нет), длина, например "\x1C\x44\xDA", "\xFF\x00\xFF", 3
	inline void *FindAddr(void *pBase, const char *ccpSig, const char *ccpMask, size_t uiLen);
	// Поиск по названию функции
	inline void *FindAddr(const char *ccpName)
	{
#ifdef _WIN32
		// Для Windows
		return (void *)GetProcAddress((HMODULE)m_pHandle, ccpName);
#elif __linux__
		// Для Linux
		return dlsym(m_pHandle, ccpName);
#endif
	}
	// Поиск по смещению
	inline void *FindAddr(unsigned long uiOffset) { return (unsigned char *)m_pBase + uiOffset; /* Прибавляем смещение к базовому адресу */ }
};

#if __linux__
// Получение размера библиотеки по базовому адресу
inline size_t GetDynLibSize(void *pBase)
{
	char cFile[255];	// Имя файла
	char cBuffer[2048];	// Буффер
	
	// Получаем карту процесса
	snprintf(cFile, sizeof(cFile) - 1, "/proc/%d/maps", getpid() /* Текущий процесс */);
	
	// Открываем файл
	FILE *pFile = fopen(cFile, "rt");
	
	size_t uiLength = 0;	// Длина
	
	void *pStart = NULL;	// Стартовый адрес
	void *pEnd = NULL;		// Конечный адрес
	
	// Пока не достигнут конец файла
	while (!feof(pFile))
	{
		// Читаем строку из файла
		fgets(cBuffer, sizeof(cBuffer) - 1, pFile);
		
		// Сканируем строку на стартовый и конечный адрес
#if defined AMD64
		sscanf(cBuffer, "%Lx-%Lx", &pStart, &pEnd);
#else
		sscanf(cBuffer, "%lx-%lx", &pStart, &pEnd);
#endif
		
		// Если стартовый адрес не равен базовому идём в начало цикла
		if (pStart != pBase)
			continue;
		
		// Вычитаем стартовый адрес из конечного и записываем
		uiLength = (size_t)pEnd  - (size_t)pStart;
		
		char cIgnore[100];	// Игнор ?
		int iValue;			// Число ?
		
		// Пока не достигнут конец файла
		while (!feof(pFile))
		{
			// Читаем строку
			fgets(cBuffer, sizeof(cBuffer) - 1, pFile);
			
			// Сканируем строку
#if defined AMD64
			sscanf(cBuffer, "%Lx-%Lx %s %s %s %d", &pStart, &pEnd, cIgnore, cIgnore, cIgnore, &iValue);
#else
			sscanf(cBuffer, "%lx-%lx %s %s %s %d", &pStart, &pEnd, cIgnore, cIgnore, cIgnore, &iValue);
#endif
			
			// Если число равно нулю выходим из цикла
			if (!iValue)
				break;
			
			// Прибавляем разницу к длине
			uiLength += (size_t)pEnd  - (size_t)pStart;
		}
		
		// Выходим из цикла
		break;
	}
	
	// Закрываем файл
	fclose(pFile);
	
	// Возвращаем длину
	return uiLength;
}
#endif

// Создание
DYNLIB::DYNLIB(void *pAddr)
{
#ifdef _WIN32
	// Для Windows
	HMODULE Module;	// Объект библиотеки
	
	// Получаем объект библиотеки
	GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCSTR)pAddr, &Module);
	
	// Получаем текущий процесс
	HANDLE Process = GetCurrentProcess();
	
	_MODULEINFO ModuleInfo;	// Информация о библиотеке
	
	// Получаем информацию о библиотеке
	GetModuleInformation(Process, Module, &ModuleInfo, sizeof ModuleInfo);
	
	// Закрываем хэндл процесса
	CloseHandle(Process);
	
	m_pHandle = Module; 						// Записываем указатель на библиотеку
	m_pBase = (void *)ModuleInfo.lpBaseOfDll;	// Записываем базовый адрес
	m_uiSize = ModuleInfo.SizeOfImage;			// Записываем размер библиотеки
#elif __linux__
	// Для Linux
	Dl_info Info;	// Информация о библиотеке
	
	// Получаем информацию о библиотеке
	dladdr(pAddr, &Info);
	
	m_pHandle = dlopen(Info.dli_fname, RTLD_NOW);	// Записываем указатель на библиотеку
	m_pBase = (void *)Info.dli_fbase;				// Записываем базовый адрес
	m_uiSize = GetDynLibSize(m_pBase);				// Записываем размер библиотеки
#endif
}

// Поиск по сигнатуре и маске
void *DYNLIB::FindAddr(const char *ccpSig, const char *ccpMask, size_t uiLen)
{
	unsigned char *ucpBuff = (unsigned char *)m_pBase;						// Получаем стартовый адрес
	unsigned char *ucpEnd = (unsigned char *)m_pBase + m_uiSize - uiLen;	// Получаем конечный	
	
	unsigned long ul;	// Текущая позиция в сигнатуре
	
	// Цикл по всей библиотеке
	while (ucpBuff <= ucpEnd)
	{
		// Проверяем сигнатуру и маску
		for (ul = 0; ul < uiLen; ul++)
		{
			// ADDRESS & MASK != SIGNATURE & MASK => break;
			if ((ucpBuff[ul] & ((unsigned char *)ccpMask)[ul]) != (((unsigned char *)ccpSig)[ul] & ((unsigned char *)ccpMask)[ul]))
				break;
		}
		
		// Если последний проверенный байт равен длине возвращаем адрес
		if (ul == uiLen)
			return (void *)ucpBuff;
		
		// Переходим к следующему байту
		ucpBuff++;
	}
	
	// Ничего не нашли, возвращаем нуль
    return NULL;
}

void *DYNLIB::FindAddr(void *pBase, const char *ccpSig, const char *ccpMask, size_t uiLen)
{
	unsigned char *ucpBuff = (unsigned char *)pBase;						// Получаем стартовый адрес
	unsigned char *ucpEnd = (unsigned char *)m_pBase + m_uiSize - uiLen;	// Получаем конечный	

	unsigned long ul;	// Текущая позиция в сигнатуре

	// Цикл по всей библиотеке
	while (ucpBuff <= ucpEnd)
	{
		// Проверяем сигнатуру и маску
		for (ul = 0; ul < uiLen; ul++)
		{
			// ADDRESS & MASK != SIGNATURE & MASK => break;
			if ((ucpBuff[ul] & ((unsigned char *)ccpMask)[ul]) != (((unsigned char *)ccpSig)[ul] & ((unsigned char *)ccpMask)[ul]))
				break;
		}

		// Если последний проверенный байт равен длине возвращаем адрес
		if (ul == uiLen)
			return (void *)ucpBuff;

		// Переходим к следующему байту
		ucpBuff++;
	}

	// Ничего не нашли, возвращаем нуль
	return NULL;
}
#endif