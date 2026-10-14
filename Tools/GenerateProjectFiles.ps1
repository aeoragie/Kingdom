# VS 솔루션(Kingdom.sln / Kingdom.slnx)을 다시 생성한다.
# 소스 파일을 추가·삭제·이름 변경한 뒤에 실행하고, VS에서 Reload All.

. (Join-Path $PSScriptRoot 'Environment.ps1')

& $UnrealBuildTool -projectfiles "-project=$ProjectFile" -game -rocket -progress
if ($LASTEXITCODE -ne 0)
{
	Write-Kingdom "실패: $LASTEXITCODE" Red
	exit $LASTEXITCODE
}

Write-Kingdom '프로젝트 파일 생성 완료. VS에서 Reload All 하세요.' Green
