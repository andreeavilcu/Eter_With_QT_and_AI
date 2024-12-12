﻿#include "Power.h"
#include "Board.h"
#include "Game.h"


bool Power::PowerAction::controlledExplosion(Player& _player, Game& _game, const bool _check) {
    Board &board = _game.m_board;

    auto explosionEffects = Explosion::getInstance().generateExplosion(board.getSize());

    bool quit = false;

    do {
        Explosion::getInstance().printExplosion();

        std::cout << "Press 'r' to rotate explosion or 'c' to confirm.\n";
        std::cout << "Press 'x' to to quit using explosion.\n";
    }
    while (!quit && Explosion::getInstance().rotateExplosion(quit));

    board.useExplosion(_game.m_returnedCards, _game.m_eliminatedCards);

    auto &opponent = _player.getColor() == Card::Color::Player1 ? _game.m_player2 : _game.m_player1;

    for (auto& card : _game.m_returnedCards)
        if (card.getColor() == _player.getColor())
            _player.returnCard(card);

        else
            opponent.returnCard(card);

    _game.m_playedExplosion = true;

    return true;
}

bool Power::PowerAction::destruction(Player& _player, Game& _game, const bool _check) {
    Board& board = _game.m_board;

    auto& opponent = _player.getColor() == Card::Color::Player1 ? _game.m_player2 : _game.m_player1;

    auto [lastRow, lastCol] = opponent.getLastPlacedCard();

    if (lastRow == -1 || lastCol == -1) {
        return false;
    }

    if (!board.checkIndexes(lastRow, lastCol) || board.m_board[lastRow][lastCol].empty()) {
        return false;
    }

    auto& stack = board.m_board[lastRow][lastCol];
    Card affectedCard = std::move(stack.back());
    stack.pop_back();

    if (!board.checkBoardIntegrity()) {
        stack.push_back(std::move(affectedCard));
        return false;
    }

    return true;
}

bool Power::PowerAction::flame(Player& _player, Game& _game, const bool _check) {
    Board& board = _game.m_board;

    size_t illusionRow = -1, illusionCol = -1;
    bool illusionFound = false;

    for (size_t row = 0; row < board.getSize(); ++row) {
        for (size_t col = 0; col < board.getSize(); ++col) {
            if (board.checkIllusion(row, col, _player.getColor() == Card::Color::Player1 ? Card::Color::Player2 : Card::Color::Player1)) {
                illusionRow = row;
                illusionCol = col;
                illusionFound = true;
                break;
            }
        }
        if (illusionFound) break;
    }

    if (illusionFound) {
        board.m_board[illusionRow][illusionCol].back().resetIllusion();
    }

    board.printBoard();

    size_t x, y, int_value;
    std::cin >> x >> y >> int_value;

    _player.playCard(_game);

    return true;
}

bool Power::PowerAction::lava(Player& _player, Game& _game, const bool _check) {
    size_t chosenValue;
    std::cin >> chosenValue;

    if (chosenValue > 4) {
        return false;
    }

    auto targetValue = static_cast<Card::Value>(chosenValue);

    size_t count = 0;
    Board& board = _game.m_board;
    for (size_t row = 0; row < board.getSize(); ++row) {
        for (size_t col = 0; col < board.getSize(); ++col) {
            if (!board.m_board[row][col].empty() &&
                board.m_board[row][col].back().getValue() == targetValue &&
                !board.m_board[row][col].back().isIllusion()) {
                ++count;
                }
        }
    }

    if (count < 2) {
        return false;
    }

    for (size_t row = 0; row < board.getSize(); ++row) {
        for (size_t col = 0; col < board.getSize(); ++col) {
            if (!board.m_board[row][col].empty()) {
                Card topCard = std::move(board.m_board[row][col].back());
                if (topCard.getValue() == targetValue && !topCard.isIllusion()) {
                    _game.m_returnedCards.push_back(std::move(topCard));
                    board.m_board[row][col].pop_back();
                }
            }
        }
    }

    return true;
}

bool Power::PowerAction::ash(Player& _player, Game& _game, const bool _check) {
    Board& board = _game.m_board;

    const auto& eliminatedCards = _game.m_eliminatedCards;
    if (eliminatedCards.empty()) {
        return false;
    }

    for (size_t i = 0; i < eliminatedCards.size(); ++i) {
        std::cout << i + 1 << ": " << eliminatedCards[i] << std::endl;
    }

    size_t cardIndex;
    std::cin >> cardIndex;

    if (cardIndex < 1 || cardIndex > eliminatedCards.size()) {
        return false;
    }

    Card chosenCard = eliminatedCards[cardIndex - 1];

    _game.m_eliminatedCards.erase(_game.m_eliminatedCards.begin() + 1);

    size_t x, y;
    std::cout << "Enter (x, y) coordinates to place the card (0-indexed): ";
    std::cin >> x >> y;

    if(!board.checkIndexes(x, y) || !board.checkValue(x, y, chosenCard.getValue())) {
        _game.m_eliminatedCards.push_back(chosenCard);
        return false;
    }

    board.placeCard(x, y, std::move(chosenCard));
    _player.placeCard(x, y);
    return true;
}

