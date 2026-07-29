#include "Application/GameApplication.h"
#include "Core/RandomProvider.h"
#include "Platform/Console/ConsoleInput.h"
#include "Platform/Console/ConsolePresenter.h"
#include "Platform/Console/ConsoleSession.h"

int main()
{
    // 플랫폼 자원은 main의 수명 동안 유지하고 애플리케이션에 구현체를 주입한다.
    ss::ConsoleSession consoleSession;
    ss::ConsoleInput input;
    ss::ConsolePresenter presenter;
    ss::RandomProvider randomProvider;

    ss::GameApplication application(input, presenter, randomProvider);
    return application.Run();
}
