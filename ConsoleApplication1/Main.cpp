#define NOMINMAX
#include <Windows.h>
#include <conio.h>

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <iomanip>
#include <random>
#include <sstream>
#include <string>
#include <vector>

namespace
{
    constexpr int SCREEN_WIDTH = 104;
    constexpr int SCREEN_HEIGHT = 34;
    constexpr float PI = 3.1415926535f;

    enum Color
    {
        BLACK = 30,
        RED = 31,
        GREEN = 32,
        YELLOW = 33,
        BLUE = 34,
        MAGENTA = 35,
        CYAN = 36,
        WHITE = 37,
        BRIGHT_BLACK = 90,
        BRIGHT_RED = 91,
        BRIGHT_GREEN = 92,
        BRIGHT_YELLOW = 93,
        BRIGHT_BLUE = 94,
        BRIGHT_MAGENTA = 95,
        BRIGHT_CYAN = 96,
        BRIGHT_WHITE = 97
    };

    struct Cell
    {
        wchar_t glyph = L' ';
        int color = WHITE;
    };

    class Screen
    {
    public:
        Screen() : cells_(SCREEN_WIDTH * SCREEN_HEIGHT)
        {
        }

        void Clear(wchar_t glyph = L' ', int color = WHITE)
        {
            std::fill(cells_.begin(), cells_.end(), Cell{glyph, color});
        }

        void Put(int x, int y, wchar_t glyph, int color = WHITE)
        {
            if (x < 0 || x >= SCREEN_WIDTH || y < 0 || y >= SCREEN_HEIGHT)
                return;
            cells_[y * SCREEN_WIDTH + x] = {glyph, color};
        }

        void Text(int x, int y, const std::wstring& text, int color = WHITE)
        {
            for (size_t i = 0; i < text.size(); ++i)
                Put(x + static_cast<int>(i), y, text[i], color);
        }

        void CenterText(int y, const std::wstring& text, int color = WHITE)
        {
            Text((SCREEN_WIDTH - static_cast<int>(text.size())) / 2, y, text, color);
        }

        void Line(int x1, int y1, int x2, int y2, wchar_t glyph, int color)
        {
            const int dx = std::abs(x2 - x1);
            const int sx = x1 < x2 ? 1 : -1;
            const int dy = -std::abs(y2 - y1);
            const int sy = y1 < y2 ? 1 : -1;
            int error = dx + dy;

            while (true)
            {
                Put(x1, y1, glyph, color);
                if (x1 == x2 && y1 == y2)
                    break;
                const int twiceError = error * 2;
                if (twiceError >= dy)
                {
                    error += dy;
                    x1 += sx;
                }
                if (twiceError <= dx)
                {
                    error += dx;
                    y1 += sy;
                }
            }
        }

        void Box(int left, int top, int right, int bottom, int color)
        {
            for (int x = left + 1; x < right; ++x)
            {
                Put(x, top, L'─', color);
                Put(x, bottom, L'─', color);
            }
            for (int y = top + 1; y < bottom; ++y)
            {
                Put(left, y, L'│', color);
                Put(right, y, L'│', color);
            }
            Put(left, top, L'┌', color);
            Put(right, top, L'┐', color);
            Put(left, bottom, L'└', color);
            Put(right, bottom, L'┘', color);
        }

        void Present() const
        {
            std::wstring frame = L"\x1b[H";
            frame.reserve(SCREEN_WIDTH * SCREEN_HEIGHT + 2048);
            int currentColor = -1;

            for (int y = 0; y < SCREEN_HEIGHT; ++y)
            {
                for (int x = 0; x < SCREEN_WIDTH; ++x)
                {
                    const Cell& cell = cells_[y * SCREEN_WIDTH + x];
                    if (cell.color != currentColor)
                    {
                        frame += L"\x1b[";
                        frame += std::to_wstring(cell.color);
                        frame += L"m";
                        currentColor = cell.color;
                    }
                    frame += cell.glyph;
                }
                if (y != SCREEN_HEIGHT - 1)
                    frame += L'\n';
            }
            frame += L"\x1b[0m";

            DWORD written = 0;
            WriteConsoleW(GetStdHandle(STD_OUTPUT_HANDLE), frame.data(),
                          static_cast<DWORD>(frame.size()), &written, nullptr);
        }

    private:
        std::vector<Cell> cells_;
    };