bool Power::PowerAction::spark(Player& _player, Game& _game, const bool _check) {
    Board& board = _game.m_board;

    std::vector<std::tuple<size_t, size_t, Card>> coveredCards;

    for (size_t row = 0; row < board.getSize(); ++row) {
        for (size_t col = 0; col < board.getSize(); ++col) {
            const auto& stack = board.m_board[row][col];
            if(stack.size() >= 2) {
                const Card& topCard = stack.back();
                const Card& playerCard = stack[stack.size() - 2];

                if(playerCard.getColor() == _player.getColor() &&
                    topCard.getColor() != _player.getColor()) {
                    coveredCards.emplace_back(row, col, playerCard);
                }
            }
        }
    }

    if(coveredCards.empty()) {
        return false;
    }

    for(size_t i = 0; i < coveredCards.size(); ++i) {
        auto [row, col, card] = coveredCards[i];
        std::cout << i + 1 << ": Card" << static_cast<int>(card.getValue()) << " at position (" << row << ", " << col << ")\n";
    }

    size_t choice;
    std::cin >> choice;

    if (choice < 1 || choice > coveredCards.size()) {
        return false;
    }

    auto [origRow, origCol, chosenCard] = coveredCards[choice -1 ];

    auto& originalStack = board.m_board[origRow][origCol];
    originalStack.erase(originalStack.end() - 2);

    size_t newRow, newCol;
    std::cout << "Enter the new position (x, y) to place the card (0-indexed): ";
    std::cin >> newRow >> newCol;

    if(!board.checkIndexes(newRow, newCol) ||
        !board.checkValue(newRow, newCol, chosenCard.getValue())) {
        originalStack.insert(originalStack.end() - 1, chosenCard);
        return false;
    }

    board.placeCard(newRow, newCol, std::move(chosenCard));
    _player.placeCard(newRow, newCol);

    return true;
}

bool Power::PowerAction::squall(Player& _player, Game& _game, const bool _check) {
    Board& board = _game.m_board;

    std::vector<std::tuple<size_t, size_t, Card>> visibleOpponentCards;

    for (size_t row = 0; row < board.getSize(); ++row) {
        for (size_t col = 0; col < board.getSize(); ++col) {
            if (!board.m_board[row][col].empty()) {
                const Card& topCard = board.m_board[row][col].back();

                if (topCard.getColor() != _player.getColor() && !topCard.isIllusion()) {
                    visibleOpponentCards.emplace_back(row, col, topCard);
                }
            }
        }
    }

    if (visibleOpponentCards.empty()) {
        return false;
    }

    std::cout << "Choose a card to return to opponent's hand:\n";
    for (size_t i = 0; i < visibleOpponentCards.size(); ++i) {
        auto [row, col, card] = visibleOpponentCards[i];
        std::cout << i + 1 << ": Card " << static_cast<int>(card.getValue())
                  << " at position (" << row << ", " << col << ")\n";
    }

    size_t choice;
    std::cin >> choice;

    if (choice < 1 || choice > visibleOpponentCards.size()) {
        return false;
    }

    auto [selectedRow, selectedCol, selectedCard] = visibleOpponentCards[choice - 1];

    if (!board.checkBoardIntegrity()) {
        board.m_board[selectedRow][selectedCol].push_back(selectedCard);
        return false;
    }

    board.m_board[selectedRow][selectedCol].pop_back();

    auto& opponent = (_player.getColor() == Card::Color::Player1)
                     ? _game.m_player2
                     : _game.m_player1;
    opponent.returnCard(selectedCard);

    return true;
}

bool Power::PowerAction::gale(Player& _player, Game& _game, const bool _check) {
    Board& board = _game.m_board;

    std::vector<std::tuple<Card, size_t, size_t>> coveredCards;

    for (size_t row = 0; row < board.getSize(); ++row) {
        for (size_t col = 0; col < board.getSize(); ++col) {
            const auto& stack = board.m_board[row][col];
            if (stack.size() > 1) {
                for (size_t i = 0; i < stack.size() - 1; ++i) {
                    coveredCards.emplace_back(stack[i], row, col);
                }
            }
        }
    }

    if (coveredCards.empty()) {
        return false;
    }

    for (auto& [card, row, col] : coveredCards) {
        auto& stack = board.m_board[row][col];

        auto it = std::find(stack.begin(), stack.end(), card);
        if (it != stack.end()) {
            stack.erase(it);
        }

        _game.m_returnedCards.push_back(std::move(card));
    }

    return true;
}

