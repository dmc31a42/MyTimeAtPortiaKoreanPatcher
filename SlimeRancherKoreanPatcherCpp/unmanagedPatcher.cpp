// defined _ITERATOR_DEBUG_LEVEL, _CRT_SECURE_NO_WARNINGS for UABE_API
#define _ITERATOR_DEBUG_LEVEL 0
#define _CRT_SECURE_NO_WARNINGS

#include "stdafx.h"
#include "unmanagedPatcher.h"
#include <iostream>
#include <vector>
#include <map>

#include "AssetsTools\defines.h"
#include "AssetsTools\AssetsFileFormat.h"
#include "AssetsTools\AssetsFileReader.h"
#include "AssetsTools\AssetsFileTable.h"
#include "AssetsTools\AssetTypeClass.h"
#include "AssetsTools\AssetsBundleFileFormat.h"
#include "AssetsTools\ClassDatabaseFile.h"

#define MY_DEBUG
using namespace std;

int filesize(FILE* file)
{
	fseek(file, 0L, SEEK_END);
	int fileLen = ftell(file);
	fseek(file, 0L, SEEK_SET);
	return fileLen;
}

int FindPathID(string str)
{
	std::size_t foundUnderBar = str.find_last_of('_');
	std::size_t foundDot = str.find_last_of('.');
	string ID_string = str.substr(foundUnderBar + 1, foundDot - foundUnderBar - 1);
	return std::stoi(ID_string);
}

unmanagedPatcher::unmanagedPatcher(string gameFolderPath, string currentDirectory)
{
	_gameFolderPath = gameFolderPath;
	_currentDirectory = currentDirectory;
	resAssetsFileName = "resources.assets";
	sharedAssetsFileName = "sharedassets0.assets";
	classDatabaseFileName = "Resource\\U5.6.0f3.dat";
	noExtGlobalAssetFileName = "globalgamemanagers";
	logOfstream.open((_currentDirectory + "output_cpp.log").c_str(), ios::trunc);

#ifdef MY_DEBUG
	logOfstream << "_gameFolderPath : " << _gameFolderPath << endl;
	logOfstream << "_currentDirectory : " << _currentDirectory << endl;
	logOfstream << "(_gameFolderPath + resAssetsFileName).c_str() : " << (_gameFolderPath + resAssetsFileName).c_str() << endl;
	logOfstream << "(_gameFolderPath + sharedAssetsFileName).c_str() : " << (_gameFolderPath + sharedAssetsFileName).c_str() << endl;
	logOfstream << "(_currentDirectory + classDatabaseFileName).c_str() : " << (_currentDirectory + classDatabaseFileName).c_str() << endl;
	logOfstream << "(_gameFolderPath + noExtGlobalAssetFileName).c_str() : " << (_gameFolderPath + noExtGlobalAssetFileName).c_str() << endl;
#endif

#ifdef MY_DEBUG
	logOfstream << endl;
#endif
#ifdef MY_DEBUG
	logOfstream << "fopen_s pFILE" << endl;
#endif
	fopen_s(&pResAssetsFile, (_gameFolderPath + resAssetsFileName).c_str(), "rb");
	fopen_s(&psharedAssetsFile, (_gameFolderPath + sharedAssetsFileName).c_str(), "rb");
	fopen_s(&pClassDatabaseFile, (_currentDirectory + classDatabaseFileName).c_str(), "rb");
	fopen_s(&pNoExtGlobalAssetsFile, (_gameFolderPath + noExtGlobalAssetFileName).c_str(), "rb");
#ifdef MY_DEBUG
	logOfstream << "create AssetsFile, ClassDatabaseFile" << endl;
#endif
	resAssetsFile = new AssetsFile(AssetsReaderFromFile, (LPARAM)pResAssetsFile);
	sharedAssetsFile = new AssetsFile(AssetsReaderFromFile, (LPARAM)psharedAssetsFile);
	classDatabaseFile = new ClassDatabaseFile();
	classDatabaseFile->Read(AssetsReaderFromFile, (LPARAM)pClassDatabaseFile);
	noExtGlobalAssetsFile = new AssetsFile(AssetsReaderFromFile, (LPARAM)pNoExtGlobalAssetsFile);
#ifdef MY_DEBUG
	logOfstream << "create AssetsFileTable" << endl;
#endif
	resAssetsFileTable = new AssetsFileTable(resAssetsFile);
	sharedAssetsFileTable = new AssetsFileTable(sharedAssetsFile);
	noExtGlobalAssetsFileTable = new AssetsFileTable(noExtGlobalAssetsFile);
#ifdef MY_DEBUG
	logOfstream << "make findByClassID" << endl;
#endif
	for (int i = 0; i < (int)classDatabaseFile->classes.size(); i++)
	{
		int classId = classDatabaseFile->classes[i].classId;
		findByClassID.insert(map<int, unsigned int>::value_type(classId, i));
	}

	FindInformation();
}