    class ConsoleGuard
    {
    public:
        ConsoleGuard()
        {
            output_ = GetStdHandle(STD_OUTPUT_HANDLE);
            input_ = GetStdHandle(STD_INPUT_HANDLE);
            GetConsoleMode(output_, &oldOutputMode_);
            GetConsoleMode(input_, &oldInputMode_);

            SetConsoleOutputCP(CP_UTF8);
            SetConsoleCP(CP_UTF8);
            SetConsoleMode(output_, oldOutputMode_ | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
            SetConsoleMode(input_, oldInputMode_ & ~(ENABLE_QUICK_EDIT_MODE | ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT));
            SetConsoleTitleW(L"ECHO FORGE : The Sword Remembers");

            Write(L"\x1b[?1049h\x1b[2J\x1b[?25l");
        }

        ~ConsoleGuard()
        {
            Write(L"\x1b[0m\x1b[?25h\x1b[?1049l");
            SetConsoleMode(output_, oldOutputMode_);
            SetConsoleMode(input_, oldInputMode_);
        }

        static void Write(const std::wstring& text)
        {
            DWORD written = 0;
            WriteConsoleW(GetStdHandle(STD_OUTPUT_HANDLE), text.c_str(),
                          static_cast<DWORD>(text.size()), &written, nullptr);
        }

    private:
        HANDLE output_ = nullptr;
        HANDLE input_ = nullptr;
        DWORD oldOutputMode_ = 0;
        DWORD oldInputMode_ = 0;
    };

    class Input
    {
    public:
        void Update()
        {
            previous_ = current_;
            for (int key = 0; key < 256; ++key)
                current_[key] = (GetAsyncKeyState(key) & 0x8000) != 0;
        }

        bool Down(int key) const
        {
            return current_[key];
        }

        bool Pressed(int key) const
        {
            return current_[key] && !previous_[key];
        }

    private:
        std::array<bool, 256> current_{};
        std::array<bool, 256> previous_{};
    };

    struct Particle
    {
        float x = 0.0f;
        float y = 0.0f;
        float vx = 0.0f;
        float vy = 0.0f;
        float life = 0.0f;
        float maxLife = 0.0f;
        wchar_t glyph = L'*';
        int color = YELLOW;
    };

    enum class Scene
    {
        Title,
        Forge,
        Forging,
        Result,
        Exit
    };

    class Game
    {
    public:
        Game() : random_(std::random_device{}())
        {
        }

        void Run()
        {
            ConsoleGuard console;
            using Clock = std::chrono::steady_clock;
            auto previousTime = Clock::now();

            while (scene_ != Scene::Exit)
            {
                const auto now = Clock::now();
                float deltaTime = std::chrono::duration<float>(now - previousTime).count();
                previousTime = now;
                deltaTime = std::min(deltaTime, 0.05f);

                input_.Update();
                Update(deltaTime);
                Render();
                screen_.Present();
                Sleep(16);
            }
        }

    private:
        Screen screen_;
        Input input_;
        Scene scene_ = Scene::Title;
        std::mt19937 random_;
        std::vector<Particle> particles_;

        float worldTime_ = 0.0f;
        float sceneTime_ = 0.0f;
        int swordLevel_ = 0;
        int gold_ = 1800;
        int fragments_ = 2;
        int attempts_ = 0;
        int successes_ = 0;

        float heat_ = 50.0f;
        float marker_ = 0.0f;
        float forgeTimeLeft_ = 0.0f;
        float strikeCooldown_ = 0.0f;
        float impactFlash_ = 0.0f;
        std::vector<float> strikeScores_;

        bool lastSuccess_ = false;
        bool lastCritical_ = false;
        float lastChance_ = 0.0f;
        float lastCraftScore_ = 0.0f;
        int previousLevel_ = 0;
        std::wstring resultHeadline_;
        std::wstring resultDetail_;
        std::wstring notice_;

        float RandomFloat(float minValue, float maxValue)
        {
            std::uniform_real_distribution<float> distribution(minValue, maxValue);
            return distribution(random_);
        }

        static float Clamp(float value, float minValue, float maxValue)
        {
            return std::max(minValue, std::min(value, maxValue));
        }

        void ChangeScene(Scene next)
        {
            scene_ = next;
            sceneTime_ = 0.0f;
            notice_.clear();
        }

