#include "Modules/ModuleManager.h"

#include "KingdomCore/Version.h"

DEFINE_LOG_CATEGORY_STATIC(LogKingdom, Log, All);

class FKingdomGameModule : public FDefaultGameModuleImpl
{
	virtual void StartupModule() override
	{
		UE_LOG(LogKingdom, Log, TEXT("KingdomCore version: %hs"), Kingdom::CoreVersion());
	}
};

IMPLEMENT_PRIMARY_GAME_MODULE(FKingdomGameModule, KingdomGame, "KingdomGame");