unmanagedPatcher::~unmanagedPatcher()
{
	if (resAssetsFileTable)
	{
		delete resAssetsFileTable;
		resAssetsFileTable = NULL;
	}
	if (sharedAssetsFileTable)
	{
		delete sharedAssetsFileTable;
		sharedAssetsFileTable = NULL;
	}
	if (noExtGlobalAssetsFileTable)
	{
		delete noExtGlobalAssetsFileTable;
		noExtGlobalAssetsFileTable = NULL;
	}

	if (classDatabaseFile)
	{
		delete classDatabaseFile;
		classDatabaseFile = NULL;
	}
	if (noExtGlobalAssetsFile)
	{
		delete noExtGlobalAssetsFile;
		noExtGlobalAssetsFile = NULL;
	}
	if (sharedAssetsFile)
	{
		delete sharedAssetsFile;
		sharedAssetsFile = NULL;
	}
	if (resAssetsFile)
	{
		delete resAssetsFile;
		resAssetsFile = NULL;
	}

	if (pResAssetsFile)
	{
		fclose(pResAssetsFile);
		pResAssetsFile = NULL;
	}
	if (psharedAssetsFile)
	{
		fclose(psharedAssetsFile);
		psharedAssetsFile = NULL;
	}
	if (pClassDatabaseFile)
	{
		fclose(pClassDatabaseFile);
		pClassDatabaseFile = NULL;
	}
	if (pNoExtGlobalAssetsFile)
	{
		fclose(pNoExtGlobalAssetsFile);
		pNoExtGlobalAssetsFile = NULL;
	}
}

