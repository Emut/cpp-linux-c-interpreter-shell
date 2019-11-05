/*
* CSparseTrie
* Umut Ekici 2019
* A memory efficient take on the Trie structure
* Nodes hold parts of keys instead of single letters. New childs spawn whenever keys diverge from each other.
* Ex: Amsterdam, America and American are inserted to an empty Trie. Result would be {Am}
*                                                                                     /\
*                                                                             {erica#}  {sterdam#}
*                                                                                 |
*																				{n#}
* Where nodes with # hold valid keys.
* Insert and Find operations are O(n) where n is the length of key. (Some would interpret this as O(1), since key sizes
* are aprox. constant and complexity does not depend on the number of keys currently in the DS.)
*
* Notes:	-This DS is NOT thread safe! Concurrent Find operations are OK; however, as Insert may change the layout of the tree
*				its access should be guarded.
*			-There is no Remove operation. It is to be implemented. 
*			-operator[] inserts a default data at the given key, if key does not exist in the Trie (similar to std::map).
*			-getElm returns the keys in lexicological order.
*			-Trie can hold any C-string with length <= than CSPARSETRIE_MAX_KEY_LEN and char values < than CSPARSETRIE_NUM_NODES
*				(i.e. key does not have to be in readable ascii.)
*/

#ifndef CSparseTrie_H
#define CSparseTrie_H

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

template <class TData> class CSparseTrie {
public :
    #define CSPARSETRIE_NUM_NODES 128
    #define CSPARSETRIE_MAX_KEY_LEN 100

    CSparseTrie();  
    ~CSparseTrie();

    CSparseTrie<TData>* Find(const char* par_cpKey);
    bool Insert(const char* par_cpKey, const TData& par_TData);
    void PrintKeys();
    int Size();
    bool getElm(int par_nElementID, char* par_cpElementKey, TData* & par_TDatapElementValue);
    CSparseTrie<TData>* getNthChild(int par_nChildID);
    TData& operator[](const char* par_cpKey);

public :
    TData* pTData;	

private :
    void PrintKeysHelper(char* par_cpKeysSoFar);
    bool getElmHelper(int par_nSearchID, int& par_nSeenCount, char* par_cpKeysSoFar, TData* & par_TDatapElementValue);

private :
    char* cpKey;		
    CSparseTrie* itsNodes[CSPARSETRIE_NUM_NODES];		
    int nKeyCount;		
    int nKeyLength;		
};


template <class TData> CSparseTrie<TData>::CSparseTrie() {
    cpKey = 0;
    nKeyCount = 0;
    nKeyLength = 0;
    for(int i = 0; i < CSPARSETRIE_NUM_NODES; i++)
    	itsNodes[i] = NULL;
    cpKey = NULL;
    nKeyLength = 0;
    pTData = NULL;
    nKeyCount = 0;
}

template <class TData> CSparseTrie<TData>::~CSparseTrie() {
    if(pTData != NULL)
    	delete pTData;
    
    if(cpKey != NULL)
    	delete []cpKey;
    	
    for(int nI = 1; nI < CSPARSETRIE_NUM_NODES; nI++)
    	if(itsNodes[nI] != NULL)
    		delete itsNodes[nI];
}

template <class TData> CSparseTrie<TData>* CSparseTrie<TData>::Find(const char* par_cpKey) {
    if(cpKey == NULL)
    {
    	if(itsNodes[(unsigned char)par_cpKey[0]] == NULL)
    		return NULL;
    	else
    		return itsNodes[(unsigned char)par_cpKey[0]]->Find(par_cpKey);
    }
    int i = 0;
    for(; i < nKeyLength; i++)
    {
    	if(par_cpKey[i] == 0)
    		return NULL;
    	if(par_cpKey[i] != cpKey[i])
    		return NULL;
    }
    if((unsigned char)par_cpKey[i] == 0)
    	if(itsNodes[0] != NULL)
    		return this;	//return pTData;	//func used to return pointer to data, now it returns pointer to node. Data is still accessible via the node!
    	else 
    		return NULL;
    else
    	if(itsNodes[(unsigned char)par_cpKey[i]] != NULL)
    		return itsNodes[(unsigned char)par_cpKey[i]]->Find(par_cpKey + i);
    	else
    		return NULL;
}

