# 에디터 타깃(KingdomEditor, Win64, Development)을 빌드한다.
# 에디터가 켜져 있으면 Live Coding과 충돌할 수 있으니 에디터를 닫고 실행한다.

. (Join-Path $PSScriptRoot 'Environment.ps1')

$BuildBatch = Join-Path $UnrealRoot 'Engine\Build\BatchFiles\Build.bat'
& $BuildBatch KingdomEditor Win64 Development "-project=$ProjectFile" -waitmutex
if ($LASTEXITCODE -ne 0)
{
	Write-Kingdom "실패: $LASTEXITCODE" Red
	exit $LASTEXITCODE
}

Write-Kingdom '에디터 빌드 완료.' Green