        int ForgeCost() const
        {
            return 120 + swordLevel_ * 95 + swordLevel_ * swordLevel_ * 12;
        }

        float BaseChance() const
        {
            static const float chances[] =
            {
                0.96f, 0.90f, 0.82f, 0.73f, 0.63f, 0.52f,
                0.41f, 0.31f, 0.22f, 0.14f, 0.08f, 0.04f
            };
            return chances[std::min(swordLevel_, 11)];
        }

        std::wstring SwordName() const
        {
            static const wchar_t* names[] =
            {
                L"Nameless Iron", L"Ember Edge", L"Crimson Oath", L"Storm Fang",
                L"Moonlit Requiem", L"Void Divider", L"Star Eater"
            };
            const int tier = std::min(swordLevel_ / 2, 6);
            return names[tier];
        }

        int SwordColor() const
        {
            if (swordLevel_ >= 10) return BRIGHT_MAGENTA;
            if (swordLevel_ >= 8) return BRIGHT_CYAN;
            if (swordLevel_ >= 6) return BRIGHT_BLUE;
            if (swordLevel_ >= 4) return BRIGHT_RED;
            if (swordLevel_ >= 2) return BRIGHT_YELLOW;
            return BRIGHT_WHITE;
        }

        void Update(float deltaTime)
        {
            worldTime_ += deltaTime;
            sceneTime_ += deltaTime;
            UpdateParticles(deltaTime);

            if (input_.Pressed(VK_ESCAPE))
            {
                if (scene_ == Scene::Title || scene_ == Scene::Forge)
                    scene_ = Scene::Exit;
                else
                    ChangeScene(Scene::Forge);
                return;
            }

            switch (scene_)
            {
            case Scene::Title:
                UpdateTitle();
                break;
            case Scene::Forge:
                UpdateForge();
                break;
            case Scene::Forging:
                UpdateForging(deltaTime);
                break;
            case Scene::Result:
                UpdateResult();
                break;
            case Scene::Exit:
                break;
            }
        }

        void UpdateTitle()
        {
            if (RandomFloat(0.0f, 1.0f) < 0.13f)
                SpawnAmbientEmber();

            if (input_.Pressed(VK_RETURN) || input_.Pressed(VK_SPACE))
                ChangeScene(Scene::Forge);
        }

        void UpdateForge()
        {
            if (RandomFloat(0.0f, 1.0f) < 0.08f)
                SpawnAmbientEmber();

            if (input_.Pressed('Q'))
            {
                scene_ = Scene::Exit;
                return;
            }

            if (input_.Pressed(VK_RETURN) || input_.Pressed(VK_SPACE))
            {
                if (gold_ < ForgeCost())
                {
                    notice_ = L"Not enough gold. The guild grants an emergency contract: +600 G";
                    gold_ += 600;
                    return;
                }
                StartForging();
            }
        }

        void StartForging()
        {
            gold_ -= ForgeCost();
            heat_ = 50.0f;
            marker_ = 0.0f;
            forgeTimeLeft_ = 14.0f;
            strikeCooldown_ = 0.0f;
            impactFlash_ = 0.0f;
            strikeScores_.clear();
            particles_.clear();
            ++attempts_;
            ChangeScene(Scene::Forging);
        }

        void UpdateForging(float deltaTime)
        {
            forgeTimeLeft_ -= deltaTime;
            strikeCooldown_ = std::max(0.0f, strikeCooldown_ - deltaTime);
            impactFlash_ = std::max(0.0f, impactFlash_ - deltaTime);

            const float speed = 2.25f + swordLevel_ * 0.11f;
            marker_ = std::sin(worldTime_ * speed) * 0.5f + 0.5f;

            heat_ -= deltaTime * (5.5f + swordLevel_ * 0.15f);
            if (input_.Down('A') || input_.Down(VK_LEFT))
                heat_ -= deltaTime * 31.0f;
            if (input_.Down('D') || input_.Down(VK_RIGHT))
                heat_ += deltaTime * 39.0f;
            heat_ = Clamp(heat_, 0.0f, 100.0f);

            if ((input_.Pressed(VK_SPACE) || input_.Pressed(VK_RETURN)) && strikeCooldown_ <= 0.0f)
                Strike();

            if (strikeScores_.size() >= 3 || forgeTimeLeft_ <= 0.0f)
                ResolveForge();
        }

