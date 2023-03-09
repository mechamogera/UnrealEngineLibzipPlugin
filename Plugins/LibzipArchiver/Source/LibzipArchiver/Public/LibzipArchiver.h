#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "LibzipArchiver.generated.h"

struct zip;

UCLASS(Blueprintable)
class LIBZIPARCHIVER_API ULibzipArchiver : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
		static bool GetRelativeFilesInDirectory(FString DirectoryPath, bool bAddParentDirectory, TArray<FString>& FilePaths);

public:
	UFUNCTION(BlueprintCallable)
		bool OpenArchiveFromStorage(const FString& ArchivePath);

	UFUNCTION(BlueprintCallable)
		bool CreateArchiveFromStorage(const FString& ArchivePath);

	UFUNCTION(BlueprintCallable)
		bool OpenEncryptedArchiveFromStorage(const FString& ArchivePath, const FString& Password);

	UFUNCTION(BlueprintCallable)
		bool CreateEncryptedArchiveFromStorage(const FString& ArchivePath, const FString& Password);

	UFUNCTION(BlueprintCallable)
		bool CloseArchive();

	UFUNCTION(BlueprintCallable)
		bool AddEntryFromStorage(const FString& EntryName, const FString& FilePath);

	UFUNCTION(BlueprintCallable)
		int64 GetArchiveEntries();

	UFUNCTION(BLueprintCallable)
		bool GetEntryToMemory(int64 Index, FString& Name, TArray<uint8>& Data);

	UFUNCTION(BLueprintCallable)
		bool WriteEntryToStorage(int64 Index, const FString& BaseDir);

protected:
	void WriteArchiveErrLog(const FString& BaseMessage);

protected:
	zip* Zipper;
	FString Password;
};
