#include "LibzipArchiver.h"
#include "zip.h"
#include "zipint.h"
#include "Misc/Paths.h"
#include "HAL/PlatformFilemanager.h"

ULibzipArchiver::~ULibzipArchiver()
{
	CloseArchive();
}

bool ULibzipArchiver::GetRelativeFilesInDirectory(FString Dir, bool bAddParentDirectory, TArray<FString>& FilePaths)
{
	FPaths::NormalizeDirectoryName(Dir);

	if (!FPaths::DirectoryExists(Dir))
	{
		UE_LOG(LogTemp, Error, TEXT("NotFoundDirectory:%s"), *Dir);
		return false;
	}

	const FString BasePath{ FPaths::GetPath(Dir) };
	const FString BaseDirToExclude = BasePath.IsEmpty() ? TEXT("") :
		((bAddParentDirectory ? BasePath : BasePath + TEXT("/") + FPaths::GetCleanFilename(Dir)) + TEXT("/"));


	class FDirScanner : public IPlatformFile::FDirectoryVisitor
	{
		const FString& BaseDirToExclude;
		TArray<FString>& FilePaths;

	public:
		FDirScanner(const FString& BaseDirectoryPathToExclude, TArray<FString>& FilePaths)
			: BaseDirToExclude(BaseDirectoryPathToExclude),
			FilePaths(FilePaths)
		{
		}

		virtual bool Visit(const TCHAR* FilenameOrDirectory, bool bIsDirectory) override
		{
			if (bIsDirectory)
			{
				return true;
			}

			const FString EntryName{ FString(FilenameOrDirectory).RightChop(BaseDirToExclude.Len()) };

			FilePaths.Add(EntryName);

			return true;
		}
	};

	FDirScanner DirScanner(BaseDirToExclude, FilePaths);

	return FPlatformFileManager::Get().GetPlatformFile().IterateDirectoryRecursively(*Dir, DirScanner);
}

bool ULibzipArchiver::OpenArchiveFromStorage(const FString& ArchivePath)
{
	int errorp;
	Zipper = zip_open(TCHAR_TO_UTF8(*ArchivePath), ZIP_RDONLY, &errorp);
	if (Zipper == NULL)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to zip_open %d"), errorp);
		return false;
	}

	return true;
}

bool ULibzipArchiver::CreateArchiveFromStorage(const FString& ArchivePath)
{
	int errorp;
	Zipper = zip_open(TCHAR_TO_UTF8(*ArchivePath), ZIP_CREATE | ZIP_EXCL, &errorp);
	if (Zipper == NULL)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to zip_open %d"), errorp);
		return false;
	}

	return true;
}

bool ULibzipArchiver::OpenEncryptedArchiveFromStorage(const FString& ArchivePath, const FString& ArchivePassword)
{
	Password = ArchivePassword;
	return OpenArchiveFromStorage(ArchivePath);
}

bool ULibzipArchiver::CreateEncryptedArchiveFromStorage(const FString& ArchivePath, const FString& ArchivePassword)
{
	Password = ArchivePassword;
	return CreateArchiveFromStorage(ArchivePath);
}

bool ULibzipArchiver::CloseArchive()
{
	if (Zipper != NULL)
	{
		if (zip_close(Zipper) < 0)
		{
			WriteArchiveErrLog("Failed to zip_close");
			return false;
		}
		Zipper = NULL;
	}
	Password = "";

	return true;
}

bool ULibzipArchiver::AddEntryFromStorage(const FString& EntryName, const FString& FilePath)
{
	zip_source_t* Source = zip_source_file(Zipper, TCHAR_TO_UTF8(*FilePath), 0, 0);
	if (Source == NULL)
	{
		WriteArchiveErrLog("Failed to zip_source_file");
		return false;
	}

	zip_int64_t Index = zip_file_add(Zipper, TCHAR_TO_UTF8(*EntryName), Source, ZIP_FL_ENC_UTF_8);
	if (Index < 0)
	{
		WriteArchiveErrLog("Failed to zip_file_add");
		return false;
	}

	if (!Password.IsEmpty())
	{
		int result = zip_file_set_encryption(Zipper, Index, ZIP_EM_AES_256, TCHAR_TO_UTF8(*Password));
		if (result < 0)
		{
			WriteArchiveErrLog("Failed to zip_file_set_encryption");
			return false;
		}
	}

	return true;
}

int64 ULibzipArchiver::GetArchiveEntries()
{
	zip_int64_t NumEntries = zip_get_num_entries(Zipper, 0);
	if (NumEntries < 0)
	{
		WriteArchiveErrLog("Failed to zip_get_num_entries");
	}

	return NumEntries;
}

void ULibzipArchiver::WriteArchiveErrLog(const FString& BaseMessage)
{
	if (Zipper != NULL)
	{
		UE_LOG(LogTemp, Error, TEXT("%s %d %d %s"), *BaseMessage, Zipper->error.zip_err, Zipper->error.sys_err, UTF8_TO_TCHAR(Zipper->error.str));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s"), *BaseMessage);
	}
	
}

bool ULibzipArchiver::GetEntryToMemory(int64 Index, FString& Name, TArray<uint8>& Data)
{
	struct zip_stat sb;
	int result = zip_stat_index(Zipper, Index, 0, &sb);
	if (result < 0)
	{
		WriteArchiveErrLog("Failed to zip_file_set_encryption");
		return false;
	}

	Name = UTF8_TO_TCHAR(sb.name);
	Data.SetNumUninitialized(sb.size, true);
	TSharedPtr<zip_file> Zf(Password.IsEmpty() ? zip_fopen(Zipper, sb.name, 0) : zip_fopen_encrypted(Zipper, sb.name, 0, TCHAR_TO_UTF8(*Password)), [](zip_file* zipfile) {
		if (zipfile) { zip_fclose(zipfile); }
		});
	if (!Zf.IsValid())
	{
		WriteArchiveErrLog("Failed to zip_fopen");
		return false;
	}
	zip_int64_t ReadByte = zip_fread(Zf.Get(), Data.GetData(), sb.size);
	if (ReadByte < 0)
	{
		WriteArchiveErrLog("Failed to zip_fread");
		return false;
	}

	return true;
}

bool ULibzipArchiver::WriteEntryToStorage(int64 Index, const FString& BaseDir)
{
	FString Name;
	TArray<uint8> Contents;
	if (!GetEntryToMemory(Index, Name, Contents))
	{
		return false;
	}

	FArchive* Archive = IFileManager::Get().CreateFileWriter(*FPaths::Combine(BaseDir, Name));
	if (Archive == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to create file"))
		return false;
	}
	TUniquePtr<FArchive> FileWriter(Archive);
	FileWriter->Serialize(Contents.GetData(), Contents.Num());
	return true;
}