        void Strike()
        {
            const float markerAccuracy = 1.0f - std::abs(marker_ - 0.5f) * 2.0f;
            const float heatAccuracy = 1.0f - std::abs(heat_ - 68.0f) / 32.0f;
            const float score = Clamp(markerAccuracy * 0.68f + heatAccuracy * 0.32f, 0.0f, 1.0f);

            strikeScores_.push_back(score);
            strikeCooldown_ = 0.36f;
            impactFlash_ = 0.16f;
            heat_ = Clamp(heat_ - 12.0f, 0.0f, 100.0f);
            SpawnImpact(score);
        }

        void ResolveForge()
        {
            while (strikeScores_.size() < 3)
                strikeScores_.push_back(0.0f);

            lastCraftScore_ = (strikeScores_[0] + strikeScores_[1] + strikeScores_[2]) / 3.0f;
            const float skillMultiplier = 0.62f + lastCraftScore_ * 0.48f;
            lastChance_ = Clamp(BaseChance() * skillMultiplier, 0.02f, 0.98f);
            previousLevel_ = swordLevel_;

            const float roll = RandomFloat(0.0f, 1.0f);
            lastCritical_ = lastCraftScore_ >= 0.94f && roll < lastChance_ * 0.20f;
            lastSuccess_ = roll < lastChance_;

            if (lastSuccess_)
            {
                swordLevel_ += lastCritical_ ? 2 : 1;
                ++successes_;
                gold_ += 65 + swordLevel_ * 15;
                resultHeadline_ = lastCritical_ ? L"RESONANCE : PERFECT ASCENSION" : L"THE BLADE ANSWERS";
                resultDetail_ = lastCritical_
                    ? L"A flawless rhythm awakened two memories at once."
                    : L"Steel, flame, and will have become one.";
            }
            else
            {
                if (previousLevel_ < 4)
                {
                    resultHeadline_ = L"THE ECHO FADES";
                    resultDetail_ = L"The blade endured. Its enhancement remains unchanged.";
                }
                else if (fragments_ > 0)
                {
                    --fragments_;
                    resultHeadline_ = L"MEMORY SHARD SHATTERED";
                    resultDetail_ = L"A shard sacrificed itself to protect the enhancement.";
                }
                else
                {
                    swordLevel_ = std::max(0, swordLevel_ - 1);
                    resultHeadline_ = L"A MEMORY WAS LOST";
                    resultDetail_ = L"The blade survives, but one enhancement has faded.";
                }
            }

            particles_.clear();
            for (int i = 0; i < 90; ++i)
                SpawnResultParticle(lastSuccess_);
            ChangeScene(Scene::Result);
        }

        void UpdateResult()
        {
            if (RandomFloat(0.0f, 1.0f) < 0.22f)
                SpawnResultParticle(lastSuccess_);

            if (sceneTime_ > 0.65f &&
                (input_.Pressed(VK_RETURN) || input_.Pressed(VK_SPACE)))
                ChangeScene(Scene::Forge);
        }

        void UpdateParticles(float deltaTime)
        {
            for (Particle& particle : particles_)
            {
                particle.life -= deltaTime;
                particle.x += particle.vx * deltaTime;
                particle.y += particle.vy * deltaTime;
                particle.vy += deltaTime * 5.0f;
            }

            particles_.erase(
                std::remove_if(particles_.begin(), particles_.end(),
                    [](const Particle& p) { return p.life <= 0.0f; }),
                particles_.end());
        }

        void SpawnAmbientEmber()
        {
            Particle particle;
            particle.x = RandomFloat(6.0f, SCREEN_WIDTH - 7.0f);
            particle.y = SCREEN_HEIGHT - 3.0f;
            particle.vx = RandomFloat(-1.2f, 1.2f);
            particle.vy = RandomFloat(-7.0f, -2.5f);
            particle.life = particle.maxLife = RandomFloat(1.0f, 2.7f);
            particle.glyph = RandomFloat(0.0f, 1.0f) > 0.6f ? L'*' : L'·';
            particle.color = RandomFloat(0.0f, 1.0f) > 0.5f ? BRIGHT_RED : BRIGHT_YELLOW;
            particles_.push_back(particle);
        }

