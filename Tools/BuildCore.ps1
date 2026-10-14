# KingdomCore를 UE 없이 CMake로 빌드하고 테스트를 돌린다.
# Visual Studio 개발자 환경을 직접 불러오므로 일반 PowerShell에서 실행해도 된다.

. (Join-Path $PSScriptRoot 'Environment.ps1')

Import-VisualStudioEnvironment
Set-Location $RepositoryRoot

$Steps = @(
	@{ Name = 'CMake 구성'; Command = { cmake -S Source/KingdomCore -B Intermediate/CMake -G Ninja -DCMAKE_BUILD_TYPE=Debug } },
	@{ Name = '빌드'; Command = { cmake --build Intermediate/CMake } },
	@{ Name = '테스트'; Command = { & (Join-Path $RepositoryRoot 'Intermediate\CMake\KingdomCoreTests.exe') } }
)

foreach ($Step in $Steps)
{
	Write-Kingdom $Step.Name
	& $Step.Command
	if ($LASTEXITCODE -ne 0)
	{
		Write-Kingdom "$($Step.Name) 실패: $LASTEXITCODE" Red
		exit $LASTEXITCODE
	}
}

Write-Kingdom '코어 빌드와 테스트 통과.' Green