bool Power::PowerAction::hurricane(Player& _player, Game& _game, const bool _check) {
    Board& board = _game.m_board;
    std::cout << "Shift a full row or column in the desired direction.\n";

    char typeChoice, directionChoice;
    int index;

    std::cout << "Choose type to shift (r: row, c: column): ";
    do {
        std::cin >> typeChoice;
        if (typeChoice != 'r' && typeChoice != 'c')
            std::cout << "Invalid choice! Choose type to shift (r: row, c: column):\n ";
    } while (typeChoice != 'r' && typeChoice != 'c');

    do {
        std::cout << "Choose direction (w: up, a: left, s: down, d: right): ";
        std::cin >> directionChoice;

        if ((typeChoice == 'r' && (directionChoice != 'a' && directionChoice != 'd')) ||
            (typeChoice == 'c' && (directionChoice != 'w' && directionChoice != 's'))) {
            std::cerr << "Invalid direction for the chosen type! Try again.\n";
        }
        else
            break;
    } while (true);

    if (typeChoice == 'r')
        std::cout << "Choose row index to shift (0 to " << board.m_board.size() - 1 << "): ";
    else
        std::cout << "Choose column index to shift (0 to " << board.m_board[0].size() - 1 << "): ";

    std::cin >> index;

    if (typeChoice == 'r' && (index < 0 || index >= board.m_board.size()))
        return false;

    if (typeChoice == 'c' && (index < 0 || index >= board.m_board[0].size()))
        return false;

    for (size_t i = 0; i < board.m_board.size(); ++i)
        if (board.m_board[index][i].empty() || board.m_board[i][index].empty())
            return false;

    if (typeChoice == 'r') {
        auto& row = board.m_board[index];

        if (directionChoice == 'a') {
            auto outStack = std::move(row[0]);
            for (size_t i = 0; i < row.size() - 1; ++i) {
                row[i] = std::move(row[i + 1]);
            }

            auto removed = std::move(row[row.size() - 1]);
            row[row.size() - 1].clear();

            for (auto& card : outStack)
                _game.m_returnedCards.push_back(card);
        }
        else if (directionChoice == 'd') {
            auto outStack=std::move(row[row.size() - 1]);
            for (size_t i = row.size() - 1; i > 0; --i) {
                row[i] = std::move(row[i - 1]);
            }

            auto removed = std::move(row[0]);
            row[0].clear();
            
            for (auto& card : outStack)
                _game.m_returnedCards.push_back(card);
        }

    }
    else if (typeChoice == 'c') {
        if (directionChoice == 's') {
            auto outStack = std::move(board.m_board[board.m_board.size() - 1][index]);
            for (size_t i = board.m_board.size() - 1; i > 0; --i) {
                board.m_board[i][index] = std::move(board.m_board[i - 1][index]);
            }

            auto removed = std::move(board.m_board[0][index]);
            board.m_board[0][index].clear();

            for (auto& card : outStack)
                _game.m_returnedCards.push_back(card);
        }

        else
            if (directionChoice == 'w') {
                auto outStack = std::move(board.m_board[0][index]);
                for (size_t i = 0; i < board.m_board.size() - 1; ++i) {
                    board.m_board[i][index] = std::move(board.m_board[i + 1][index]);
                }

                auto removed = std::move(board.m_board[board.m_board.size() - 1][index]);
                board.m_board[board.m_board.size() - 1][index].clear();

                for (auto& card : outStack)
                    _game.m_returnedCards.push_back(card);
            }
    }

    return true;
}

