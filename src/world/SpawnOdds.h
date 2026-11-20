#pragma once

// SpawnOdds: the arithmetic that decides, once a second, whether a new customer arrives.
// Replaces: the static `chance` and `chanceDivisor` in Chair.java and the dice roll at
//           the top of Chair.assignCustomer().
// Guide: chapter 20 (testing).
//
// It is a class of its own, with the random number passed in, so that the rule can be
// tested with chosen numbers instead of real dice.

class SpawnOdds
{
public:
    // One roll. `random01` is a random number from 0 up to (not including) 1.
    // Returns true if a customer should arrive. Every roll also makes the next one a
    // little more likely.
    bool roll(double random01);

    // A customer has just arrived; `customersBefore` is how many were already in the
    // cafe. The busier the cafe, the longer until the next one.
    void customerSpawned(int customersBefore);

    // The Advertisement upgrade: higher levels bring customers sooner.
    void setAdvertisementLevel(int level);

    int chance() const;
    double divisor() const;

private:
    int m_chance = 10;
    double m_divisor = 1.0;
};
