#include "LibzipArchiver.h"
#include "Misc/AutomationTest.h"

BEGIN_DEFINE_SPEC(GetRelativeFilesInDir, "LibzipArchiver.Library", EAutomationTestFlags::ProductFilter | EAutomationTestFlags::ApplicationContextMask)
END_DEFINE_SPEC(GetRelativeFilesInDir)
void GetRelativeFilesInDir::Define()
{
	Describe("get relative files in directory", [this]() {
		It("should return filepaths", [this]() {
			FString Dir = FPaths::Combine(FPaths::ProjectPluginsDir(), "LibzipArchiver", "Source", "ThirdParty", "libzip", "lib");
			TArray<FString> FilePaths;
			bool bResult = ULibzipArchiver::GetRelativeFilesInDirectory(Dir, false, FilePaths);
			TestTrue("get relative file result", bResult);
			TestEqual("get realative files number", 2, FilePaths.Num());
			TestEqual("get relative 1st file", "Win64/libz-static.lib", FilePaths[0]);
			TestEqual("get relative 2nd file", "Win64/libzip-static.lib", FilePaths[1]);
		});
	});

	Describe("get relative files in directory", [this]() {
		It("should return false when invaild file specified", [this]() {
			AddExpectedError("NotFoundDirectory", EAutomationExpectedErrorFlags::Contains, 0);
			TArray<FString> FilePaths;
			bool bResult = ULibzipArchiver::GetRelativeFilesInDirectory("hoge", true, FilePaths);
			TestFalse("get relative file result", bResult);
		});
	});
}