bool Power::PowerAction::gust(Player& _player, Game& _game, const bool _check) {
    size_t x, y;
    Board& board = _game.m_board;

    std::cout << "Moves horizontally or vertically any visible card on the board to a position adjacent to a card with a lower number.\n";
    std::cout << "Enter (x, y) coordinates for card (0-indexed): ";
    std::cin >> x >> y;

    if (board.m_board[x][y].empty()) {
        std::cout << "There is no card in this position!\n";
        return false;
    }

    std::map<char, std::pair<int, int>> directionMap = {
        {'w', {-1, 0}}, 
        {'s', {1, 0}},  
        {'a', {0, -1}}, 
        {'d', {0, 1}}   
    };

    std::cout << "Choose a direction (w = up, a = left, s = down, d = right): ";
    char direction;
    std::cin >> direction;

    if (directionMap.find(direction) == directionMap.end()) {
        std::cout << "Invalid direction!\n";
        return false;
    }

    auto offset = directionMap[direction];
    size_t newX = x + offset.first;
    size_t newY = y + offset.second;

    if (!board.checkIndexes(newX, newY)) {
        std::cout << "The move is outside the board boundaries!\n";
        return false;
    }

    if (board.m_board[newX][newY].empty()) {
        std::cout << "The target position is empty. Cannot move card there!\n";
        return false;
    }

    Card& currentCard = board.m_board[x][y].back();
    Card& targetCard = board.m_board[newX][newY].back();

    if (static_cast<size_t>(currentCard.getValue()) <= static_cast<size_t>(targetCard.getValue())) {
        return false;
    }

    if(board.checkBoardIntegrity())
        return true;
    
    board.m_board[x][y].push_back(std::move(currentCard));
    board.m_board[newX][newY].pop_back();

    return false;
}

bool Power::PowerAction::mirage(Player& _player, Game& _game, const bool _check) {
    size_t x, y;
    Board& board = _game.m_board;

    std::cout << "Replace your own placed illusion with another illusion.\n";
    std::cout << "Enter (x, y) coordinates for illusion (0-indexed)\n";
    std::cin >> x >> y;
    
    if (!_player.wasIllusionPlayed()) {
        std::cout << "No illusion has been played yet!\n";
        return false;
    }

    if (!board.checkIndexes(x, y))
        return false;

    if (board.m_board[x][y].empty()){
        std::cout << "There is no illusion in this position!\n";
        return false;
    }

    auto topCard = std::move(board.m_board[x][y].back());
    if (topCard.isIllusion() && topCard.getColor() == _player.getColor()) {
        topCard.resetIllusion();
        
        _game.m_returnedCards.push_back(topCard);

        _player.playIllusion(_game);

        return true;
    }
    std::cout << " Could not replace the ilusion!" << std::endl;
    return false;
}

bool Power::PowerAction::storm(Player& _player, Game& _game, const bool _check) {
    size_t x, y;
    Board& board = _game.m_board;

    std::cout << "Remove from play any pile of cards containing 2 or more cards.\n";
    std::cout << "Enter (x, y) coordinates for power action (0-indexed)\n";
    std::cin >> x >> y;

    if (!board.checkIndexes(x,y))
        return false;
 
    if (!board.isAPile(y,x))
        return false;

    auto eliminatedCard = std::move(board.m_board[x][y].back());
    board.m_board[x][y].pop_back();

    if (board.checkBoardIntegrity()) {
        board.m_board[x][y].push_back(std::move(eliminatedCard));
        return false;
    }

    return true;
}

bool Power::PowerAction::tide(Player& _player, Game& _game, const bool _check) {
    size_t x, y,v,w;
    Board& board = _game.m_board;

    std::cout << "Switch 2 pile of cards.\n";
    std::cout << "Enter (x, y) coordinates for power action (0-indexed)(FIRST Pile)\n";
    std::cin >> x >> y;
    std::cout << "Enter (x, y) coordinates for power action (0-indexed)(SECOND Pile)\n";
    std::cin >> v >> w;

    if (board.checkIndexes(x,y) || board.checkIndexes(v, w))
        return false;
    
    if (!board.isAPile(y, x) || !board.isAPile(w, v))
        return false;
    
    std::swap(board.m_board[x][y], board.m_board[v][w]);

    return true;
}

bool Power::PowerAction::mist(Player& _player, Game& _game, const bool _check) {
    size_t x, y;
    Board& board = _game.m_board;

    std::cout << "Play an extra illusion.\n"; 
    std::cout << "Enter (x, y) coordinates for power action (0-indexed)\n";
    std::cin >> x >> y;

    if (_player.wasIllusionPlayed()) {
        std::cout << "You have already played an illusion. You cannot have two illusions at the same time!\n";
        return false;
    }

    if (!board.checkIndexes(x, y))
        return false;

    if (board.m_board[x][y].back().isIllusion()) {
        std::cout << "There is a illusion in this position!\n";
        return false;
    }

    for (const auto& card : board.m_board[x][y]) {
        if (card.isIllusion() && card.getColor() == _player.getColor()) {
            std::cout << "You cannot have two illusions on the board at the same time!" << std::endl;
            return false; 
        }
    }

    _player.playIllusion(_game);
    return true;
}