template <class TData> bool CSparseTrie<TData>::Insert(const char* par_cpKey, const TData& par_TData) {
    if(cpKey != NULL)
    {
    	bool isSameKey = true;
    	int i = 0;
    	for(; i < nKeyLength; i++)
    		if(cpKey[i] != par_cpKey[i])
    		{
    			isSameKey = false;
    			break;
    		}
    	if(isSameKey)
    	{
    		if(par_cpKey[i] == 0)	//is already the key here
    		{
    			if(itsNodes[0] == NULL)	//if node is not marked, mark it.
    			{
    				pTData = new TData();
    				*pTData = par_TData;
    				itsNodes[0] = this;
    				++nKeyCount;
    				return true;
    			}
    			*pTData = par_TData;
    			return false;	
    		}
    		if(itsNodes[par_cpKey[i]] == NULL)
    			itsNodes[par_cpKey[i]] = new CSparseTrie();
    
    		bool bRetVal = itsNodes[par_cpKey[i]]->Insert(par_cpKey + i, par_TData);
    		if(bRetVal)
    			++nKeyCount;
    		return bRetVal;
    	}
    	else
    	{
    		//key in node and queried key are not matching starting from i th char
    		CSparseTrie* temp = new CSparseTrie();
    		temp->Insert(cpKey+i, *pTData);
    		temp->nKeyCount = nKeyCount;
    		for(int nI = 0; nI < CSPARSETRIE_NUM_NODES; nI++)
    		{
    			temp->itsNodes[nI] = itsNodes[nI];
    			itsNodes[nI] = NULL;
    		}
    		itsNodes[cpKey[i]] = temp;
    
    		cpKey[i] = 0;
    		nKeyLength = i;
    
    		Insert(par_cpKey, par_TData);
    		return true;
    	}
    
    
    }
    
    
    if(cpKey == NULL && itsNodes[par_cpKey[0]] == NULL)
    {
    	cpKey = new char[strlen(par_cpKey) + 1];
    	nKeyLength = sprintf(cpKey, "%s", par_cpKey);
    	cpKey[nKeyLength] = 0;
    	itsNodes[0] = this;
    	pTData = new TData;
    	*pTData = par_TData;
    	++nKeyCount;
    	return true;
    }
    return false;
}

template <class TData> void CSparseTrie<TData>::PrintKeys() {
    char cpBuffer[CSPARSETRIE_MAX_KEY_LEN + 1];	//+1 for null term
    cpBuffer[0] = 0;
    PrintKeysHelper(cpBuffer);
}

template <class TData> int CSparseTrie<TData>::Size() {
    return nKeyCount;
}

template <class TData> bool CSparseTrie<TData>::getElm(int par_nElementID, char* par_cpElementKey, TData* & par_TDatapElementValue) {
    if(par_cpElementKey == NULL)
    	return false;
    	
    if(par_nElementID >= nKeyCount)
    	return false;
    	
    int nSeenCount = 0;
    char cpKeysSoFar[CSPARSETRIE_MAX_KEY_LEN + 1];
    cpKeysSoFar[0] = 0;
    
    getElmHelper(par_nElementID + 1, nSeenCount, cpKeysSoFar, par_TDatapElementValue);	//+1 added to ElmID: helper counts than compares to required ID!
    sprintf(par_cpElementKey, "%s", cpKeysSoFar);
    return true;
}

template <class TData> CSparseTrie<TData>* CSparseTrie<TData>::getNthChild(int par_nChildID) {
    if(par_nChildID < CSPARSETRIE_NUM_NODES)
    	return itsNodes[par_nChildID];
    else
    	return NULL;
}

