#include "LibzipArchiver.h"
#include "Misc/AutomationTest.h"
#include "HAL/PlatformFilemanager.h"

BEGIN_DEFINE_SPEC(Archive, "LibzipArchiver.Archive", EAutomationTestFlags::ProductFilter | EAutomationTestFlags::ApplicationContextMask)
	void ArchiveFilesTest(const FString& ZipPath, const FString& Password, const TMap<FString, FString>& EntryAndFilePaths);
	void UnarchiveFilesTest(const FString& ZipPath, const FString& OutDir, const FString& Password, const TArray<FString>& UnarchiveFiles);

    UPROPERTY(Transient)
	ULibzipArchiver* Archiver;
	FString TempDirPath;
	IPlatformFile& FileManager = FPlatformFileManager::Get().GetPlatformFile();
END_DEFINE_SPEC(Archive)

void Archive::ArchiveFilesTest(const FString& ZipPath, const FString& Password, const TMap<FString, FString>& EntryAndFilePaths)
{
	bool bCreateResult = Password.IsEmpty() ? Archiver->CreateArchiveFromStorage(ZipPath) : 
		Archiver->CreateEncryptedArchiveFromStorage(ZipPath, Password);
	TestTrue("create archive", bCreateResult);
	for (auto EntryAndFilePath : EntryAndFilePaths)
	{
		bool bAddResult = Archiver->AddEntryFromStorage(FPaths::GetCleanFilename(EntryAndFilePath.Key), EntryAndFilePath.Value);
		TestTrue("add archive", bAddResult);
	}

	bool bWriteCloseResult = Archiver->CloseArchive();
	TestTrue("close archive", bWriteCloseResult);
}

void Archive::UnarchiveFilesTest(const FString& ZipPath, const FString& OutDir, const FString& Password, const TArray<FString>& UnarchiveFiles)
{
	bool bOpenResult = Password.IsEmpty() ? Archiver->OpenArchiveFromStorage(ZipPath) : 
		Archiver->OpenEncryptedArchiveFromStorage(ZipPath, Password);
	TestTrue("open archive", bOpenResult);
	TestEqual("archive entry number", FString::Printf(TEXT("%lld"), Archiver->GetArchiveEntries()), FString::FromInt(UnarchiveFiles.Num()));
	bool bWriteResult = Archiver->WriteEntryToStorage(0, OutDir);
	TestTrue("write entry", bWriteResult);
	bool bReadCloseResult = Archiver->CloseArchive();
	TestTrue("close archive", bReadCloseResult);
	for (auto File : UnarchiveFiles)
	{
		TestTrue("unarchive file exist", FPaths::FileExists(File));
		TestTrue("unarchive file size", FileManager.FileSize(*File) > 0);
	}

}

void Archive::Define()
{
	Describe("archive files", [this]() {
		BeforeEach([this]() {
			TempDirPath = FPaths::Combine(FPaths::ProjectSavedDir(), "temp", "ArchiveSpec");
			if (FPaths::DirectoryExists(TempDirPath))
			{
				FileManager.DeleteDirectoryRecursively(*TempDirPath);
			}
			FileManager.CreateDirectory(*TempDirPath);
			Archiver = NewObject<ULibzipArchiver>(ULibzipArchiver::StaticClass());
		});

		It("should archive and unarchive", [this]() {
			FString TargetFilePath = FPaths::Combine(FPaths::ProjectPluginsDir(), 
				"LibzipArchiver", "Source", "ThirdParty", "libzip", "lib", "Win64", "libzip-static.lib");
			FString TargetFileName = FPaths::GetCleanFilename(TargetFilePath);
			FString OutZipPath = FPaths::Combine(TempDirPath, "test.zip");

			// archive
			ArchiveFilesTest(OutZipPath, "", { { TargetFileName, TargetFilePath } });

			// unarchive
			UnarchiveFilesTest(OutZipPath, TempDirPath, "", { FPaths::Combine(TempDirPath, TargetFileName) });

		});

		It("should archive and unarchive with password", [this]() {
			FString TargetFilePath = FPaths::Combine(FPaths::ProjectPluginsDir(),
				"LibzipArchiver", "Source", "ThirdParty", "libzip", "lib", "Win64", "libzip-static.lib");
			FString TargetFileName = FPaths::GetCleanFilename(TargetFilePath);
			FString OutZipPath = FPaths::Combine(TempDirPath, "test.zip");
			FString Password = "Password";

			// archive
			ArchiveFilesTest(OutZipPath, Password, { { TargetFileName, TargetFilePath } });

			// unarchive
			UnarchiveFilesTest(OutZipPath, TempDirPath, Password, { FPaths::Combine(TempDirPath, TargetFileName) });
		});

		It("should not unarchive with invalid password", [this]() {
			FString TargetFilePath = FPaths::Combine(FPaths::ProjectPluginsDir(),
				"LibzipArchiver", "Source", "ThirdParty", "libzip", "lib", "Win64", "libzip-static.lib");
			FString TargetFileName = FPaths::GetCleanFilename(TargetFilePath);
			FString OutZipPath = FPaths::Combine(TempDirPath, "test.zip");
			FString Password = "Password";
			FString InvalidPassword = "test";

			// archive
			ArchiveFilesTest(OutZipPath, Password, { { TargetFileName, TargetFilePath } });

			// unarchive
			bool bOpenResult = Archiver->OpenEncryptedArchiveFromStorage(OutZipPath, InvalidPassword);
			TestTrue("open archive", bOpenResult);
			TestEqual("archive entry number", FString::Printf(TEXT("%lld"), Archiver->GetArchiveEntries()), "1");
			AddExpectedError("Failed to zip_fopen", EAutomationExpectedErrorFlags::Contains, 0);
			bool bWriteResult = Archiver->WriteEntryToStorage(0, TempDirPath);
			TestFalse("write archive", bWriteResult);
			bool bReadCloseResult = Archiver->CloseArchive();
			TestTrue("close archive", bReadCloseResult);
		});

		AfterEach([this]() {
			if (FPaths::DirectoryExists(TempDirPath))
			{
				FileManager.DeleteDirectoryRecursively(*TempDirPath);
			}
		});
	});
}