bool Power::PowerAction::wave(Player& _player, Game& _game, const bool _check) {
    size_t x, y;
    Board& board = _game.m_board;

    std::cout << "Move the stack to an adjacent empty position. Play a card on the new empty position.\n";
    std::cout << "Enter (x, y) coordinates for power action (0-indexed): ";
    std::cin >> x >> y;

    if (!board.checkIndexes(x, y)) {
        std::cout << "Invalid coordinates!\n";
        return false;
    }

    if (!board.isAPile(x, y)) {
        return false;
    }

    std::map<char, std::pair<int, int>> directionMap = {
        {'w', {-1, 0}}, 
        {'s', {1, 0}},  
        {'a', {0, -1}}, 
        {'d', {0, 1}}  
    };

    std::cout << "Choose a direction (w = up, a = left, s = down, d = right): ";
    char direction;
    std::cin >> direction;

    if (directionMap.find(direction) == directionMap.end()) {
        std::cout << "Invalid direction!\n";
        return false;
    }

    auto offset = directionMap[direction];
    int newX = static_cast<int>(x) + offset.first;
    int newY = static_cast<int>(y) + offset.second;

    if (!board.checkIndexes(newX, newY)) {
        std::cout << "The move is outside the board boundaries!\n";
        return false;
    }

    if (!board.m_board[newX][newY].empty()) {
        std::cout << "The target position is not empty!\n";
        return false;
    }

    board.m_board[newX][newY] = std::move(board.m_board[x][y]);
    board.m_board[x][y].clear();

    _player.playCard(_game);

    return true;
}

bool Power::PowerAction::whirlpool(Player& _player, Game& _game, const bool _check) {
    size_t x, y;
    Board& board = _game.m_board;
    std::cout << "Move 2 cards from the same row or column, but separated by an empty space, onto that empty space. The card with the higher number is placed on top, and in case of a tie, the player chooses.\n";
    
    char choice;
    std::cout << "Choose row ('r') or column ('c'): ";
    std::cin >> choice;

    if (choice != 'r' && choice != 'c') {
        std::cout << "Invalid choice. Please choose 'r' for row or 'c' for column.\n";
        return false;
    }

    std::cout << "Enter the index of the row/column (0-indexed): ";
    size_t index;
    std::cin >> index;

    std::cout << "Enter (x, y) coordinates for the empty spot (0-indexed): ";
    std::cin >> x >> y;

    if (!board.checkIndexes(x, y) || !board.m_board[x][y].empty()) {
        std::cout << "Invalid coordinates or not an empty spot.\n";
        return false;
    }

    auto firstIndex = static_cast<size_t>(-1), secondIndex = static_cast<size_t>(-1);

    if (choice == 'r') {
        if (y > 0 && !board.m_board[x][y - 1].empty() && !board.isAPile(x, y - 1))
            firstIndex = y - 1;
        if (y < board.m_board[x].size() - 1 && !board.m_board[x][y + 1].empty() && !board.isAPile(x, y + 1))
            secondIndex = y + 1;
    }
    else {
        if (x > 0 && !board.m_board[x - 1][y].empty() && !board.isAPile(x - 1, y))
            firstIndex = x - 1;
        if (x < board.m_board.size() - 1 && !board.m_board[x + 1][y].empty() && !board.isAPile(x + 1, y))
            secondIndex = x + 1;
    }

    if (firstIndex == static_cast<size_t>(-1) || secondIndex == static_cast<size_t>(-1)) {
        std::cout << "Not enough valid cards to perform the operation.\n";
        return false;
    }

    auto& firstCard = (choice == 'r') ? board.m_board[x][firstIndex].back() : board.m_board[firstIndex][y].back();
    auto& secondCard = (choice == 'r') ? board.m_board[x][secondIndex].back() : board.m_board[secondIndex][y].back();

    if (board.isAPile(x, firstIndex) || board.isAPile(x, secondIndex)) {
        std::cout << "Cards cannot be moved because they are part of a pile.\n";
        return false;
    }

    if (firstCard.getValue() == secondCard.getValue()) {
        std::cout << "Cards have the same value. Choose which card goes on top:\n";
        std::cout << "1. First card\n2. Second card\n";
        int cardChoice;
        std::cin >> cardChoice;

        if (choice == 1) {
            board.m_board[x][y].push_back(std::move(firstCard));
            board.m_board[x][firstIndex].clear();

            board.m_board[x][y].push_back(std::move(secondCard));
            board.m_board[x][secondIndex].clear();
        }
        else {
            board.m_board[x][y].push_back(std::move(secondCard));
            board.m_board[x][secondIndex].clear();

            board.m_board[x][y].push_back(std::move(firstCard));
            board.m_board[x][firstIndex].clear();
        }
    }
    else {
        auto& higherCard = (firstCard.getValue() > secondCard.getValue()) ? firstCard : secondCard;
        auto& lowerCard = (firstCard.getValue() > secondCard.getValue()) ? secondCard : firstCard;

        board.m_board[x][y].push_back(std::move(higherCard));
        board.m_board[x][(choice == 'r') ? firstIndex : secondIndex].clear();

        board.m_board[x][y].push_back(std::move(lowerCard));
        board.m_board[x][(choice == 'r') ? secondIndex : firstIndex].clear();
    }

    return true;
}