        void SpawnImpact(float score)
        {
            const int count = 28 + static_cast<int>(score * 35.0f);
            for (int i = 0; i < count; ++i)
            {
                Particle particle;
                particle.x = 52.0f;
                particle.y = 16.0f;
                const float angle = RandomFloat(PI * 1.08f, PI * 1.92f);
                const float speed = RandomFloat(12.0f, 38.0f);
                particle.vx = std::cos(angle) * speed;
                particle.vy = std::sin(angle) * speed * 0.45f;
                particle.life = particle.maxLife = RandomFloat(0.35f, 1.05f);
                particle.glyph = i % 4 == 0 ? L'✦' : L'*';
                particle.color = i % 3 == 0 ? BRIGHT_WHITE : (i % 2 == 0 ? BRIGHT_YELLOW : BRIGHT_RED);
                particles_.push_back(particle);
            }
        }

        void SpawnResultParticle(bool success)
        {
            Particle particle;
            particle.x = 52.0f + RandomFloat(-5.0f, 5.0f);
            particle.y = 18.0f + RandomFloat(-2.0f, 2.0f);
            const float angle = RandomFloat(0.0f, PI * 2.0f);
            const float speed = RandomFloat(3.0f, 16.0f);
            particle.vx = std::cos(angle) * speed;
            particle.vy = std::sin(angle) * speed * 0.45f - 2.0f;
            particle.life = particle.maxLife = RandomFloat(0.5f, 1.6f);
            particle.glyph = success ? (RandomFloat(0.0f, 1.0f) > 0.6f ? L'✦' : L'*') : L'·';
            particle.color = success ? SwordColor() : BRIGHT_BLACK;
            particles_.push_back(particle);
        }

        void DrawParticles()
        {
            for (const Particle& particle : particles_)
            {
                const float lifeRatio = particle.life / particle.maxLife;
                const int color = lifeRatio < 0.28f ? BRIGHT_BLACK : particle.color;
                screen_.Put(static_cast<int>(std::round(particle.x)),
                            static_cast<int>(std::round(particle.y)),
                            particle.glyph, color);
            }
        }

        void DrawBackdrop()
        {
            screen_.Clear();

            for (int y = 1; y < SCREEN_HEIGHT - 1; ++y)
            {
                screen_.Put(1, y, L'│', BRIGHT_BLACK);
                screen_.Put(SCREEN_WIDTH - 2, y, L'│', BRIGHT_BLACK);
            }
            for (int x = 2; x < SCREEN_WIDTH - 2; ++x)
            {
                screen_.Put(x, 0, L'─', BRIGHT_BLACK);
                screen_.Put(x, SCREEN_HEIGHT - 1, L'─', BRIGHT_BLACK);
            }
            screen_.Put(1, 0, L'┌', BRIGHT_BLACK);
            screen_.Put(SCREEN_WIDTH - 2, 0, L'┐', BRIGHT_BLACK);
            screen_.Put(1, SCREEN_HEIGHT - 1, L'└', BRIGHT_BLACK);
            screen_.Put(SCREEN_WIDTH - 2, SCREEN_HEIGHT - 1, L'┘', BRIGHT_BLACK);

            const int pulse = static_cast<int>((std::sin(worldTime_ * 1.7f) + 1.0f) * 0.5f * 12.0f);
            for (int x = 3; x < SCREEN_WIDTH - 3; ++x)
            {
                const int distance = std::abs(x - SCREEN_WIDTH / 2);
                if (distance < 14 + pulse && (x + static_cast<int>(worldTime_ * 7.0f)) % 3 == 0)
                    screen_.Put(x, SCREEN_HEIGHT - 2, L'▁', distance < 8 ? BRIGHT_RED : RED);
            }
        }

        void DrawHeader()
        {
            screen_.Text(4, 2, L"E C H O   F O R G E", BRIGHT_RED);

            std::wstringstream right;
            right << L"GOLD " << gold_ << L" G   SHARDS " << fragments_;
            screen_.Text(SCREEN_WIDTH - 5 - static_cast<int>(right.str().size()), 2,
                         right.str(), BRIGHT_YELLOW);
            screen_.Line(4, 3, SCREEN_WIDTH - 5, 3, L'─', BRIGHT_BLACK);
        }

