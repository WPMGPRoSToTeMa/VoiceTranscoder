#ifndef VIRTFUNCS_H
#define VIRTFUNCS_H

// Получение указателя виртуальной таблицы по объекту
inline void **GetVTable(void *pObject, int iBase)
{
	return *(void ***)((char *)pObject + iBase);
}

#ifdef GOLDSOURCE
#include <extdll.h>
#ifdef METAMOD
#include <meta_api.h>

// Получение виртуальной таблицы по классу
inline void **GetVTable(const char *ccpClass, int iBase)
{
	// Создаём энтити
	edict_t *pEdict = CREATE_ENTITY();
	
	// Вызываем энтити в движке
	CALL_GAME_ENTITY(PLID, ccpClass, &pEdict->v);
	
	// Возвращаем указатель на виртуальную таблицу
	return *(void ***)((char *)pEdict->pvPrivateData + iBase);
}

#endif
#endif

#endif