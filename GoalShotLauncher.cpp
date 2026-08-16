#include "bakkesmod/plugin/bakkesmodplugin.h"
#include "bakkesmod/wrappers/includes.h"

#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

class GoalShotLauncher final : public BakkesMod::Plugin::BakkesModPlugin
{
public:
    void onLoad() override;
    void onUnload() override;

private:
    void shootBall();
};

BAKKESMOD_PLUGIN(
    GoalShotLauncher,
    "Goal Shot Launcher",
    "1.0.0",
    PLUGINTYPE_FREEPLAY
)

void GoalShotLauncher::onLoad()
{
    cvarManager->registerCvar(
        "gs_speed",
        "1000",
        "Goal shot speed in KPH",
        true,
        true, 10.0f,
        true, 5000.0f,
        true
    );

    cvarManager->registerNotifier(
        "gs_shoot",
        [this](std::vector<std::string>)
        {
            shootBall();
        },
        "Shoot the Freeplay ball toward the opponent goal",
        PERMISSION_FREEPLAY
    );

    cvarManager->setBind("D", "gs_shoot");
    cvarManager->log("[GoalShotLauncher] Loaded. D = shoot, gs_speed <KPH> changes speed.");
}

void GoalShotLauncher::onUnload()
{
    if (cvarManager->getBindStringForKey("D") == "gs_shoot")
        cvarManager->removeBind("D");
}

void GoalShotLauncher::shootBall()
{
    if (!gameWrapper || !gameWrapper->IsInFreeplay())
    {
        cvarManager->log("[GoalShotLauncher] Freeplay only.");
        return;
    }

    ServerWrapper server = gameWrapper->GetCurrentGameState();
    if (server.IsNull())
    {
        cvarManager->log("[GoalShotLauncher] No Freeplay server found.");
        return;
    }

    BallWrapper ball = server.GetBall();
    if (ball.IsNull())
    {
        cvarManager->log("[GoalShotLauncher] Ball not found.");
        return;
    }

    CarWrapper car = gameWrapper->GetLocalCar();
    if (car.IsNull())
    {
        cvarManager->log("[GoalShotLauncher] Local car not found.");
        return;
    }

    const Vector carLocation = car.GetLocation();
    const Vector goal0 = server.GetGoalLocation(0);
    const Vector goal1 = server.GetGoalLocation(1);

    const Vector d0 = goal0 - carLocation;
    const Vector d1 = goal1 - carLocation;
    const float dist0 = d0.X * d0.X + d0.Y * d0.Y + d0.Z * d0.Z;
    const float dist1 = d1.X * d1.X + d1.Y * d1.Y + d1.Z * d1.Z;

    // In normal Freeplay, the farther goal from the local car is the one being attacked.
    const int targetGoal = (dist1 > dist0) ? 1 : 0;

    const Vector ballLocation = ball.GetLocation();
    const Vector target = server.GenerateGoalAimLocation(targetGoal, ballLocation);

    Vector direction = target - ballLocation;
    const float length = std::sqrt(
        direction.X * direction.X +
        direction.Y * direction.Y +
        direction.Z * direction.Z
    );

    if (length < 1.0f)
        return;

    direction.X /= length;
    direction.Y /= length;
    direction.Z /= length;

    CVarWrapper speedCvar = cvarManager->getCvar("gs_speed");
    float speedKph = speedCvar.IsNull() ? 1000.0f : speedCvar.getFloatValue();
    speedKph = std::clamp(speedKph, 10.0f, 5000.0f);

    // Rocket League physics uses Unreal Units/second. 100 UU = 1 metre.
    // KPH -> UU/s = KPH * 100000 / 3600.
    const float speedUU = speedKph * (100000.0f / 3600.0f);

    // Raise the ball's max-speed ceiling before setting velocity.
    ball.SetMaxLinearSpeed(speedUU * 1.10f);
    ball.SetMaxLinearSpeed2(speedUU * 1.10f);
    ball.SetVelocity(Vector(
        direction.X * speedUU,
        direction.Y * speedUU,
        direction.Z * speedUU
    ));

    cvarManager->log(
        "[GoalShotLauncher] Shot at " + std::to_string(static_cast<int>(speedKph)) + " KPH"
    );
}
