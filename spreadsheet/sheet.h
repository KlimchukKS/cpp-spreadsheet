#pragma once

#include "cell.h"
#include "common.h"

#include <functional>
#include <unordered_map>

class Cell;

class Sheet : public SheetInterface {
public:
    Sheet() = default;
    ~Sheet();

    void SetCell(Position pos, std::string text) override;

    const CellInterface* GetCell(Position pos) const override;
    CellInterface* GetCell(Position pos) override;

    void ClearCell(Position pos) override;

    Size GetPrintableSize() const override;

    void PrintValues(std::ostream& output) const override;
    void PrintTexts(std::ostream& output) const override;

private:
    struct PositionHasher {
        size_t operator() (Position pos) const {
            return pos.row + pos.col * 1000000007;
        }
    };

    std::unordered_map<Position, std::unique_ptr<Cell>, PositionHasher> sheet_values_;
    Size minimum_printable_area_;

    // проходимся по прямым зависимостям
    bool IsAcyclicGraph(Position start_vertex);

    // Очищаем кэш у всех вершин обратной зависимости
    void ClearCache(Position start_vertex);

    void UpdateDependencies(Position start_vertex);

    void ClearDependencies(Position start_vertex);
};
