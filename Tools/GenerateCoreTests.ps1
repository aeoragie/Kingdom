# 코어 테스트용 VS 솔루션(Intermediate/CMakeVS)을 만들고 연다.
# 코어에 .cpp를 더하거나 CMakeLists.txt를 고친 뒤에도 다시 실행한다.

. (Join-Path $PSScriptRoot 'Environment.ps1')
Import-VisualStudioEnvironment
Set-Location $RepositoryRoot

$BuildDirectory = Join-Path $RepositoryRoot 'Intermediate\CMakeVS'

cmake -S Source/KingdomCore -B $BuildDirectory -G 'Visual Studio 18 2026' -A x64

if ($LASTEXITCODE -ne 0)
{
	Write-Kingdom "실패: $LASTEXITCODE" Red
	exit $LASTEXITCODE
}

cmake --open $BuildDirectory
Write-Kingdom '코어 테스트 솔루션을 열었습니다. F5로 테스트를 실행합니다.' Green