        void DrawSword(int centerX, int topY, int color, int level, float glow)
        {
            const int auraColor = glow > 0.52f ? color : BRIGHT_BLACK;
            const int length = 11 + std::min(level / 2, 4);

            if (level >= 2)
            {
                for (int i = 0; i < length; ++i)
                {
                    if ((i + static_cast<int>(worldTime_ * 9.0f)) % 3 == 0)
                    {
                        screen_.Put(centerX - 2, topY + i, L'·', auraColor);
                        screen_.Put(centerX + 2, topY + i, L'·', auraColor);
                    }
                }
            }

            screen_.Put(centerX, topY - 1, L'✦', level >= 6 ? color : BRIGHT_WHITE);
            screen_.Put(centerX, topY, L'▲', color);
            for (int i = 1; i < length; ++i)
            {
                screen_.Put(centerX - 1, topY + i, L'╱', color);
                screen_.Put(centerX, topY + i, level >= 8 ? L'║' : L'│', BRIGHT_WHITE);
                screen_.Put(centerX + 1, topY + i, L'╲', color);
            }
            screen_.Put(centerX - 4, topY + length, L'═', BRIGHT_YELLOW);
            screen_.Put(centerX - 3, topY + length, L'═', BRIGHT_YELLOW);
            screen_.Put(centerX - 2, topY + length, L'╪', BRIGHT_YELLOW);
            screen_.Put(centerX - 1, topY + length, L'╪', BRIGHT_YELLOW);
            screen_.Put(centerX, topY + length, L'╬', BRIGHT_WHITE);
            screen_.Put(centerX + 1, topY + length, L'╪', BRIGHT_YELLOW);
            screen_.Put(centerX + 2, topY + length, L'╪', BRIGHT_YELLOW);
            screen_.Put(centerX + 3, topY + length, L'═', BRIGHT_YELLOW);
            screen_.Put(centerX + 4, topY + length, L'═', BRIGHT_YELLOW);
            screen_.Put(centerX, topY + length + 1, L'║', YELLOW);
            screen_.Put(centerX, topY + length + 2, L'║', YELLOW);
            screen_.Put(centerX, topY + length + 3, L'◆', BRIGHT_RED);
        }

        void DrawProgressBar(int x, int y, int width, float value, int fillColor,
                             const std::wstring& label)
        {
            value = Clamp(value, 0.0f, 1.0f);
            screen_.Text(x, y, label, BRIGHT_WHITE);
            screen_.Put(x + static_cast<int>(label.size()), y, L'[', BRIGHT_BLACK);
            const int filled = static_cast<int>(value * width);
            for (int i = 0; i < width; ++i)
                screen_.Put(x + static_cast<int>(label.size()) + 1 + i, y,
                            i < filled ? L'█' : L'░',
                            i < filled ? fillColor : BRIGHT_BLACK);
            screen_.Put(x + static_cast<int>(label.size()) + width + 1, y, L']', BRIGHT_BLACK);
        }

        void Render()
        {
            DrawBackdrop();
            switch (scene_)
            {
            case Scene::Title:
                RenderTitle();
                break;
            case Scene::Forge:
                RenderForge();
                break;
            case Scene::Forging:
                RenderForging();
                break;
            case Scene::Result:
                RenderResult();
                break;
            case Scene::Exit:
                break;
            }
            DrawParticles();
        }

        void RenderTitle()
        {
            const int titleColor = std::sin(worldTime_ * 2.2f) > 0.0f ? BRIGHT_RED : BRIGHT_YELLOW;
            screen_.CenterText(5, L"███████╗ ██████╗██╗  ██╗ ██████╗", titleColor);
            screen_.CenterText(6, L"██╔════╝██╔════╝██║  ██║██╔═══██╗", titleColor);
            screen_.CenterText(7, L"█████╗  ██║     ███████║██║   ██║", titleColor);
            screen_.CenterText(8, L"██╔══╝  ██║     ██╔══██║██║   ██║", titleColor);
            screen_.CenterText(9, L"███████╗╚██████╗██║  ██║╚██████╔╝", titleColor);
            screen_.CenterText(10, L"╚══════╝ ╚═════╝╚═╝  ╚═╝ ╚═════╝", titleColor);
            screen_.CenterText(12, L"F  O  R  G  E", BRIGHT_WHITE);
            screen_.CenterText(14, L"—  T H E   S W O R D   R E M E M B E R S  —", BRIGHT_BLACK);

            DrawSword(SCREEN_WIDTH / 2, 16, BRIGHT_RED, 6,
                      (std::sin(worldTime_ * 3.0f) + 1.0f) * 0.5f);

            if (static_cast<int>(worldTime_ * 2.0f) % 2 == 0)
                screen_.CenterText(31, L"[ ENTER ]  AWAKEN THE FORGE", BRIGHT_YELLOW);
            else
                screen_.CenterText(31, L"[ ENTER ]  AWAKEN THE FORGE", WHITE);
        }