bool Power::PowerAction::tsunami(Player& _player, Game& _game, const bool _check) {
    
    //dupa ce player 1 a ales coloana restrictionata pentru player2 player 2 nu mai poate pune  carti niciunde
    char line;
    Board& board = _game.m_board;
    std::cout << "Tsunami!";
    std::cout << "Choose a row ('r') ora column ('c') to restrict:\n";
    std::cin >> line;

    
   if (line != 'r' && line != 'c')
        return false;

    size_t index;
    std::cout << "Enter the index of the row/column to restrict:\n";
    std::cin >> index;

    if (index >= board.getSize())
        return false;

    bool hasFreeSpace = false;
    for (size_t i = 0; i < board.getSize(); ++i) {
        for (size_t j = 0; j < board.getSize(); ++j) {
            if (board.m_board[i][j].empty() &&
                !((line == 'r' && i == index) || (line == 'c' && j == index))) {
                hasFreeSpace = true;
                break;
            }
        }
        if (hasFreeSpace) break;
    }

    if (!hasFreeSpace) {
        std::cout << "The opponent must have at least one free space to play a card outside the restricted row/column.\n";
        return false;
    }

    Power& power = Power::getInstance();

    if (line == 'r') {
        power.setRestrictedRow(index);
    }
    else {
        power.setRestrictedCol(index);
    }

    power.setJustBlocked(true);
    return true;
}

bool Power::PowerAction::waterfall(Player& _player, Game& _game, const bool _check) {
    Board& board = _game.m_board;
    char choice;
    size_t index;
    char direction;

    std::cout << "Waterfall!";
    std::cout << "Do you want to choose a row or a column? (r for row, c for column)\n";
    std::cin >> choice;

    if (choice != 'r' && choice != 'c') {
        std::cout << "Invalid choice.\n";
        return false;
    }

    std::cout << "Choose an index (row/column) with at least 3 occupied positions.\n";
    std::cin >> index;

    if ((choice == 'r' && index >= board.getSize()) || (choice == 'c' && index >= board.getSize())) {
        std::cout << "Invalid index selection.\n";
        return false;
    }

    int occupiedPositions = 0;
    if (choice == 'r') {
        for (size_t col = 0; col < board.getSize(); ++col) {
            if (!board.m_board[index][col].empty())
                ++occupiedPositions;
        }
    }
    else {
        for (size_t row = 0; row < board.getSize(); ++row) {
            if (!board.m_board[row][index].empty())
                ++occupiedPositions;
        }
    }

    if (occupiedPositions < 3) {
        std::cout << "Not enough occupied positions.\n";
        return false;
    }

    std::cout << "Choose the direction of the cascade (l for left, r for right, u for up, d for down)\n";
    std::cin >> direction;

    if ((choice == 'r' && direction != 'l' && direction != 'r') ||
        (choice == 'c' && direction != 'u' && direction != 'd')) {
        std::cout << "Invalid direction.\n";
        return false;
    }

    std::vector<Card> mergedStack;

    if (choice == 'r') {
        if (direction == 'l') {
            for (size_t col = 0; col < board.getSize(); ++col) {
                while (!board.m_board[index][col].empty()) {
                    mergedStack.push_back(std::move(board.m_board[index][col].back()));
                    board.m_board[index][col].pop_back();
                }
            }
            board.m_board[index][0] = std::move(mergedStack);

            for (size_t col = 1; col < board.getSize(); ++col) {
                board.m_board[index][col].clear();
            }
        }
        else { 
            for (size_t col = board.getSize() - 1; col < board.getSize(); --col) {
                while (!board.m_board[index][col].empty()) {
                    mergedStack.push_back(std::move(board.m_board[index][col].back()));
                    board.m_board[index][col].pop_back();
                }
            }
            board.m_board[index][board.getSize() - 1] = std::move(mergedStack);

            for (size_t col = 0; col < board.getSize() - 1; ++col) {
                board.m_board[index][col].clear();
            }
        }
    }
    else {
        if (direction == 'u') {
            for (size_t row = 0; row < board.getSize(); ++row) {
                while (!board.m_board[row][index].empty()) {
                    mergedStack.push_back(std::move(board.m_board[row][index].back()));
                    board.m_board[row][index].pop_back();
                }
            }
            board.m_board[0][index] = std::move(mergedStack);

           
            for (size_t row = 1; row < board.getSize(); ++row) {
                board.m_board[row][index].clear();
            }
        }
        else { 
            for (size_t row = board.getSize() - 1; row < board.getSize(); --row) {
                while (!board.m_board[row][index].empty()) {
                    mergedStack.push_back(std::move(board.m_board[row][index].back()));
                    board.m_board[row][index].pop_back();
                }
            }
            board.m_board[board.getSize() - 1][index] = std::move(mergedStack);

            
            for (size_t row = 0; row < board.getSize() - 1; ++row) {
                board.m_board[row][index].clear();
            }
        }
    }

    std::cout << "Cards have merged successfully.\n";
    return true;
}

