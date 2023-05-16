#include <iostream>
#include <string>
#include <chrono>
#include <termios.h>
#include <thread>
#include <unistd.h>

using namespace std;

bool input_available()
{
    fd_set readfds;
    struct timeval tv;

    FD_ZERO(&readfds);
    FD_SET(STDIN_FILENO, &readfds);

    tv.tv_sec = 0;
    tv.tv_usec = 0;

    int result = select(STDIN_FILENO + 1, &readfds, nullptr, nullptr, &tv);
    return result > 0;
}

class Character
{
public:
    Character(int health) : health_(health) {}

    int health() const { return health_; }
    bool is_dead() const { return health_ <= 0; }

    void damage(int amount)
    {
        if (is_dead())
            return;
        health_ -= amount;
    }

protected:
    int health_;
};

class Hero : public Character
{
public:
    Hero() : Character(40) {}

    void attack(Character &target)
    {
        if (is_dead())
            return;
        target.damage(2);
    }
};

class Monster : public Character
{
public:
    Monster(int health, int damage) : Character(health), damage_(damage) {}

    void attack(Hero &hero)
    {
        if (is_dead())
            cout << "Monster is dead and cannot attack." << endl;
            return;
        hero.damage(damage_);
    }

private:
    int damage_;
};

void set_terminal_mode(bool raw)
{
    static struct termios oldt, newt;

    if (raw)
    {
        tcgetattr(STDIN_FILENO, &oldt); // Save the old terminal settings
        newt = oldt;
        newt.c_lflag &= ~(ICANON | ECHO); // Disable canonical mode and echo
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    }
    else
    {
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt); // Restore the old settings
    }
}

string get_input()
{
    string input;
    int c;
    while ((c = getchar()) != EOF && c != '\n')
    {
        input.push_back(static_cast<char>(c));
    }

    if (c == '\n')
        cout << input << endl;

    return input;
}

int main()
{
    set_terminal_mode(true);

    Hero hero;
    Monster orc(7, 1);
    Monster dragon(20, 3);

    cout << "Welcome to the Hero vs Monsters game!" << endl;
    cout << "Type 'attack orc + Enter' or 'attack dragon + Enter' to attack monsters." << endl;
    cout << "Defeat both monsters to win the game. Good luck!" << endl;

    using clock = chrono::steady_clock;
    auto last_update = clock::now();
    auto update_interval = chrono::milliseconds(100);

    bool game_running = true;
    while (game_running)
    {
        if (clock::now() - last_update >= update_interval)
        {
            last_update = clock::now();
            static int counter = 0;
            counter++;

            if (counter % 15 == 0 && !orc.is_dead())
            {
                orc.attack(hero);
                cout << "Orc hits hero. Hero health is " << hero.health() << endl;
            }

            if (counter % 20 == 0 && !dragon.is_dead())
            {
                dragon.attack(hero);
                cout << "Dragon hits hero. Hero health is " << hero.health() << endl;
            }

            if (orc.is_dead() && dragon.is_dead())
            {
                cout << "Congratulations! You have defeated both monsters!" << endl;
                game_running = false;
            }
            else if (hero.is_dead())
            {
                cout << "Game over. The hero has been defeated." << endl;
                game_running = false;
            }
        }

        if (input_available())
        {
            string input = get_input();
            if (!input.empty())
            {
                if (input == "attack orc")
                {
                    hero.attack(orc);
                    cout << "Hero hits orc. Orc health is " << orc.health() << endl;
                }
                else if (input == "attack dragon")
                {
                    hero.attack(dragon);
                    cout << "Hero hits dragon. Dragon health is " << dragon.health() << endl;
                }
            }
        }

        this_thread::sleep_for(chrono::milliseconds(100));
    }

    set_terminal_mode(false);
    return 0;
}