        void RenderForge()
        {
            DrawHeader();
            screen_.Box(4, 5, 65, 29, BRIGHT_BLACK);
            screen_.Box(68, 5, 99, 29, BRIGHT_BLACK);
            screen_.Text(7, 6, L"THE ANVIL", BRIGHT_RED);
            screen_.Text(71, 6, L"BLADE MEMORY", BRIGHT_CYAN);

            DrawSword(84, 8, SwordColor(), swordLevel_,
                      (std::sin(worldTime_ * 2.8f) + 1.0f) * 0.5f);

            std::wstringstream level;
            level << L"+" << swordLevel_ << L"  " << SwordName();
            screen_.CenterText(26, level.str(), SwordColor());
            screen_.CenterText(27, L"Every scar becomes a story.", BRIGHT_BLACK);

            screen_.Text(8, 9, L"ENHANCEMENT RITUAL", BRIGHT_WHITE);
            screen_.Text(8, 11, L"Control the flame. Read the rhythm. Strike three times.", BRIGHT_BLACK);
            screen_.Text(8, 13, L"← / A", BRIGHT_CYAN);
            screen_.Text(20, 13, L"cool the steel", WHITE);
            screen_.Text(8, 15, L"→ / D", BRIGHT_RED);
            screen_.Text(20, 15, L"feed the flame", WHITE);
            screen_.Text(8, 17, L"SPACE", BRIGHT_YELLOW);
            screen_.Text(20, 17, L"strike when rhythm and heat align", WHITE);

            std::wstringstream chance;
            chance << L"Base resonance    " << std::fixed << std::setprecision(0)
                   << BaseChance() * 100.0f << L"%";
            screen_.Text(8, 21, chance.str(), BRIGHT_CYAN);

            std::wstringstream cost;
            cost << L"Ritual cost       " << ForgeCost() << L" G";
            screen_.Text(8, 23, cost.str(), gold_ >= ForgeCost() ? BRIGHT_YELLOW : BRIGHT_RED);

            screen_.Text(8, 26, L"[ ENTER / SPACE ] BEGIN RITUAL", BRIGHT_YELLOW);
            screen_.Text(8, 27, L"[ Q / ESC ]       LEAVE FORGE", BRIGHT_BLACK);

            if (!notice_.empty())
                screen_.CenterText(31, notice_, BRIGHT_YELLOW);
            else
            {
                std::wstringstream record;
                record << L"RITUALS " << attempts_ << L"   ASCENSIONS " << successes_;
                screen_.CenterText(31, record.str(), BRIGHT_BLACK);
            }
        }