bool Power::PowerAction::support(Player& _player, Game& _game, const bool _check) {
    size_t x, y;
    Board& board = _game.m_board;

    std::cout << "Support!";
    std::cout << " Choose a card of value 1, 2 or 3 to increase its value by 1.\n";
    std::cout << "Enter the coordinates of the card:\n";
    std::cin >> x >> y;

    if (!board.checkIndexes(x, y)) {
        return false;
    }

    if (board.m_board[x][y].empty()) {
        return false;
    }

    if (board.m_board[x][y].back().isIllusion()) {
        return false;
    }

    Card& selectedCard = board.m_board[x][y].back();
    if (selectedCard.getColor() != _player.getColor()) {
        return false;
    }

    if (selectedCard.getValue() == Card::Value::Four || selectedCard.getValue() == Card::Value::Eter) {
        return false;
    }

    Power& power = Power::getInstance(); 
    power.setPlus(x, y);
    std::cout << "The value of the selected card has been increased by 1.\n";

    return true;
}

bool Power::PowerAction::earthquake(Player& _player, Game& _game, const bool _check) {
    Board& board= _game.m_board;
    bool anyRemoved = false;
    

    for (size_t row = 0; row < board.m_board.size(); ++row) {
        for (size_t col = 0; col < board.m_board.size(); ++col) {
            if (board.m_board[row][col].empty()) continue;

            if (board.m_board[row][col].back().getValue() == Card::Value::One && !board.m_board[row][col].back().isIllusion())
            {
                _game.m_eliminatedCards.push_back(std::move(board.m_board[row][col].back()));
                board.m_board[row][col].pop_back();
                anyRemoved = true;
            }

        }
    }

    if (anyRemoved) {
        std::cout << "Earthquake! All cards with the value 1 have been removed.\n";
    }
    else {
        std::cout << "There were no cards with the value 1 on the board.\n";
    }
    return anyRemoved;
}

bool Power::PowerAction::crumble(Player& _player, Game& _game, const bool _check) {
    size_t x, y;
    Board& board = _game.m_board;

    std::cout << "Choose an opponent's card of value 2, 3, or 4 to decrease its value by 1.\n";
    std::cout << "Enter the coordinates of the card:\n";
    std::cin >> x >> y;

    if (!board.checkIndexes(x, y)) {
        return false;
    }

    if (board.m_board[x][y].empty()) {
        return false;
    }

    if (board.m_board[x][y].back().isIllusion()) {
        return false;
    }

    Card& selectedCard = board.m_board[x][y].back();
    if (selectedCard.getColor() == _player.getColor()) {
        return false;
    }

    //TODO :daca cartea este acoperita sau întoarsă în mână isi pierde bonusul?

    if(selectedCard.getValue() == Card::Value::One || selectedCard.getValue() == Card::Value::Eter) {
        return false;
    }

    Power& power = Power::getInstance();
    power.setMinus(x, y);
    std::cout << "The value of the selected card has been decreased by 1.\n";

    return true;
}

