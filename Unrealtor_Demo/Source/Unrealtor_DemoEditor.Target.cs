using UnrealBuildTool;

public class Unrealtor_DemoEditorTarget : TargetRules
{
	public Unrealtor_DemoEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_6;
		ExtraModuleNames.Add("Unrealtor_Demo");
	}
}