        void RenderForging()
        {
            DrawHeader();

            const int frameColor = impactFlash_ > 0.0f ? BRIGHT_WHITE : BRIGHT_BLACK;
            screen_.Box(4, 5, 99, 29, frameColor);
            screen_.CenterText(6, L"—  LISTEN TO THE HAMMER'S ECHO  —", BRIGHT_RED);

            // Animated hammer.
            const float hammerSwing = strikeCooldown_ > 0.18f ? strikeCooldown_ * 7.0f : 0.0f;
            const int hammerX = 39 - static_cast<int>(hammerSwing * 3.0f);
            const int hammerY = 12 - static_cast<int>(hammerSwing * 2.0f);
            screen_.Text(hammerX, hammerY, L"██████", impactFlash_ > 0.0f ? BRIGHT_WHITE : BRIGHT_BLACK);
            screen_.Text(hammerX + 2, hammerY + 1, L"██", impactFlash_ > 0.0f ? BRIGHT_YELLOW : BRIGHT_BLACK);
            screen_.Line(hammerX + 3, hammerY + 2, 50, 16, L'╲',
                         impactFlash_ > 0.0f ? BRIGHT_YELLOW : BRIGHT_BLACK);

            // Hot blade on the anvil.
            const int bladeColor = heat_ > 82.0f ? BRIGHT_WHITE :
                                   heat_ > 58.0f ? BRIGHT_YELLOW :
                                   heat_ > 28.0f ? BRIGHT_RED : BRIGHT_BLACK;
            screen_.Text(40, 16, L"═════════════════════════►", bladeColor);
            screen_.Text(34, 17, L"▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄", BRIGHT_BLACK);
            screen_.Text(38, 18, L"▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀", BRIGHT_BLACK);

            DrawProgressBar(10, 21, 31, heat_ / 100.0f, bladeColor, L"HEAT   ");
            screen_.Text(71, 21, heat_ >= 58.0f && heat_ <= 78.0f ? L"RESONANT" : L"UNSTABLE",
                         heat_ >= 58.0f && heat_ <= 78.0f ? BRIGHT_GREEN : BRIGHT_RED);

            screen_.Text(10, 24, L"RHYTHM ", BRIGHT_WHITE);
            screen_.Put(18, 24, L'[', BRIGHT_BLACK);
            constexpr int rhythmWidth = 62;
            const int markerPosition = static_cast<int>(marker_ * (rhythmWidth - 1));
            for (int i = 0; i < rhythmWidth; ++i)
            {
                const bool sweetSpot = i >= 27 && i <= 34;
                screen_.Put(19 + i, 24, sweetSpot ? L'◆' : L'─',
                            sweetSpot ? BRIGHT_GREEN : BRIGHT_BLACK);
            }
            screen_.Put(19 + markerPosition, 24, L'█', BRIGHT_YELLOW);
            screen_.Put(19 + rhythmWidth, 24, L']', BRIGHT_BLACK);

            std::wstringstream timer;
            timer << L"TIME " << std::fixed << std::setprecision(1)
                  << std::max(0.0f, forgeTimeLeft_) << L"s";
            screen_.Text(10, 27, timer.str(), forgeTimeLeft_ < 4.0f ? BRIGHT_RED : BRIGHT_WHITE);

            screen_.Text(44, 27, L"STRIKES", BRIGHT_BLACK);
            for (int i = 0; i < 3; ++i)
            {
                if (i < static_cast<int>(strikeScores_.size()))
                {
                    const float score = strikeScores_[i];
                    screen_.Text(53 + i * 9, 27,
                                 score >= 0.86f ? L"[PERF]" : score >= 0.62f ? L"[GOOD]" : L"[MISS]",
                                 score >= 0.86f ? BRIGHT_CYAN :
                                 score >= 0.62f ? BRIGHT_YELLOW : BRIGHT_RED);
                }
                else
                    screen_.Text(53 + i * 9, 27, L"[ -- ]", BRIGHT_BLACK);
            }

            screen_.CenterText(31, L"A / D : TEMPER HEAT     SPACE : STRIKE     ESC : ABORT", BRIGHT_BLACK);

            if (impactFlash_ > 0.0f)
                screen_.CenterText(10, L"✦  K R A A A N G  ✦", BRIGHT_YELLOW);
        }

        void RenderResult()
        {
            DrawHeader();

            const int resultColor = lastSuccess_ ? SwordColor() : BRIGHT_RED;
            screen_.Box(12, 5, 91, 29, sceneTime_ < 0.18f ? BRIGHT_WHITE : resultColor);
            screen_.CenterText(7, resultHeadline_, resultColor);
            screen_.CenterText(9, resultDetail_, lastSuccess_ ? BRIGHT_WHITE : BRIGHT_BLACK);

            DrawSword(52, 11, lastSuccess_ ? SwordColor() : BRIGHT_BLACK,
                      swordLevel_, (std::sin(worldTime_ * 5.0f) + 1.0f) * 0.5f);

            std::wstringstream transition;
            transition << L"+" << previousLevel_ << L"  ";
            transition << (lastSuccess_ ? L"▶▶▶" : L"───");
            transition << L"  +" << swordLevel_;
            screen_.CenterText(26, transition.str(), resultColor);

            std::wstringstream score;
            score << L"CRAFT " << std::fixed << std::setprecision(0) << lastCraftScore_ * 100.0f
                  << L"%     FINAL RESONANCE " << lastChance_ * 100.0f << L"%";
            screen_.CenterText(28, score.str(), BRIGHT_BLACK);

            if (sceneTime_ > 0.65f && static_cast<int>(worldTime_ * 2.0f) % 2 == 0)
                screen_.CenterText(31, L"[ ENTER ]  RETURN TO THE ANVIL", BRIGHT_YELLOW);
        }
    };
}

int main()
{
    Game game;
    game.Run();
    return 0;
}