void unmanagedPatcher::FindInformation()
{
#ifdef MY_DEBUG
	logOfstream << "start FindInformation" << endl;
#endif
	//resources
	{
		map<string, int> resourcesMaterialNames;
		int resourcesMaterialCount;
		map<string, int> resouecesMonoBehaviourNames;
		int resouecesMonoBehaviourCount;

		resourcesMaterialNames.insert(map<string, int>::value_type("KaushanScript-Regular SDF Material", 0));
		resourcesMaterialNames.insert(map<string, int>::value_type("SourceHanSansSC-Bold SDF Material", 0));
		resourcesMaterialCount = resourcesMaterialNames.size();

		resouecesMonoBehaviourNames.insert(map<string, int>::value_type("KaushanScript-Regular SDF", 0));
		resouecesMonoBehaviourNames.insert(map<string, int>::value_type("SourceHanSansSC-Bold SDF", 0));
		resouecesMonoBehaviourCount = resouecesMonoBehaviourNames.size();


		unsigned int resourcesCurrentPathID = 1;
		resourcesCurrentPathID = 1;
		for (; resourcesCurrentPathID <= resAssetsFileTable->assetFileInfoCount; resourcesCurrentPathID++)
		{
			if (resourcesMaterialNames.size() == 0)
			{
				break;
			}
			AssetFileInfoEx *tempAssetFileInfoEx = resAssetsFileTable->getAssetInfo(resourcesCurrentPathID);
#ifdef MY_DEBUG
			logOfstream << "[" << resourcesCurrentPathID << "] : " << "->name : " << tempAssetFileInfoEx->name << endl;
#endif
			map<string, int>::iterator FindIter = resourcesMaterialNames.find(tempAssetFileInfoEx->name);
			if (FindIter != resourcesMaterialNames.end())
			{
				FindIter->second--;
				if (FindIter->second == -1)
				{
					UnmanagedAssetInfo tempAssetInfo;
					tempAssetInfo.pathID = resourcesCurrentPathID;
					tempAssetInfo.name = tempAssetFileInfoEx->name;
					tempAssetInfo.offset = (int)tempAssetFileInfoEx->absolutePos;
					tempAssetInfo.size = tempAssetFileInfoEx->curFileSize;
					assetInfos.push_back(tempAssetInfo);
					AssetTypeTemplateField *tempAssetTypeTemplateField = new AssetTypeTemplateField;
					tempAssetTypeTemplateField->FromClassDatabase(classDatabaseFile, &classDatabaseFile->classes[findByClassID[tempAssetFileInfoEx->curFileType]], (DWORD)0);
					AssetTypeInstance tempAssetTypeInstance((DWORD)1, &tempAssetTypeTemplateField, AssetsReaderFromFile, (LPARAM)pResAssetsFile, resAssetsFile->header.endianness ? true : false, tempAssetFileInfoEx->absolutePos);
					AssetTypeValueField *pBase = tempAssetTypeInstance.GetBaseField();
					if (pBase) {
						AssetTypeValueField *pShader = pBase->Get("m_Shader")->Get("m_PathID");
						AssetTypeValueField *pm_TexEnvs = pBase->Get("m_SavedProperties")->Get("m_TexEnvs")->Get("Array");
						if (pm_TexEnvs && pm_TexEnvs->IsDummy() == false)
						{
							int TexEnvPosition = -1;
							int TexEnvsSize = pm_TexEnvs->GetChildrenCount();
							for (int i = 0; i < TexEnvsSize; i++)
							{
								AssetTypeValueField *pm_TexEnv = pBase->Get("m_SavedProperties")->Get("m_TexEnvs")->Get("Array")->Get((unsigned int)i)->Get("first");
								if (pm_TexEnv && pm_TexEnv->IsDummy() == false)
								{
									string TexEnvName = pm_TexEnv->GetValue()->AsString();
									if (TexEnvName == "_MainTex")
									{
										TexEnvPosition = i;
									}
								}
							}
							AssetTypeValueField *pAtlas = pBase->Get("m_SavedProperties")->Get("m_TexEnvs")->Get("Array")->Get((unsigned int)TexEnvPosition)->Get("second")->Get("m_Texture")->Get("m_PathID");

							//->Get("m_TexEnvs")->Get("Array")->Get((unsigned int)0)->Get("data")->Get("second")->Get("m_Texture")->Get("m_PathID");

							// 만약 한번에 필드를 못얻겠으면 순차적으로 확인해볼것
							//AssetTypeValueField *pAtlas = pBase->Get("m_SavedProperties");
							// //->Get("m_TexEnvs")->Get("Array")->Get((unsigned int)0)->Get("data")->Get("second")->Get("m_Texture")->Get("m_PathID");
#ifdef MY_DEBUG
							bool pShaderExist = pShader ? true : false;
							bool pAtlasExist = pAtlas ? true : false;
							bool pShaderIsDummy = pShader->IsDummy();
							bool pAtlasIsDummy = pAtlas->IsDummy();
							logOfstream << "[" << FindIter->first << "]" << "pShaderExist : " << pShaderExist << endl;
							logOfstream << "[" << FindIter->first << "]" << "pAtlasExist : " << pAtlasExist << endl;
							logOfstream << "[" << FindIter->first << "]" << "pShaderIsDummy : " << pShaderIsDummy << endl;
							logOfstream << "[" << FindIter->first << "]" << "pAtlasIsDummy : " << pAtlasIsDummy << endl;
#endif
							if (pShader && pAtlas && !pShader->IsDummy() && !pAtlas->IsDummy())
							{
								int shaderPathID = pShader->GetValue()->AsInt();
								int atlasPathID = pAtlas->GetValue()->AsInt();

								//AssetFileInfoEx *shaderAssetFileInfoEx = sharedAssetsFileTable->getAssetInfo(shaderPathID);
								UnmanagedAssetInfo shaderAssetInfo;
								shaderAssetInfo.pathID = shaderPathID;
								shaderAssetInfo.name = FindIter->first + "_Shader";
								//shaderAssetInfo.offset = (int)shaderAssetFileInfoEx->absolutePos;
								//shaderAssetInfo.size = shaderAssetFileInfoEx->curFileSize;
								assetInfos.push_back(shaderAssetInfo);

								AssetFileInfoEx *atlasAssetFileInfoEx = resAssetsFileTable->getAssetInfo(atlasPathID);
								UnmanagedAssetInfo atlasAssetInfo;
								atlasAssetInfo.pathID = atlasPathID;
								atlasAssetInfo.name = FindIter->first + "_Atlas";
								shaderAssetInfo.offset = (int)atlasAssetFileInfoEx->absolutePos;
								atlasAssetInfo.size = atlasAssetFileInfoEx->curFileSize;
								assetInfos.push_back(atlasAssetInfo);

								resourcesMaterialCount--;
								if (resourcesMaterialCount == 0)
								{
									break;
								}
							}
						}
					}
				}
			}
		}

		resourcesCurrentPathID = 440000;
		for (; resourcesCurrentPathID <= resAssetsFileTable->assetFileInfoCount; resourcesCurrentPathID++)
		{
			if (resouecesMonoBehaviourNames.size() == 0)
			{
				break;
			}
			AssetFileInfoEx *tempAssetFileInfoEx = resAssetsFileTable->getAssetInfo(resourcesCurrentPathID);
			if (tempAssetFileInfoEx->curFileType < 4294901760)
			{
				continue;
			}
			AssetTypeTemplateField *tempAssetTypeTemplateField = new AssetTypeTemplateField;
			tempAssetTypeTemplateField->FromClassDatabase(classDatabaseFile, &classDatabaseFile->classes[findByClassID[0x00000072]], (DWORD)0);
			AssetTypeInstance tempAssetTypeInstance((DWORD)1, &tempAssetTypeTemplateField, AssetsReaderFromFile, (LPARAM)pResAssetsFile, resAssetsFile->header.endianness ? true : false, tempAssetFileInfoEx->absolutePos);
			AssetTypeValueField *pBase = tempAssetTypeInstance.GetBaseField();
			if (pBase)
			{
				AssetTypeValueField *pm_Name = pBase->Get("m_Name");
				if (pm_Name && pm_Name->IsDummy() == false)
				{
					string m_Name = pm_Name->GetValue()->AsString();
#ifdef MY_DEBUG
					logOfstream << "[PathID : " << resourcesCurrentPathID << "] : " << m_Name << ", ->curFileType : " << tempAssetFileInfoEx->curFileType << endl;
#endif
					map<string, int>::iterator FindIter = resouecesMonoBehaviourNames.find(m_Name);
					if (FindIter != resouecesMonoBehaviourNames.end())
					{
						FindIter->second--;
						if (FindIter->second == -1)
						{
							UnmanagedAssetInfo tempAssetInfo;
							tempAssetInfo.pathID = resourcesCurrentPathID;
							tempAssetInfo.name = "MonoBehaviour " + m_Name;
							tempAssetInfo.offset = (int)tempAssetFileInfoEx->absolutePos;
							tempAssetInfo.size = tempAssetFileInfoEx->curFileSize;
							assetInfos.push_back(tempAssetInfo);

							AssetTypeValueField *pScript = pBase->Get("m_Script")->Get("m_PathID");

							if (pScript && pScript->IsDummy() == false)
							{
								UnmanagedAssetInfo scriptAssetInfo;
								scriptAssetInfo.pathID = pScript->GetValue()->AsInt();
								scriptAssetInfo.name = "TMP_FontAsset";
								scriptAssetInfo.offset = -1;
								scriptAssetInfo.size = -1;
								assetInfos.push_back(scriptAssetInfo);

								resouecesMonoBehaviourCount--;
								if (resouecesMonoBehaviourCount == 0)
								{
									break;
								}
							}
						}
					}
					/*free(pm_Name);
					pm_Name = 0;*/
				}
				/*free(pBase);
				pBase = 0;*/
			}
		}
	}

	//sharedAssets0
	map<string, int> sharedAssets0MaterialNames;
	int sharedAssets0MaterialCount;
	map<string, int> sharedAssets0MonoBehaviourNames;
	int sharedAssets0MonoBehaviourCount;
	
	sharedAssets0MaterialNames.insert(map<string,int>::value_type("SourceHanSansSC-Medium SDF Material",0));
	sharedAssets0MaterialCount = sharedAssets0MaterialNames.size();

	sharedAssets0MonoBehaviourNames.insert(map<string, int>::value_type("SourceHanSansSC-Medium SDF",0));
	sharedAssets0MonoBehaviourCount = sharedAssets0MonoBehaviourNames.size();
	
	
	unsigned int sharedAssetCurrentPathID = 1;
	sharedAssetCurrentPathID = 1;
	for (; sharedAssetCurrentPathID <= sharedAssetsFileTable->assetFileInfoCount; sharedAssetCurrentPathID++)
	{
		if (sharedAssets0MaterialNames.size() == 0)
		{
			break;
		}
		AssetFileInfoEx *tempAssetFileInfoEx = sharedAssetsFileTable->getAssetInfo(sharedAssetCurrentPathID);
#ifdef MY_DEBUG
		logOfstream << "[" << sharedAssetCurrentPathID << "] : " << "->name : " << tempAssetFileInfoEx->name << endl;
#endif
		map<string, int>::iterator FindIter = sharedAssets0MaterialNames.find(tempAssetFileInfoEx->name);
		if (FindIter != sharedAssets0MaterialNames.end())
		{
			FindIter->second--;
			if (FindIter->second == -1)
			{
				UnmanagedAssetInfo tempAssetInfo;
				tempAssetInfo.pathID = sharedAssetCurrentPathID;
				tempAssetInfo.name = tempAssetFileInfoEx->name;
				tempAssetInfo.offset = (int)tempAssetFileInfoEx->absolutePos;
				tempAssetInfo.size = tempAssetFileInfoEx->curFileSize;
				assetInfos.push_back(tempAssetInfo);
				AssetTypeTemplateField *tempAssetTypeTemplateField = new AssetTypeTemplateField;
				tempAssetTypeTemplateField->FromClassDatabase(classDatabaseFile, &classDatabaseFile->classes[findByClassID[tempAssetFileInfoEx->curFileType]], (DWORD)0);
				AssetTypeInstance tempAssetTypeInstance((DWORD)1, &tempAssetTypeTemplateField, AssetsReaderFromFile, (LPARAM)psharedAssetsFile, sharedAssetsFile->header.endianness?true:false, tempAssetFileInfoEx->absolutePos);
				AssetTypeValueField *pBase = tempAssetTypeInstance.GetBaseField();
				if (pBase) {
					AssetTypeValueField *pShader = pBase->Get("m_Shader")->Get("m_PathID");
					AssetTypeValueField *pm_TexEnvs = pBase->Get("m_SavedProperties")->Get("m_TexEnvs")->Get("Array");
					if(pm_TexEnvs && pm_TexEnvs->IsDummy() == false)
					{
						int TexEnvPosition = -1;
						int TexEnvsSize = pm_TexEnvs->GetChildrenCount();
						for (int i = 0; i < TexEnvsSize; i++)
						{
							AssetTypeValueField *pm_TexEnv = pBase->Get("m_SavedProperties")->Get("m_TexEnvs")->Get("Array")->Get((unsigned int)i)->Get("first");
							if (pm_TexEnv && pm_TexEnv->IsDummy() == false)
							{
								string TexEnvName = pm_TexEnv->GetValue()->AsString();
								if (TexEnvName == "_MainTex")
								{
									TexEnvPosition = i;
								}
							}
						}
						AssetTypeValueField *pAtlas = pBase->Get("m_SavedProperties")->Get("m_TexEnvs")->Get("Array")->Get((unsigned int)TexEnvPosition)->Get("second")->Get("m_Texture")->Get("m_PathID");

						//->Get("m_TexEnvs")->Get("Array")->Get((unsigned int)0)->Get("data")->Get("second")->Get("m_Texture")->Get("m_PathID");

						// 만약 한번에 필드를 못얻겠으면 순차적으로 확인해볼것
						//AssetTypeValueField *pAtlas = pBase->Get("m_SavedProperties");
						// //->Get("m_TexEnvs")->Get("Array")->Get((unsigned int)0)->Get("data")->Get("second")->Get("m_Texture")->Get("m_PathID");
#ifdef MY_DEBUG
						bool pShaderExist = pShader ? true : false;
						bool pAtlasExist = pAtlas ? true : false;
						bool pShaderIsDummy = pShader->IsDummy();
						bool pAtlasIsDummy = pAtlas->IsDummy();
						logOfstream << "[" << FindIter->first << "]" << "pShaderExist : " << pShaderExist << endl;
						logOfstream << "[" << FindIter->first << "]" << "pAtlasExist : " << pAtlasExist << endl;
						logOfstream << "[" << FindIter->first << "]" << "pShaderIsDummy : " << pShaderIsDummy << endl;
						logOfstream << "[" << FindIter->first << "]" << "pAtlasIsDummy : " << pAtlasIsDummy << endl;
#endif
						if (pShader && pAtlas && !pShader->IsDummy() && !pAtlas->IsDummy())
						{
							int shaderPathID = pShader->GetValue()->AsInt();
							int atlasPathID = pAtlas->GetValue()->AsInt();

							//AssetFileInfoEx *shaderAssetFileInfoEx = sharedAssetsFileTable->getAssetInfo(shaderPathID);
							UnmanagedAssetInfo shaderAssetInfo;
							shaderAssetInfo.pathID = shaderPathID;
							shaderAssetInfo.name = FindIter->first + "_Shader";
							//shaderAssetInfo.offset = (int)shaderAssetFileInfoEx->absolutePos;
							//shaderAssetInfo.size = shaderAssetFileInfoEx->curFileSize;
							assetInfos.push_back(shaderAssetInfo);

							AssetFileInfoEx *atlasAssetFileInfoEx = sharedAssetsFileTable->getAssetInfo(atlasPathID);
							UnmanagedAssetInfo atlasAssetInfo;
							atlasAssetInfo.pathID = atlasPathID;
							atlasAssetInfo.name = FindIter->first + "_Atlas";
							shaderAssetInfo.offset = (int)atlasAssetFileInfoEx->absolutePos;
							atlasAssetInfo.size = atlasAssetFileInfoEx->curFileSize;
							assetInfos.push_back(atlasAssetInfo);

							sharedAssets0MaterialCount--;
							if (sharedAssets0MaterialCount == 0)
							{
								break;
							}
						}
					}		
				}
			}
		}
	}

	for (; sharedAssetCurrentPathID <= sharedAssetsFileTable->assetFileInfoCount; sharedAssetCurrentPathID++)
	{
		if (sharedAssets0MonoBehaviourNames.size() == 0)
		{
			break;
		}
		AssetFileInfoEx *tempAssetFileInfoEx = sharedAssetsFileTable->getAssetInfo(sharedAssetCurrentPathID);
		if (tempAssetFileInfoEx->curFileType < 4294901760)
		{
			continue;
		}
		AssetTypeTemplateField *tempAssetTypeTemplateField = new AssetTypeTemplateField;
		tempAssetTypeTemplateField->FromClassDatabase(classDatabaseFile, &classDatabaseFile->classes[findByClassID[0x00000072]], (DWORD)0);
		AssetTypeInstance tempAssetTypeInstance((DWORD)1, &tempAssetTypeTemplateField, AssetsReaderFromFile, (LPARAM)psharedAssetsFile, sharedAssetsFile->header.endianness ? true : false, tempAssetFileInfoEx->absolutePos);
		AssetTypeValueField *pBase = tempAssetTypeInstance.GetBaseField();
		if (pBase)
		{
			AssetTypeValueField *pm_Name = pBase->Get("m_Name");
			if (pm_Name && pm_Name->IsDummy() == false)
			{
				string m_Name = pm_Name->GetValue()->AsString();
#ifdef MY_DEBUG
				logOfstream << "[PathID : " << sharedAssetCurrentPathID << "] : " << m_Name << ", ->curFileType : " << tempAssetFileInfoEx->curFileType <<  endl;
#endif
				map<string, int>::iterator FindIter = sharedAssets0MonoBehaviourNames.find(m_Name);
				if (FindIter != sharedAssets0MonoBehaviourNames.end())
				{
					FindIter->second--;
					if (FindIter->second == -1)
					{
						UnmanagedAssetInfo tempAssetInfo;
						tempAssetInfo.pathID = sharedAssetCurrentPathID;
						tempAssetInfo.name = "MonoBehaviour " +  m_Name;
						tempAssetInfo.offset = (int)tempAssetFileInfoEx->absolutePos;
						tempAssetInfo.size = tempAssetFileInfoEx->curFileSize;
						assetInfos.push_back(tempAssetInfo);

						AssetTypeValueField *pScript = pBase->Get("m_Script")->Get("m_PathID");

						if (pScript && pScript->IsDummy() == false)
						{
							UnmanagedAssetInfo scriptAssetInfo;
							scriptAssetInfo.pathID = pScript->GetValue()->AsInt();
							scriptAssetInfo.name = "TMP_FontAsset";
							scriptAssetInfo.offset = -1;
							scriptAssetInfo.size = -1;
							assetInfos.push_back(scriptAssetInfo);

							sharedAssets0MonoBehaviourCount--;
							if (sharedAssets0MonoBehaviourCount == 0)
							{
								break;
							}
						}
					}
				}
				/*free(pm_Name);
				pm_Name = 0;*/
			}
			/*free(pBase);
			pBase = 0;*/
		}	
	}
}

