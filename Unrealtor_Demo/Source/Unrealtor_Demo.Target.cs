using UnrealBuildTool;

public class Unrealtor_DemoTarget : TargetRules
{
	public Unrealtor_DemoTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_6;
		ExtraModuleNames.Add("Unrealtor_Demo");
	}
}
