# 다른 스크립트가 dot-source(. "$PSScriptRoot\Environment.ps1")로 불러 쓰는 공통 경로 설정.
# 엔진 경로가 다르면 환경 변수 KINGDOM_UE_ROOT를 설정해 두면 그 값을 쓴다.

$ErrorActionPreference = 'Stop'

$UnrealRoot = if ($env:KINGDOM_UE_ROOT) { $env:KINGDOM_UE_ROOT } else { 'C:\Program Files\Epic Games\UE_5.8' }
$RepositoryRoot = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
$ProjectFile = Join-Path $RepositoryRoot 'Kingdom.uproject'
$UnrealBuildTool = Join-Path $UnrealRoot 'Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.exe'

if (-not (Test-Path $UnrealBuildTool))
{
	throw "엔진을 찾지 못했습니다: $UnrealRoot (KINGDOM_UE_ROOT 환경 변수로 엔진 경로를 지정하세요)"
}

function Write-Kingdom([string] $Message, [ConsoleColor] $Color = 'Cyan')
{
	Write-Host "[Kingdom] $Message" -ForegroundColor $Color
}

# Visual Studio 개발자 환경(cl, cmake, ninja)을 현재 PowerShell 세션으로 가져온다.
# vcvars64.bat을 cmd에서 실행한 뒤 바뀐 환경 변수를 그대로 옮긴다.
function Import-VisualStudioEnvironment
{
	$VsWhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
	$VisualStudioRoot = & $VsWhere -latest -property installationPath
	if (-not $VisualStudioRoot)
	{
		throw 'Visual Studio를 찾지 못했습니다.'
	}

	# vcvars64.bat이 내부에서 vswhere를 찾으므로 PATH에 넣어 준다(경고 메시지 방지)
	$env:PATH = "$(Split-Path $VsWhere);$env:PATH"

	$VcVars = Join-Path $VisualStudioRoot 'VC\Auxiliary\Build\vcvars64.bat'
	$Lines = & cmd.exe /c "`"$VcVars`" >nul 2>&1 && set"
	foreach ($Line in $Lines)
	{
		$Index = $Line.IndexOf('=')
		if ($Index -gt 0)
		{
			[Environment]::SetEnvironmentVariable($Line.Substring(0, $Index), $Line.Substring($Index + 1), 'Process')
		}
	}
}
