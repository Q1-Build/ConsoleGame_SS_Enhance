#include "Game/Domain/PlayerProgress.h"

#include <cassert>

namespace ss
{
    const Sword& PlayerProgress::GetSword() const noexcept
    {
        return sword_;
    }

    int PlayerProgress::GetGold() const noexcept
    {
        return gold_;
    }

    int PlayerProgress::GetFragments() const noexcept
    {
        return fragments_;
    }

    int PlayerProgress::GetAttemptCount() const noexcept
    {
        return attemptCount_;
    }

    int PlayerProgress::GetSuccessCount() const noexcept
    {
        return successCount_;
    }

    int PlayerProgress::GetBossVictoryCount() const noexcept
    {
        return bossVictoryCount_;
    }

    bool PlayerProgress::CanAfford(int amount) const noexcept
    {
        return amount >= 0 && gold_ >= amount;
    }

    bool PlayerProgress::SpendGold(int amount)
    {
        assert(amount >= 0);

        // 검사와 차감을 한 메서드에서 처리해 음수 재화 상태가 만들어지지 않게 한다.
        if (!CanAfford(amount))
        {
            return false;
        }

        gold_ -= amount;
        return true;
    }

    void PlayerProgress::GrantGold(int amount)
    {
        assert(amount >= 0);
        gold_ += amount;
    }

    bool PlayerProgress::ConsumeFragment()
    {
        // 조각이 없을 때는 상태를 바꾸지 않고 호출자가 다음 실패 규칙을 선택하게 한다.
        if (fragments_ <= 0)
        {
            return false;
        }

        --fragments_;
        return true;
    }

    void PlayerProgress::GrantFragments(int amount)
    {
        assert(amount >= 0);
        fragments_ += amount;
    }

    void PlayerProgress::RecordAttempt() noexcept
    {
        ++attemptCount_;
    }

    void PlayerProgress::RecordSuccess() noexcept
    {
        ++successCount_;
    }

    void PlayerProgress::RecordBossVictory() noexcept
    {
        // 현재 수직 슬라이스의 세 보스를 모두 처치한 뒤에는 엔딩 상태를 유지한다.
        if (bossVictoryCount_ < 3)
        {
            ++bossVictoryCount_;
        }
    }

    void PlayerProgress::EnhanceSword(int levelCount)
    {
        sword_.Enhance(levelCount);
    }

    void PlayerProgress::DowngradeSword()
    {
        sword_.LoseLevel();
    }
}
