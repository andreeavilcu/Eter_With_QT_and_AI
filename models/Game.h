#pragma once

#include <vector>
#include <stack>

#include "Player.h"
#include "Board.h"
#include "Explosion.h"

#ifndef GAME_H
#define GAME_H

class Game {
public:
    enum class GameType : size_t {
        Training = 0,
        WizardDuel,
        PowerDuel,
        WizardAndPowerDuel,
    };

    enum class GridSize : size_t {
        Three = 3,
        Four = 4,
    };

protected:
    friend class Wizard;
    friend class Power;

    Board m_board; 
    GameType m_gameType;

    Player m_player1, m_player2;
    Card::Color m_winner{ Card::Color::Undefined };

public:
    std::vector<Card> m_returnedCards{};
    std::vector<Card> m_eliminatedCards{};

protected:
    bool m_playedExplosion{ false };

public:
    explicit Game(GameType _gameType);

    Board& getBoard() { return m_board; }
    GameType getGameType() { return m_gameType; }

    void run();

protected:
    [[nodiscard]] std::vector<Card> getEliminatedCards() const;

    void setEliminatedCards(const std::vector<Card> &m_eliminated_cards);

public:
    [[nodiscard]] bool checkEmptyDeck() const;
    [[nodiscard]] bool checkCardAfterReturn(Card::Color _color, Card::Value _value) const;

    [[nodiscard]] bool checkEndOfGame(Card::Color _color);

    [[nodiscard]] bool checkPartial(size_t _x, size_t _y, size_t _value) const;

    [[nodiscard]] bool playWizard(Card::Color _color);
    [[nodiscard]] bool playPower(Card::Color _color);

    void playExplosion();

};

#endif 