vector<UnmanagedAssetInfo> unmanagedPatcher::GetAssetInfos()
{
	return this->assetInfos;
}

void unmanagedPatcher::MakeModdedAssets()
{
	// sharedPatch
	string sharedPatchListFilePath = "temp\\sharedassets0_patch\\sharedassets0_patch_list.txt";

	ifstream ifsSharedPatchListFile(sharedPatchListFilePath);
	std::vector<string> sharedPatchFileName;
	if (!ifsSharedPatchListFile.is_open())
	{
		cout << "cannot open patchFileList text file" << endl;
		cout << "Exit" << endl;
		return;
	}
#ifdef MY_DEBUG
	logOfstream << "Patch File List : " << endl;
#endif
	std::vector<FILE*> pSharedPatchFile;
	while (!ifsSharedPatchListFile.eof())
	{
		string temp;
		FILE *pTempPatchFile = NULL;
		ifsSharedPatchListFile >> temp;
		if (temp == "")
		{
			continue;
		}
		sharedPatchFileName.push_back(temp);
#ifdef MY_DEBUG
		logOfstream << sharedPatchFileName[sharedPatchFileName.size() - 1] << endl;
#endif
		fopen_s(&pTempPatchFile, temp.c_str(), "rb");
		if (pTempPatchFile == NULL)
		{
			cout << "cannot open patch file : " << temp << endl;
			cout << "Exit" << endl;
			fclose(pTempPatchFile);
			for (int i = 0; i < (int)pSharedPatchFile.size(); i++)
			{
				fclose(pSharedPatchFile[i]);
			}
			return;
		}
		pSharedPatchFile.push_back(pTempPatchFile);
	}
	ifsSharedPatchListFile.close();

	string sharedModdedFilePath = _gameFolderPath + sharedAssetsFileName + ".modded";
	std::vector<AssetsReplacer*> sharedReplacors;
	std::vector<AssetFileInfoEx*> sharedAssetsFileInfos;

	for (unsigned int i = 0; i < sharedPatchFileName.size(); i++)
	{
		int tempPathID = FindPathID(sharedPatchFileName[i]);
		sharedAssetsFileInfos.push_back(sharedAssetsFileTable->getAssetInfo(tempPathID)); // I know the ID - no need to search
		sharedReplacors.push_back(MakeAssetModifierFromFile(0, (*sharedAssetsFileInfos[i]).index, (*sharedAssetsFileInfos[i]).curFileType, (*sharedAssetsFileInfos[i]).inheritedUnityClass,
			pSharedPatchFile[i], 0, (QWORD)filesize(pSharedPatchFile[i]))); // I expect that the size parameter refers to the file size but I couldn't check this until now
	}
	FILE *pModdedSharedFile;
	fopen_s(&pModdedSharedFile, (sharedModdedFilePath).c_str(), "wb");
	sharedAssetsFile->Write(AssetsWriterToFile, (LPARAM)pModdedSharedFile, 0, sharedReplacors.data(), sharedReplacors.size(), 0);

	for (unsigned int i = 0; i < pSharedPatchFile.size(); i++)
	{
		if (pSharedPatchFile[i])
		{
			fclose(pSharedPatchFile[i]);
			pSharedPatchFile[i] = NULL;
		}
	}
	if (pModdedSharedFile)
	{
		fclose(pModdedSharedFile);
		pModdedSharedFile = NULL;
	}
	//////////////////////////////////////////////
	// resources
	//////////////////////////////////////////////
	string resPatchListFilePath = "temp\\resources_patch\\resources_patch_list.txt";

	ifstream ifsResPatchListFile(resPatchListFilePath);
	std::vector<string> resPatchFileName;
	if (!ifsResPatchListFile.is_open())
	{
		cout << "cannot open patchFileList text file" << endl;
		cout << "Exit" << endl;
		return;
	}
#ifdef MY_DEBUG
	logOfstream << "Patch File List : " << endl;
#endif
	std::vector<FILE*> pResPatchFile;
	while (!ifsResPatchListFile.eof())
	{
		string temp;
		FILE *pTempPatchFile = NULL;
		ifsResPatchListFile >> temp;
		if (temp == "")
		{
			continue;
		}
		resPatchFileName.push_back(temp);
#ifdef MY_DEBUG
		logOfstream << resPatchFileName[resPatchFileName.size() - 1] << endl;
#endif
		fopen_s(&pTempPatchFile, temp.c_str(), "rb");
		if (pTempPatchFile == NULL)
		{
			cout << "cannot open patch file : " << temp << endl;
			cout << "Exit" << endl;
			fclose(pTempPatchFile);
			for (int i = 0; i < (int)pResPatchFile.size(); i++)
			{
				fclose(pResPatchFile[i]);
			}
			return;
		}
		pResPatchFile.push_back(pTempPatchFile);
	}
	ifsResPatchListFile.close();

	string resModdedFilePath = _gameFolderPath + resAssetsFileName + ".modded";
	std::vector<AssetsReplacer*> resReplacors;
	std::vector<AssetFileInfoEx*> resAssetsFileInfos;

	for (unsigned int i = 0; i < resPatchFileName.size(); i++)
	{
		int tempPathID = FindPathID(resPatchFileName[i]);
		resAssetsFileInfos.push_back(resAssetsFileTable->getAssetInfo(tempPathID)); // I know the ID - no need to search
		resReplacors.push_back(MakeAssetModifierFromFile(0, (*resAssetsFileInfos[i]).index, (*resAssetsFileInfos[i]).curFileType, (*resAssetsFileInfos[i]).inheritedUnityClass,
			pResPatchFile[i], 0, (QWORD)filesize(pResPatchFile[i]))); // I expect that the size parameter refers to the file size but I couldn't check this until now
	}
	FILE *pModdedResFile;
	fopen_s(&pModdedResFile, (resModdedFilePath).c_str(), "wb");
	resAssetsFile->Write(AssetsWriterToFile, (LPARAM)pModdedResFile, 0, resReplacors.data(), resReplacors.size(), 0);

	for (unsigned int i = 0; i < pResPatchFile.size(); i++)
	{
		if (pResPatchFile[i])
		{
			fclose(pResPatchFile[i]);
			pResPatchFile[i] = NULL;
		}
	}
	if (pModdedResFile)
	{
		fclose(pModdedResFile);
		pModdedResFile = NULL;
	}

#ifdef MY_DEBUG
	logOfstream << "Slime Rancher Korean Translation Patch Complete. Exit" << endl;
#endif
}


UnmanagedAssetInfo::UnmanagedAssetInfo()
{

}

UnmanagedAssetInfo::UnmanagedAssetInfo(int pathID, string name, int offset, int size)
{
	this->pathID = pathID;
	this->name = name;
	this->offset = offset;
	this->size = size;
}