template <class TData> TData& CSparseTrie<TData>::operator[](const char* par_cpKey) {
    if(cpKey == NULL)
    {
    	if(itsNodes[par_cpKey[0]] == NULL)
    	{
    		TData* newdata = new TData();	//NO DATA
    	
    		Insert(par_cpKey, *newdata);
    		return operator[](par_cpKey);
    	}
    	else
    	{
    		int nCountBefore = itsNodes[par_cpKey[0]]->nKeyCount;
    		TData& nRetVal = itsNodes[par_cpKey[0]]->operator[](par_cpKey);
    		if(nCountBefore != itsNodes[par_cpKey[0]]->nKeyCount)
    			++nKeyCount;
    		return nRetVal;
    	}
    }
    int i = 0;
    for(; i < nKeyLength; i++)
    {
    	if(par_cpKey[i] == 0)
    	{
    		TData* newdata = new TData();	//NO DATA
    		Insert(par_cpKey, *newdata);
    		
    		return operator[](par_cpKey);
    	}
    	if(par_cpKey[i] != cpKey[i])
    		{
    		TData* newdata = new TData();	//NO DATA
    		Insert(par_cpKey, *newdata);
    		
    		return operator[](par_cpKey);
    	}
    }
    if(par_cpKey[i] == 0)
    	if(itsNodes[0] != NULL)
    		{
    		return *pTData; //DATA FOUND!
    		}
    	else 
    	{
    		TData* newdata = new TData();	//NO DATA
    		Insert(par_cpKey, *newdata);
    		
    		return operator[](par_cpKey);
    	}
    else
    	if(itsNodes[par_cpKey[i]] != NULL)
    	{
    		int nCountBefore = itsNodes[par_cpKey[i]]->nKeyCount;
    		TData& nRetVal = itsNodes[par_cpKey[i]]->operator[](par_cpKey + i);
    		if(nCountBefore != itsNodes[par_cpKey[i]]->nKeyCount)
    			++nKeyCount;
    		return nRetVal;
    	}
    	else
    	{
    		TData* newdata = new TData();	//NO DATA
    		Insert(par_cpKey, *newdata);
    		
    		return operator[](par_cpKey);
    	}
}

template <class TData> void CSparseTrie<TData>::PrintKeysHelper(char* par_cpKeysSoFar) {
    if(cpKey != NULL)
    	strcat(par_cpKeysSoFar, cpKey);
    if(itsNodes[0] != NULL)
    	printf("%s\n", par_cpKeysSoFar);
    
    for(int i = 1; i < CSPARSETRIE_NUM_NODES; i++)
    {
    	if(itsNodes[i] != NULL)
    		itsNodes[i]->PrintKeysHelper(par_cpKeysSoFar);
    }
    
    par_cpKeysSoFar[strlen(par_cpKeysSoFar) - nKeyLength] = 0;
}

template <class TData> bool CSparseTrie<TData>::getElmHelper(int par_nSearchID, int& par_nSeenCount, char* par_cpKeysSoFar, TData* & par_TDatapElementValue) {
    if(cpKey != NULL)
    	strcat(par_cpKeysSoFar, cpKey);
    if(itsNodes[0] != NULL)
    {
    	++par_nSeenCount;
    	if(par_nSearchID == par_nSeenCount)
    	{
    		par_TDatapElementValue = pTData;
    		return true;
    	}
    }
    
    
    for(int i = 1; i < CSPARSETRIE_NUM_NODES; i++)
    {
    	if(itsNodes[i] != NULL)
    	{
    		bool bRetVal = itsNodes[i]->getElmHelper(par_nSearchID, par_nSeenCount, par_cpKeysSoFar, par_TDatapElementValue);
    		if(bRetVal)
    			return true;
    	}
    }
    
    par_cpKeysSoFar[strlen(par_cpKeysSoFar) - nKeyLength] = 0;
    return false;
}

#endif
/*
int main(){

	CSparseTrie<void*> myTrie;

	myTrie.Insert("printf", (void*)printf);
	myTrie.Insert("malloc", (void*)malloc);
	myTrie.Insert("free", (void*)free);


	for(int i = 0; i < myTrie.Size(); ++i){
		char cpKey[CSPARSETRIE_MAX_KEY_LEN + 1];
		void** npVal;
		myTrie.getElm(i, cpKey, npVal);
		printf("%s:%p\n", cpKey, *npVal);
	}


	int (*f)(char*);
	auto item = myTrie.Find("printf");
	f = (int (*)(char*))*(item->pTData);

	int (*f2)(char*, int);
	f2 = (int (*)(char*, int))*(item->pTData);

	int (*f3)(...);
	f3 = (int (*)(...))*(item->pTData);

	f("whaaaa\n");
	f2("wow%d\n", 999);
	f3("This is nice %d %c\n", 112, 'a');

	item = myTrie.Find("malloc");
	f3 = (int (*)(...))*(item->pTData);
	item = myTrie.Find("free");
	auto f4 = (int (*)(...))*(item->pTData);
	
	int nRetVal = f3(100);
	char* cpTemp = (char*)nRetVal;
	printf("%p", cpTemp);
	
	sprintf(cpTemp, "ASDasdasdasdASD");
	f4(cpTemp);
	char c;
	scanf("%c", &c);
	return 0;
}
*/