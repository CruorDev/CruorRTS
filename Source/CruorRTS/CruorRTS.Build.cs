using UnrealBuildTool;

public class CruorRTS : ModuleRules
{
    public CruorRTS(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core", 
                "GameplayAbilities",
                "GameplayTags",
                "GameplayTasks", 
                "MassCore",
                "MassCommon",
                "MassEntity",
                "MassGameplayDebug",
                "MassLOD",
                "MassMovement",
                "MassRepresentation",
                "MassSpawner",
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore", 
                "AnimToTexture"
            }
        );
    }
}