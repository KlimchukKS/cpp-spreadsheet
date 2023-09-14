#pragma once

#include "cell.h"
#include "common.h"

#include <functional>
#include <unordered_map>

class Sheet : public SheetInterface {
public:
    ~Sheet();

    void SetCell(Position pos, std::string text) override;

    const CellInterface* GetCell(Position pos) const override;
    CellInterface* GetCell(Position pos) override;

    void ClearCell(Position pos) override;

    Size GetPrintableSize() const override;

    void PrintValues(std::ostream& output) const override;
    void PrintTexts(std::ostream& output) const override;

    // Можете дополнить ваш класс нужными полями и методами
private:
    struct PositionHasher {
        size_t operator() (Position pos) const {
            return pos.row + pos.col * 1000000007;
        }
    };

    std::unordered_map<Position, std::unique_ptr<Cell>, PositionHasher> sheet_values_;
    Size minimum_printable_area_;

    bool IsAcyclicGraph(Position pos);
};
