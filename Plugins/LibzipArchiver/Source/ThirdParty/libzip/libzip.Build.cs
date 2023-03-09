using System.IO;
using UnrealBuildTool;

public class LibZip : ModuleRules
{
    public LibZip(ReadOnlyTargetRules Target) : base(Target)
    {
        Type = ModuleType.External;

        PublicIncludePaths.AddRange(
            new string[] {
                Path.Combine(ModuleDirectory, "include"),
            }
        );

        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "lib", "Win64", "libzip-static.lib"));
            PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "lib", "Win64", "libz-static.lib"));
        }
    }
}