bool Power::PowerAction::border(Player& _player, Game& _game, const bool _check) {
    size_t x, y;
    Board& board = _game.m_board;

    std::cout << "Border!\n";
    std::cout << "Place a neutral card on the board to define at least one boundary.\n";
    std::cout << "Enter the coordinates for placing the neutral  card.\n";
    std::cin >> x >> y;

    if (!board.checkIndexes(x, y))
        return false;

    if ((x != 0 && x != board.m_board.size() - 1 && y != 0 && y != board.m_board.size() - 1)) {
        return false;
    }


    bool canShift = false;
    if (x == 0) { 
        board.circularShiftUp();
        canShift = true;
    }
    else if (x == board.m_board.size() - 1) { 
        board.circularShiftDown();
        canShift = true;
    }
    else if (y == 0) { 
        board.circularShiftLeft();
        canShift = true;
    }
    else if (y == board.m_board.size() - 1) { 
        board.circularShiftRight();
        canShift = true;
    }

    if (!canShift) {
        return false;
    }

    bool definesBoundary = false;
    if (x == 0 || x == board.m_board.size() - 1 || y == 0 || y == board.m_board.size() - 1) {
        definesBoundary = true;
    }

    if (!definesBoundary) {
        return false;
    }
    //TO DO daca putem da shift puem pune cartea , daca nu return false
    // daca cartea nu defineste niciun border, iar return false
    Card neutralCard(Card::Value::Eter);
    board.placeCard(x, y, std::move(neutralCard));


    std::cout << "Now play a card from your hand.\n";
    size_t cardIndex;
    std::cin >> cardIndex;

    auto cardToPlay = _player.useCard(static_cast<Card::Value>(cardIndex));

    if (!cardToPlay) {
        std::cout << "Failed to play the card.\n";
        return false;
    }

    board.placeCard(x, y, std::move(*cardToPlay));
    return true;
}

bool Power::PowerAction::avalanche(Player& _player, Game& _game, const bool _check) {
    size_t x1, y1, x2, y2;
    Board& board = _game.m_board;

    std::cout << "Avalanche!\n";
    std::cout << "Choose two adiacent stacks on the board.\n";
    std::cout << "Enter the coordinates for the first stack.\n";
    std::cin >> x1 >> y1;
    std::cout << "Eneter the coordinates for the second stack/.\n";
    std::cin >> x2 >> y2;

  
    if (!board.checkIndexes(x1, y1) && !board.checkIndexes(x2, y2))
        return false;

    if (!(x1 == x2 && abs(static_cast<int>(y1) - static_cast<int>(y2)) == 1) && !(y1 == y2 && abs(static_cast<int>(x1) - static_cast<int>(x2)) == 1))
        return false;

    if (x1 == x2) {
        if (y1 < y2) {
            if (y1 > 0 && board.m_board[x1][y1 - 1].empty()) {
                board.m_board[x1][y1 - 1] = std::move(board.m_board[x1][y1]);
                board.m_board[x1][y1] = std::move(board.m_board[x1][y2]);
                board.m_board[x1][y2].clear();
            }
            else if (y2 < board.getSize() - 1 && board.m_board[x1][y2 + 1].empty()) {
                board.m_board[x1][y2 + 1] = std::move(board.m_board[x1][y2]);
                board.m_board[x1][y2] = std::move(board.m_board[x1][y1]);
                board.m_board[x1][y1].clear();
            }
            else
                return false;
        }
    }
    else if (y1 == y2) {
        if (x1 < x2) {
            if (x1 > 0 && board.m_board[x1 - 1][y1].empty()) {
                board.m_board[x1 - 1][y1] = std::move(board.m_board[x1][y1]);
                board.m_board[x1][y1] = std::move(board.m_board[x2][y1]);
                board.m_board[x2][y1].clear();
            }
            else if (x2 < board.getSize() - 1 && board.m_board[x2 + 1][y1].empty()) {
                board.m_board[x2 + 1][y1] = std::move(board.m_board[x2][y1]);
                board.m_board[x2][y1] = std::move(board.m_board[x1][y1]);
                board.m_board[x1][y1].clear();
            }
            else
                return false;
        }
    }

    return true;
}

bool Power::PowerAction::rock(Player& _player, Game& _game, const bool _check) {
    //TO DO: putere disponibila doar daca se joaca jocul cu iluzii
    size_t x, y;
    Board& board = _game.m_board;
    
    std::cout << "Rock.\n";
    std::cout << "Cover any illusion with a card (from your hand) without flipping the illusion face up.";
    std::cout << "Enter the coordinates for the illusion to cover:\n";
    std::cin >> x >> y;

    if (!board.checkIndexes(x, y))
        return false;

    if (!board.m_board[x][y].back().isIllusion())
        return false;

    std::cout << "Choose a card value to cover the illusion (0: Eter, 1: One, 2: Two, 3: Three, 4: Four):\n";
    int cardValue;
    std::cin >> cardValue;

    auto selectedCard = _player.useCard(static_cast<Card::Value>(cardValue));

    if (selectedCard == std::nullopt)
        return false;
    
    board.placeCard(x, y, std::move(*selectedCard));
   
   
    std::cout << "Illusion covered with a card.\n";
    return true;
}
