#include "world/SpawnOdds.h"

#include <cmath>

bool SpawnOdds::roll(double random01)
{
    // A whole number from 0 up to chance/divisor; only 0 wins.
    const int rolled = int(random01 * (m_chance / m_divisor));
    m_chance = int(m_chance / 1.08);
    return rolled == 0;
}

void SpawnOdds::customerSpawned(int customersBefore)
{
    m_chance = 100 * (customersBefore + 1);
}

void SpawnOdds::setAdvertisementLevel(int level)
{
    m_divisor = 1 + std::pow(level, 1.1) / 10.0;
}

int SpawnOdds::chance() const
{
    return m_chance;
}

double SpawnOdds::divisor() const
{
    return m_divisor;
}
